#include "io_uring_ctx.hpp"
#include <system_error>

using namespace coring_server;

io_uring_ctx::io_uring_ctx(const unsigned int queue_depth) {
	int ret = io_uring_queue_init(queue_depth, &ring, 0);
	if (ret < 0)
		throw std::system_error(-ret, std::generic_category(), "Failed to initialize io_uring submissions & completion queue");
}

io_uring_ctx::io_uring_ctx(const unsigned int queue_depth, io_uring_params *params) {
	int ret = io_uring_queue_init_params(queue_depth, &ring, params);
	if (ret < 0)
		throw std::system_error(-ret, std::generic_category(), "Failed to initialize io_uring submissions & completion queue with params");
}

io_uring_ctx::~io_uring_ctx() {
	io_uring_queue_exit(&ring);
}

int io_uring_ctx::wait_cqe(io_uring_cqe **cqe) {
	return io_uring_wait_cqe(&ring, cqe);
}

int io_uring_ctx::submit_and_wait(const unsigned int wait_nr) {
	return io_uring_submit_and_wait(&ring, wait_nr);
}

unsigned int io_uring_ctx::peek_batch_cqe(io_uring_cqe **cqe_ptrs, const unsigned int count) {
	return io_uring_peek_batch_cqe(&ring, cqe_ptrs, count);
}

void io_uring_ctx::cq_advance(const unsigned int nr) {
	io_uring_cq_advance(&ring, nr);
}

void io_uring_ctx::cqe_seen(io_uring_cqe *cqe) {
	io_uring_cqe_seen(&ring, cqe);
}

void io_uring_ctx::add_accept(int sockfd, std::coroutine_handle<> handle) {
	io_uring_sqe *sqe = io_uring_get_sqe(&ring);
	io_uring_prep_accept(sqe, sockfd, nullptr, nullptr, 0);
	io_uring_sqe_set_data(sqe, handle.address());
}

// TODO: take flags param and separate timeout enqueuing into own func
void io_uring_ctx::add_read(int clientfd, std::array<char, BUFFER_LEN> *buf, std::coroutine_handle<> handle, kernel_timespec *ts) {
	io_uring_sqe *sqe = io_uring_get_sqe(&ring);
	io_uring_prep_recv(sqe, clientfd, buf, buf->size(), 0);
	io_uring_sqe_set_data(sqe, handle.address());
	io_uring_sqe_set_flags(sqe, IOSQE_IO_LINK);

	sqe = io_uring_get_sqe(&ring);
	io_uring_prep_link_timeout(sqe, ts, 0);
	io_uring_sqe_set_data(sqe, nullptr);
	io_uring_sqe_set_flags(sqe, 0);
}

void io_uring_ctx::add_write(int clientfd, std::array<char, BUFFER_LEN> *buf, const unsigned int write_len, std::coroutine_handle<> handle, kernel_timespec *ts) {
	io_uring_sqe *sqe = io_uring_get_sqe(&ring);
	io_uring_prep_write(sqe, clientfd, buf, write_len, 0);
	io_uring_sqe_set_data(sqe, handle.address());
	io_uring_sqe_set_flags(sqe, IOSQE_IO_LINK);

	sqe = io_uring_get_sqe(&ring);
	io_uring_prep_link_timeout(sqe, ts, 0);
	io_uring_sqe_set_data(sqe, nullptr);
	io_uring_sqe_set_flags(sqe, 0);
}

void io_uring_ctx::add_close(int clientfd, std::coroutine_handle<> handle) {
	io_uring_sqe *sqe = io_uring_get_sqe(&ring);
	io_uring_prep_close(sqe, clientfd);
	io_uring_sqe_set_data(sqe, handle.address());
}
