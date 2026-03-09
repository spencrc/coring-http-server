#pragma once
#include "constants.hpp"
#include <array>
#include <coroutine>
#include <exception>
#include <liburing.h>
#include <optional>
#include <string>

#ifdef DEBUG_MODE
constexpr bool AWAITER_DEBUG = true;
#else
constexpr bool AWAITER_DEBUG = false;
#endif

template <typename T = void>
class Task {
  public:
	std::coroutine_handle<T> h;
	using promise_type = T;
};

class Awaiter;
struct Promise {
	Task<Promise> get_return_object() {
		return Task{std::coroutine_handle<Promise>::from_promise(*this)};
	}
	std::suspend_never initial_suspend() noexcept { return {}; }
	std::suspend_never final_suspend() noexcept { return {}; }
	void return_void() noexcept {}
	void unhandled_exception() noexcept { std::terminate(); }
	Awaiter *awaiter = nullptr;
};

class Awaiter {
  public:
	virtual bool await_ready() const noexcept { return false; }
	virtual void await_suspend([[maybe_unused]] std::coroutine_handle<Promise> handle) noexcept {}
	void setRes(int res) {
		this->res = res;
	}
	int getRes() const noexcept {
		return res;
	}
#ifdef DEBUG_MODE
	int id = 0;
#endif
  private:
	int res = -1;
};

class AcceptAwaiter : public Awaiter {
  public:
	AcceptAwaiter(int sockfd, io_uring *ring) noexcept : sockfd(sockfd), ring(ring) {
#ifdef DEBUG_MODE
		id = 1;
#endif
	}
	void await_suspend(std::coroutine_handle<Promise> handle) noexcept override {
		handle.promise().awaiter = this;

		io_uring_sqe *sqe = io_uring_get_sqe(ring);
		io_uring_sqe_set_data(sqe, handle.address());

		io_uring_prep_accept(sqe, sockfd, nullptr, nullptr, 0);
		io_uring_submit(ring);
	}
	int await_resume() const noexcept {
		return getRes();
	}

  private:
	int sockfd;
	io_uring *ring;
};

class RecvAwaiter : public Awaiter {
  public:
	RecvAwaiter(int clientfd, io_uring *ring) noexcept : clientfd(clientfd), ring(ring) {
#ifdef DEBUG_MODE
		id = 2;
#endif
	}
	void await_suspend(std::coroutine_handle<Promise> handle) noexcept override {
		handle.promise().awaiter = this;

		io_uring_sqe *sqe = io_uring_get_sqe(ring);
		io_uring_sqe_set_data(sqe, handle.address());
		io_uring_sqe_set_flags(sqe, IOSQE_IO_LINK);
		io_uring_prep_recv(sqe, clientfd, &buffer, buffer.size(), 0);

		sqe = io_uring_get_sqe(ring);
		io_uring_sqe_set_data(sqe, nullptr);
		io_uring_sqe_set_flags(sqe, 0);
		io_uring_prep_link_timeout(sqe, &ts, 0);

		io_uring_submit(ring);
	}
	std::optional<std::string> await_resume() const noexcept {
		if (getRes() <= 0) {
			return {};
		}
		return buffer.data();
	}

  private:
	int clientfd;
	io_uring *ring;
	std::array<char, BUFFER_LEN> buffer{};
	struct __kernel_timespec ts = {
		1,
		0,
	};
};

class WriteAwaiter : public Awaiter {
  public:
	WriteAwaiter(int clientfd, io_uring *ring) noexcept : clientfd(clientfd), ring(ring) {
#ifdef DEBUG_MODE
		id = 3;
#endif
	}
	void await_suspend(std::coroutine_handle<Promise> handle) noexcept override {
		handle.promise().awaiter = this;

		const std::string response = "HTTP/1.1 200 \r\n"
									 "Content-Length: 12 \r\n"
									 "Content-Type: text/html \r\n"
									 "Connection: keep-alive \r\n\r\n"
									 "Hello World!";

		std::copy(response.begin(), response.begin() + response.size(), buffer.begin());

		io_uring_sqe *sqe = io_uring_get_sqe(ring);
		io_uring_sqe_set_data(sqe, handle.address());
		io_uring_sqe_set_flags(sqe, IOSQE_IO_LINK);
		io_uring_prep_write(sqe, clientfd, &buffer, response.size(), 0);

		sqe = io_uring_get_sqe(ring);
		io_uring_sqe_set_data(sqe, nullptr);
		io_uring_sqe_set_flags(sqe, 0);
		io_uring_prep_link_timeout(sqe, &ts, 0);

		io_uring_submit(ring);
	}
	int await_resume() const noexcept { return getRes(); }

  private:
	int clientfd;
	io_uring *ring;
	std::array<char, BUFFER_LEN> buffer{};
	struct __kernel_timespec ts = {
		1,
		0,
	};
};