#include "coring_server/task.hpp"
#include "coring_server/parser/client_req_parser.hpp"

using namespace coring_server;

void awaiter::set_res(int res) noexcept {
	this->res = res;
}

int awaiter::get_res() const noexcept {
	return this->res;
}

accept_awaiter::accept_awaiter(int sockfd, io_uring_ctx *ev) noexcept : sockfd(sockfd),
																		ev(ev) {
#ifdef DEBUG_MODE
	id = 1;
#endif
}

void accept_awaiter::await_suspend(std::coroutine_handle<promise> handle) noexcept {
	handle.promise().awaiter = this;

	ev->submit_accept(sockfd, handle);
}

recv_awaiter::recv_awaiter(int clientfd, io_uring_ctx *ev, std::array<char, BUFFER_LEN> *buf, kernel_timespec *ts) noexcept : clientfd(clientfd),
																															  ev(ev),
																															  buf(buf),
																															  ts(ts) {
#ifdef DEBUG_MODE
	id = 2;
#endif
}

void recv_awaiter::await_suspend(std::coroutine_handle<promise> handle) noexcept {
	handle.promise().awaiter = this;

	ev->submit_expiring_read(clientfd, buf, handle, ts);
}

parse_awaiter::parse_awaiter(std::string req) noexcept
	: req(req) {
#ifdef DEBUG_MODE
	id = 3;
#endif
}

void parse_awaiter::await_suspend(std::coroutine_handle<promise> handle) noexcept {
	ClientReqParser p(handle);
	p.execute(req);
}

write_awaiter::write_awaiter(int clientfd, io_uring_ctx *ev, std::array<char, BUFFER_LEN> *buf, kernel_timespec *ts) noexcept : clientfd(clientfd),
																																ev(ev),
																																buf(buf),
																																ts(ts) {
#ifdef DEBUG_MODE
	id = 4;
#endif
}

void write_awaiter::await_suspend(std::coroutine_handle<promise> handle) noexcept {
	handle.promise().awaiter = this;

	const std::string response = "HTTP/1.1 200 \r\n"
								 "Content-Length: 12 \r\n"
								 "Content-Type: text/html \r\n"
								 "Connection: keep-alive \r\n\r\n"
								 "Hello World!";
	std::copy(response.begin(), response.begin() + response.size(), buf->begin());

	ev->submit_expiring_write(clientfd, buf, response.size(), handle, ts);
}

close_awaiter::close_awaiter(int clientfd, io_uring_ctx *ev) noexcept : clientfd(clientfd),
																		ev(ev) {
#ifdef DEBUG_MODE
	id = 5;
#endif
}

void close_awaiter::await_suspend(std::coroutine_handle<promise> handle) noexcept {
	handle.promise().awaiter = this;

	ev->submit_close(clientfd, handle);
}
