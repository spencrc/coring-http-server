#pragma once

#include <array>
#include <liburing.h>
#include <span>

namespace coring_server {
using kernel_timespec = __kernel_timespec;
constexpr unsigned int BUFFER_LEN = 1024;

class io_uring_ctx {
  public:
	io_uring_ctx(unsigned int queue_depth);
	io_uring_ctx(unsigned int queue_depth, io_uring_params *params);
	~io_uring_ctx();
	int wait_cqe(io_uring_cqe **cqe);
	int submit_and_wait(unsigned int wait_nr);
	unsigned int peek_batch_cqe(io_uring_cqe **cqe_ptrs, unsigned int count);
	void cq_advance(unsigned int count);
	void cqe_seen(io_uring_cqe *cqe);
	void accept(int sockfd, void *data, unsigned int sqe_flags);
	void accept_direct(int sockfd, void *data, unsigned int file_index, unsigned int sqe_flags);
	void read(int clientfd, std::array<char, BUFFER_LEN> *buf, void *data, unsigned int sqe_flags);
	void write(int clientfd, std::array<char, BUFFER_LEN> *buf, unsigned int write_len, void *data, unsigned int sqe_flags);
	void link_timeout(kernel_timespec *ts);
	void close(int clientfd, void *data);
	void close_direct(unsigned int file_index, void *data);
	int register_files_sparse(unsigned int nr_files);
	int register_files_update(unsigned int offset, std::span<const int> fds);
	int register_napi(io_uring_napi *napi);

  private:
	io_uring ring{};
	io_uring_napi *napi = nullptr;
	bool fixed_files = false;
};
} // namespace coring_server
