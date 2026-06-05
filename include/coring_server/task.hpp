#pragma once
#include "evented.hpp"
#include "parser/client_req_parser.hpp"
#include <coroutine>
#include <exception>
#include <string>

#ifdef DEBUG_MODE
constexpr bool AWAITER_DEBUG = true;
#else
constexpr bool AWAITER_DEBUG = false;
#endif

template <typename T = void>
class task {
  public:
	std::coroutine_handle<T> h;
	using promise_type = T;
};

class awaiter;
struct promise {
	task<promise> get_return_object() {
		return task<promise>{std::coroutine_handle<promise>::from_promise(*this)};
	}
	std::suspend_never initial_suspend() noexcept { return {}; }
	std::suspend_never final_suspend() noexcept { return {}; }
	void return_void() noexcept {}
	void unhandled_exception() noexcept { std::terminate(); }
	awaiter *awaiter = nullptr;
	int exchanges = 0;
};

class awaiter {
  public:
	virtual bool await_ready() const noexcept { return false; }
	virtual void await_suspend([[maybe_unused]] std::coroutine_handle<promise> handle) noexcept {}
	void setRes(int res) noexcept {
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

class accept_awaiter : public awaiter {
  public:
	accept_awaiter(int sockfd, evented *ev) noexcept : sockfd(sockfd), ev(ev) {
#ifdef DEBUG_MODE
		id = 1;
#endif
	}
	void await_suspend(std::coroutine_handle<promise> handle) noexcept override {
		handle.promise().awaiter = this;

		ev->submit_accept(sockfd, handle);
	}
	int await_resume() const noexcept {
		return getRes();
	}

  private:
	int sockfd;
	evented *ev;
};

class recv_awaiter : public awaiter {
  public:
	recv_awaiter(int clientfd, evented *ev, std::array<char, BUFFER_LEN> *buf, __kernel_timespec *ts) noexcept : clientfd(clientfd), ev(ev), buf(buf), ts(ts) {
#ifdef DEBUG_MODE
		id = 2;
#endif
	}
	void await_suspend(std::coroutine_handle<promise> handle) noexcept override {
		handle.promise().awaiter = this;

		ev->submit_expiring_read(clientfd, buf, handle, ts);
	}
	int await_resume() const noexcept {
		return getRes();
	}

  private:
	int clientfd;
	evented *ev;
	std::array<char, BUFFER_LEN> *buf;
	__kernel_timespec *ts;
};

class parse_awaiter : public awaiter {
  public:
	parse_awaiter(std::string req) noexcept : req(req) {
#ifdef DEBUG_MODE
		id = 3;
#endif
	}

	void await_suspend(std::coroutine_handle<promise> handle) noexcept override {
		ClientReqParser p(handle);
		p.execute(req);
	}

	void await_resume() const noexcept {}

  private:
	std::string req;
};

const std::string response = "HTTP/1.1 200 \r\n"
							 "Content-Length: 12 \r\n"
							 "Content-Type: text/html \r\n"
							 "Connection: keep-alive \r\n\r\n"
							 "Hello World!";

class write_awaiter : public awaiter {
  public:
	write_awaiter(int clientfd, evented *ev, std::array<char, BUFFER_LEN> *buf, __kernel_timespec *ts) noexcept : clientfd(clientfd), ev(ev), buf(buf), ts(ts) {
#ifdef DEBUG_MODE
		id = 4;
#endif
	}
	void await_suspend(std::coroutine_handle<promise> handle) noexcept override {
		handle.promise().awaiter = this;

		std::copy(response.begin(), response.begin() + response.size(), buf->begin());

		ev->submit_expiring_write(clientfd, buf, response.size(), handle, ts);
	}
	int await_resume() const noexcept { return getRes(); }

  private:
	int clientfd;
	evented *ev;
	std::array<char, BUFFER_LEN> *buf;
	__kernel_timespec *ts;
};

class close_awaiter : public awaiter {
  public:
	close_awaiter(int clientfd, evented *ev) noexcept : clientfd(clientfd), ev(ev) {
#ifdef DEBUG_MODE
		id = 5;
#endif
	}
	void await_suspend(std::coroutine_handle<promise> handle) noexcept override {
		handle.promise().awaiter = this;

		ev->submit_close(clientfd, handle);
	}
	int await_resume() const noexcept { return getRes(); }

  private:
	int clientfd;
	evented *ev;
};
