#pragma once
#include <array>
#include <coroutine>
#include <iostream>
#include <liburing.h>

class ConnectionCoroutine {
    public:
    std::coroutine_handle<> h;
    struct Promise {
        ConnectionCoroutine get_return_object() { 
            return ConnectionCoroutine{std::coroutine_handle<Promise>::from_promise(*this) }; 
        }
        std::suspend_never initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() noexcept { std::terminate(); }
    };
    using promise_type = Promise;
};

class AcceptAwaiter {
    public:
    AcceptAwaiter(int sockfd, io_uring* ring) noexcept : sockfd(sockfd), ring(ring) {}
    bool await_ready() const noexcept { return false; }
    void await_suspend(std::coroutine_handle<> handle) noexcept { 
        io_uring_sqe *sqe = io_uring_get_sqe(ring);
        io_uring_sqe_set_data(sqe, handle.address());

        io_uring_prep_accept(sqe, sockfd, nullptr, nullptr, 0);
        io_uring_submit(ring);
    }
    void await_resume() const noexcept { 
        std::cout << "We did it!" << std::endl;
    }
    private:
    int sockfd;
    io_uring* ring;
};

// class RecvAwaiter {
//     public:
//     RecvAwaiter(int fd) noexcept : clientfd(fd) {}
//     bool await_ready() const noexcept { return false; }
//     void await_suspend(std::coroutine_handle<> handle) noexcept {
//         recv(clientfd, &buffer, buffer.size(), 0);
//         //handle.resume();
//     }
//     std::string await_resume() const noexcept { return buffer.data(); }

//     private:
//     int clientfd;
//     std::array<char, 1024> buffer{};
//     bool ready = false;
// };