#pragma once
#include <array>
#include <coroutine>
#include <exception>
#include <liburing.h>
#include <string>

class ConnectionCoroutine {
  public:
	struct Promise;
	std::coroutine_handle<Promise> h;

	struct Promise {
		// need to create Awaiter template and have field storing one here

		ConnectionCoroutine get_return_object() {
			return ConnectionCoroutine{std::coroutine_handle<Promise>::from_promise(*this)};
		}
		std::suspend_never initial_suspend() noexcept { return {}; }
		std::suspend_never final_suspend() noexcept { return {}; }
		void return_void() {}
		void unhandled_exception() noexcept { std::terminate(); }
	};
	using promise_type = Promise;
};

class AcceptAwaiter {
  public:
	AcceptAwaiter(int sockfd, io_uring *ring) noexcept : sockfd(sockfd), ring(ring) {}
	bool await_ready() const noexcept { return false; }
	void await_suspend(std::coroutine_handle<> handle) noexcept {
		io_uring_sqe *sqe = io_uring_get_sqe(ring);
		io_uring_sqe_set_data(sqe, handle.address());

		io_uring_prep_accept(sqe, sockfd, nullptr, nullptr, 0);
		io_uring_submit(ring);
	}
	void await_resume() const noexcept {}

  private:
	int sockfd;
	io_uring *ring;
};

constexpr unsigned int BUFFER_LEN = 2048;
class RecvAwaiter {
  public:
	RecvAwaiter(int clientfd, io_uring *ring) noexcept : clientfd(clientfd), ring(ring) {}
	bool await_ready() const noexcept { return false; }
	void await_suspend(std::coroutine_handle<> handle) noexcept {
		io_uring_sqe *sqe = io_uring_get_sqe(ring);
		io_uring_sqe_set_data(sqe, handle.address());

		io_uring_prep_recv(sqe, clientfd, &buffer, buffer.size(), 0);
		io_uring_submit(ring);
	}
	std::string await_resume() const noexcept { return buffer.data(); }

  private:
	int clientfd;
	io_uring *ring;
	std::array<char, BUFFER_LEN> buffer{};
};