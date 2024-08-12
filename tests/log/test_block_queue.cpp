#include <gtest/gtest.h>
#include <pthread.h>

#include "log/block_queue.h"

// 测试 BlockQueue 的构造函数
TEST(BlockQueueTest, Constructor) {
    BlockQueue<int> queue(10);
    EXPECT_EQ(queue.Size(), 0);
    EXPECT_EQ(queue.MaxSize(), 10);
}

// 测试 Clear 方法
TEST(BlockQueueTest, Clear) {
    BlockQueue<int> queue(10);
    queue.Push(1);
    queue.Clear();
    EXPECT_EQ(queue.Size(), 0);
    EXPECT_TRUE(queue.Empty());
}

// 测试 Full 方法
TEST(BlockQueueTest, Full) {
    BlockQueue<int> queue(2);
    queue.Push(1);
    queue.Push(2);
    EXPECT_TRUE(queue.Full());
}

// 测试 Empty 方法
TEST(BlockQueueTest, Empty) {
    BlockQueue<int> queue(2);
    EXPECT_TRUE(queue.Empty());
    queue.Push(1);
    EXPECT_FALSE(queue.Empty());
}

// 测试 Front 方法
TEST(BlockQueueTest, Front) {
    BlockQueue<int> queue(2);
    queue.Push(1);
    int front;
    EXPECT_TRUE(queue.Front(front));
    EXPECT_EQ(front, 1);
}

// 测试 Back 方法
TEST(BlockQueueTest, Back) {
    BlockQueue<int> queue(2);
    queue.Push(1);
    int back;
    EXPECT_TRUE(queue.Back(back));
    EXPECT_EQ(back, 1);
}

// 测试 Push 方法
TEST(BlockQueueTest, Push) {
    BlockQueue<int> queue(2);
    EXPECT_TRUE(queue.Push(1));
    EXPECT_EQ(queue.Size(), 1);
}

// 测试 Pop 方法
TEST(BlockQueueTest, Pop) {
    BlockQueue<int> queue(2);
    queue.Push(1);
    int elem;
    EXPECT_TRUE(queue.Pop(elem));
    EXPECT_EQ(elem, 1);
    EXPECT_EQ(queue.Size(), 0);
}

// 测试带超时的 Pop 方法
TEST(BlockQueueTest, PopWithTimeout) {
    BlockQueue<int> queue(2);
    int elem;
    EXPECT_FALSE(queue.Pop(elem, 1000)); // 超时返回 false
}

// 多线程并发测试
void *PushThread(void *arg) {
    BlockQueue<int> *queue = (BlockQueue<int> *)arg;
    for (int i = 0; i < 10000; i++) {
        queue->Push(i);
    }
    return NULL;
}

void *PopThread(void *arg) {
    BlockQueue<int> *queue = (BlockQueue<int> *)arg;
    for (int i = 0; i < 10000; i++) {
        int elem;
        queue->Pop(elem);
    }
    return NULL;
}

TEST(BlockQueueTest, MultiThread) {
    BlockQueue<int> queue(100);
    pthread_t push_tid, pop_tid;
    pthread_create(&push_tid, NULL, PushThread, &queue);
    pthread_create(&pop_tid, NULL, PopThread, &queue);
    pthread_join(push_tid, NULL);
    pthread_join(pop_tid, NULL);
    EXPECT_EQ(queue.Size(), 0);
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}