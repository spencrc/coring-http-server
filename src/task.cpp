#include "task.hpp"
#include "client_req_parser.hpp"
#include "io_uring_ctx.hpp"
#include <cstring>

using namespace coring_server;

void base_awaiter::set_res(int res) noexcept {
	this->res = res;
}

int base_awaiter::get_res() const noexcept {
	return this->res;
}

accept_awaiter::accept_awaiter(int sockfd, io_uring_ctx *ev) noexcept : sockfd(sockfd),
																		ev(ev) {}

void accept_awaiter::await_suspend(std::coroutine_handle<promise> handle) noexcept {
	handle.promise().awaiter = this;

	ev->add_accept(sockfd, handle.address());
}

recv_awaiter::recv_awaiter(int clientfd, io_uring_ctx *ev, std::array<char, BUFFER_LEN> *buf, kernel_timespec *ts) noexcept : clientfd(clientfd),
																															  ev(ev),
																															  buf(buf),
																															  ts(ts) {}

void recv_awaiter::await_suspend(std::coroutine_handle<promise> handle) noexcept {
	handle.promise().awaiter = this;

	ev->add_read(clientfd, buf, handle.address(), ts);
}

parse_awaiter::parse_awaiter(std::string_view req) noexcept
	: req(req) {}

void parse_awaiter::await_suspend(std::coroutine_handle<promise> handle) noexcept {
	ClientReqParser p(handle);
	p.execute(req);
}

write_awaiter::write_awaiter(int clientfd, io_uring_ctx *ev, std::array<char, BUFFER_LEN> *buf, kernel_timespec *ts) noexcept : clientfd(clientfd),
																																ev(ev),
																																buf(buf),
																																ts(ts) {}

void write_awaiter::await_suspend(std::coroutine_handle<promise> handle) noexcept {
	handle.promise().awaiter = this;

	constexpr std::string_view response = "HTTP/1.1 200 \r\n"
								 "Content-Length: 12 \r\n"
								 "Content-Type: text/html \r\n"
								 "Connection: keep-alive \r\n\r\n"
								 "Hello World!";
	memcpy(buf->begin(), response.data(), response.size());

	ev->add_write(clientfd, buf, response.size(), handle.address(), ts);
}

close_awaiter::close_awaiter(int clientfd, io_uring_ctx *ev) noexcept : clientfd(clientfd),
																		ev(ev) {}

void close_awaiter::await_suspend(std::coroutine_handle<promise> handle) noexcept {
	handle.promise().awaiter = this;

	ev->add_close(clientfd, handle.address());
}
