#include "evented.hpp"

evented::evented(const unsigned int queue_depth) {
	io_uring_queue_init(queue_depth, &ring, 0);
}

evented::~evented() {
	io_uring_queue_exit(&ring);
}

int evented::wait_cqe(io_uring_cqe **cqe) {
	return io_uring_wait_cqe(&ring, cqe);
}

void evented::cqe_seen(io_uring_cqe *cqe) {
	io_uring_cqe_seen(&ring, cqe);
}

int evented::submit_accept(int sockfd, std::coroutine_handle<> handle) {
	io_uring_sqe *sqe = io_uring_get_sqe(&ring);
	io_uring_prep_accept(sqe, sockfd, nullptr, nullptr, 0);
	io_uring_sqe_set_data(sqe, handle.address());

	return io_uring_submit(&ring);
}

int evented::submit_expiring_read(int clientfd, std::array<char, BUFFER_LEN> *buf, std::coroutine_handle<> handle, __kernel_timespec *ts) {
	io_uring_sqe *sqe = io_uring_get_sqe(&ring);
	io_uring_prep_recv(sqe, clientfd, buf, buf->size(), 0);
	io_uring_sqe_set_data(sqe, handle.address());
	io_uring_sqe_set_flags(sqe, IOSQE_IO_LINK);

	sqe = io_uring_get_sqe(&ring);
	io_uring_prep_link_timeout(sqe, ts, 0);
	io_uring_sqe_set_data(sqe, nullptr);
	io_uring_sqe_set_flags(sqe, 0);

	return io_uring_submit(&ring);
}

int evented::submit_expiring_write(int clientfd, std::array<char, BUFFER_LEN> *buf, const unsigned int write_len, std::coroutine_handle<> handle, __kernel_timespec *ts) {
	io_uring_sqe *sqe = io_uring_get_sqe(&ring);
	io_uring_prep_write(sqe, clientfd, buf, write_len, 0);
	io_uring_sqe_set_data(sqe, handle.address());
	io_uring_sqe_set_flags(sqe, IOSQE_IO_LINK);

	sqe = io_uring_get_sqe(&ring);
	io_uring_prep_link_timeout(sqe, ts, 0);
	io_uring_sqe_set_data(sqe, nullptr);
	io_uring_sqe_set_flags(sqe, 0);

	return io_uring_submit(&ring);
}

int evented::submit_close(int clientfd, std::coroutine_handle<> handle) {
	io_uring_sqe *sqe = io_uring_get_sqe(&ring);
	io_uring_prep_close(sqe, clientfd);
	io_uring_sqe_set_data(sqe, handle.address());

	return io_uring_submit(&ring);
}