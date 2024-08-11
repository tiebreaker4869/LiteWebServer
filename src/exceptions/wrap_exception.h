#ifndef LWS_EXCEPTIONS_WRAP_EXCEPTION_H_
#define LWS_EXCEPRIONS_WRAP_EXCEPTION_H_
#include <exception>

class ServerException : public std::exception {
public:
    ServerException(const char* msg) : msg_(msg) {}
    const char* what() const noexcept override {
        return msg_;
    }
private:
    const char* msg_;
};

#endif