#ifndef LWS_LOCK_LOCKER_H_
#define LWS_LOCK_LOCKER_H_

#include <exception>
#include <pthread.h>
#include <semaphore.h>

#include "exceptions/wrap_exception.h"

class Semaphore
{
public:
  Semaphore()
  {
    if (sem_init(&sem_, 0, 0) != 0)
    {
      throw ServerException("Semaphore init failed");
    }
  }

  Semaphore(int val)
  {
    if (sem_init(&sem_, 0, val) != 0)
    {
      throw ServerException("Semaphore init failed");
    }
  }

  ~Semaphore() { sem_destroy(&sem_); }

  bool Wait() { return sem_wait(&sem_) == 0; }

  bool Post() { return sem_post(&sem_) == 0; }

private:
  sem_t sem_;
};

class Mutex
{
public:
  Mutex()
  {
    if (pthread_mutex_init(&mutex_, nullptr) != 0)
    {
      throw ServerException("Mutex init failed");
    }
  }

  ~Mutex() { pthread_mutex_destroy(&mutex_); }

  bool Lock() { return pthread_mutex_lock(&mutex_) == 0; }

  bool Unlock() { return pthread_mutex_unlock(&mutex_) == 0; }

  pthread_mutex_t *Get() { return &mutex_; }

private:
  pthread_mutex_t mutex_;
};

class ConditionVariable
{
public:
  ConditionVariable()
  {
    if (pthread_cond_init(&cond_, nullptr) != 0)
    {
      throw ServerException("ConditionVariable init failed");
    }
  }

  ~ConditionVariable() { pthread_cond_destroy(&cond_); }

  bool Wait(pthread_mutex_t *mutex)
  {
    return pthread_cond_wait(&cond_, mutex) == 0;
  }

  bool TimeWait(pthread_mutex_t *mutex, struct timespec t)
  {
    return pthread_cond_timedwait(&cond_, mutex, &t) == 0;
  }

  bool Signal() { return pthread_cond_signal(&cond_) == 0; }

  bool Broadcast() { return pthread_cond_broadcast(&cond_) == 0; }

private:
  pthread_cond_t cond_;
};

class LockGuard
{
public:
  LockGuard(Mutex &mutex) : mutex_(mutex) { mutex_.Lock(); }

  ~LockGuard() { mutex_.Unlock(); }

private:
  Mutex &mutex_;
};

#endif