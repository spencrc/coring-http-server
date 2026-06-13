#include "io_uring_ctx.hpp"
#include <system_error>

using namespace coring_server;

io_uring_ctx::io_uring_ctx(unsigned int queue_depth) {
	int ret = io_uring_queue_init(queue_depth, &ring, 0);
	if (ret < 0)
		throw std::system_error(-ret, std::generic_category(), "Failed to initialize io_uring submissions & completion queue");
}

io_uring_ctx::io_uring_ctx(unsigned int queue_depth, io_uring_params *params) {
	int ret = io_uring_queue_init_params(queue_depth, &ring, params);
	if (ret < 0)
		throw std::system_error(-ret, std::generic_category(), "Failed to initialize io_uring submissions & completion queue with params");
}

io_uring_ctx::~io_uring_ctx() {
	if (fixed_files) io_uring_unregister_files(&ring);
	io_uring_queue_exit(&ring);
}

int io_uring_ctx::wait_cqe(io_uring_cqe **cqe) {
	return io_uring_wait_cqe(&ring, cqe);
}

int io_uring_ctx::submit_and_wait(unsigned int wait_nr) {
	return io_uring_submit_and_wait(&ring, wait_nr);
}

unsigned int io_uring_ctx::peek_batch_cqe(io_uring_cqe **cqe_ptrs, unsigned int count) {
	return io_uring_peek_batch_cqe(&ring, cqe_ptrs, count);
}

void io_uring_ctx::cq_advance(unsigned int nr) {
	io_uring_cq_advance(&ring, nr);
}

void io_uring_ctx::cqe_seen(io_uring_cqe *cqe) {
	io_uring_cqe_seen(&ring, cqe);
}

void io_uring_ctx::accept(int sockfd, void *data, unsigned int sqe_flags) {
	io_uring_sqe *sqe = io_uring_get_sqe(&ring);
	io_uring_prep_accept(sqe, sockfd, nullptr, nullptr, 0);
	io_uring_sqe_set_data(sqe, data);
	io_uring_sqe_set_flags(sqe, sqe_flags);
}

void io_uring_ctx::accept_direct(int sockfd, void *data, unsigned int file_index, unsigned int sqe_flags) {
	io_uring_sqe *sqe = io_uring_get_sqe(&ring);
	io_uring_prep_accept_direct(sqe, sockfd, nullptr, nullptr, 0, file_index);
	io_uring_sqe_set_data(sqe, data);
	io_uring_sqe_set_flags(sqe, sqe_flags);
}

void io_uring_ctx::read(int clientfd, std::array<char, BUFFER_LEN> *buf, void *data, unsigned int sqe_flags) {
	io_uring_sqe *sqe = io_uring_get_sqe(&ring);
	io_uring_prep_recv(sqe, clientfd, buf, buf->size(), 0);
	io_uring_sqe_set_data(sqe, data);
	io_uring_sqe_set_flags(sqe, sqe_flags);
}

void io_uring_ctx::write(int clientfd, std::array<char, BUFFER_LEN> *buf, unsigned int write_len, void *data, unsigned int sqe_flags) {
	io_uring_sqe *sqe = io_uring_get_sqe(&ring);
	io_uring_prep_write(sqe, clientfd, buf, write_len, 0);
	io_uring_sqe_set_data(sqe, data);
	io_uring_sqe_set_flags(sqe, sqe_flags);
}

void io_uring_ctx::link_timeout(kernel_timespec *ts) {
	io_uring_sqe *sqe = io_uring_get_sqe(&ring);
	io_uring_prep_link_timeout(sqe, ts, 0);
	io_uring_sqe_set_data(sqe, nullptr);
	io_uring_sqe_set_flags(sqe, 0);
}

void io_uring_ctx::close(int clientfd, void *data) {
	io_uring_sqe *sqe = io_uring_get_sqe(&ring);
	io_uring_prep_close(sqe, clientfd);
	io_uring_sqe_set_data(sqe, data);
}

void io_uring_ctx::close_direct(unsigned int file_index, void *data) {
	io_uring_sqe *sqe = io_uring_get_sqe(&ring);
	io_uring_prep_close_direct(sqe, file_index);
	io_uring_sqe_set_data(sqe, data);
};

int io_uring_ctx::register_files_sparse(unsigned int nr_files) {
	int ret = io_uring_register_files_sparse(&ring, nr_files);
	if (ret < 0)
		throw std::system_error(-ret, std::generic_category(), "Failed to sparse register files");
	fixed_files = true;
	return ret;
}

int io_uring_ctx::register_files_update(unsigned int offset, std::span<const int> fds) {
	return io_uring_register_files_update(&ring, offset, fds.data(), fds.size());
}
