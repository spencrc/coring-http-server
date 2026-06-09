#pragma once

#include <array>
#include <coroutine>
#include <liburing.h>

namespace coring_server {
using kernel_timespec = __kernel_timespec;
constexpr unsigned int BUFFER_LEN = 1024;

class io_uring_ctx {
  public:
	io_uring_ctx(const unsigned int queue_depth);
	io_uring_ctx(const unsigned int queue_depth, io_uring_params *params);
	~io_uring_ctx();
	int wait_cqe(io_uring_cqe **cqe);
	int submit_and_wait(const unsigned int wait_nr);
	unsigned int peek_batch_cqe(io_uring_cqe **cqe_ptrs, const unsigned int count);
	void cq_advance(const unsigned int count);
	void cqe_seen(io_uring_cqe *cqe);
	void add_accept(int sockfd, void *data);
	void add_read(int clientfd, std::array<char, BUFFER_LEN> *buf, void *data, kernel_timespec *ts);
	void add_write(int clientfd, std::array<char, BUFFER_LEN> *buf, const unsigned int write_len, void *data, kernel_timespec *ts);
	void add_link_timeout(kernel_timespec *ts); // TODO: implement
	void add_close(int clientfd, void *data);

  private:
	io_uring ring{};
};
} // namespace coring_server
