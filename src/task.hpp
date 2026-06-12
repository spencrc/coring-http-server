#pragma once
#include "io_uring_ctx.hpp"
#include <coroutine>
#include <exception>
#include <string>

namespace coring_server {

template <typename T = void>
class task {
  public:
	std::coroutine_handle<T> h;
	using promise_type = T;
};

class base_awaiter;
struct promise {
	task<promise> get_return_object() {
		return task<promise>{std::coroutine_handle<promise>::from_promise(*this)};
	}
	std::suspend_never initial_suspend() noexcept { return {}; }
	std::suspend_never final_suspend() noexcept { return {}; }
	void return_void() noexcept {}
	void unhandled_exception() noexcept { std::terminate(); }
	base_awaiter *awaiter = nullptr;
	int exchanges = 0;
};

class base_awaiter {
  public:
	bool await_ready() const noexcept { return false; }
	void set_res(int res) noexcept;
	int get_res() const noexcept;

  private:
	int res = -1;

};

class accept_awaiter : public base_awaiter {
  public:
	accept_awaiter(int sockfd, io_uring_ctx *ev) noexcept;
	void await_suspend(std::coroutine_handle<promise> handle) noexcept;
	int await_resume() const noexcept { return get_res(); }

  private:
	int sockfd;
	io_uring_ctx *ev;
};

class recv_awaiter : public base_awaiter {
  public:
	recv_awaiter(int clientfd, io_uring_ctx *ev, std::array<char, BUFFER_LEN> *buf, kernel_timespec *ts) noexcept;
	void await_suspend(std::coroutine_handle<promise> handle) noexcept;
	int await_resume() const noexcept { return get_res(); }

  private:
	int clientfd;
	io_uring_ctx *ev;
	std::array<char, BUFFER_LEN> *buf;
	kernel_timespec *ts;
};

class parse_awaiter : public base_awaiter {
  public:
	parse_awaiter(std::string_view req) noexcept;
	void await_suspend(std::coroutine_handle<promise> handle) noexcept;
	void await_resume() const noexcept {}

  private:
	std::string_view req;
};

class write_awaiter : public base_awaiter {
  public:
	write_awaiter(int clientfd, io_uring_ctx *ev, std::array<char, BUFFER_LEN> *buf, kernel_timespec *ts) noexcept;
	void await_suspend(std::coroutine_handle<promise> handle) noexcept;
	int await_resume() const noexcept { return get_res(); }

  private:
	int clientfd;
	io_uring_ctx *ev;
	std::array<char, BUFFER_LEN> *buf;
	kernel_timespec *ts;
};

class close_awaiter : public base_awaiter {
  public:
	close_awaiter(int clientfd, io_uring_ctx *ev) noexcept;
	void await_suspend(std::coroutine_handle<promise> handle) noexcept;
	int await_resume() const noexcept { return get_res(); }

  private:
	int clientfd;
	io_uring_ctx *ev;
};
} // namespace coring_server
