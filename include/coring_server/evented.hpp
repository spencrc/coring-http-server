#pragma once

#include <array>
#include <coroutine>
#include <liburing.h>

namespace coring_server {
using kernel_timespec = __kernel_timespec;
constexpr unsigned int BUFFER_LEN = 1024;

class evented {
  public:
	evented(const unsigned int queue_depth);
	evented(const unsigned int queue_depth, io_uring_params *params);
	~evented();
	int wait_cqe(io_uring_cqe **cqe);
	void cqe_seen(io_uring_cqe *cqe);
	int submit_accept(int sockfd, std::coroutine_handle<> handle);
	int submit_expiring_read(int clientfd, std::array<char, BUFFER_LEN> *buf, std::coroutine_handle<> handle, kernel_timespec *ts);
	int submit_expiring_write(int clientfd, std::array<char, BUFFER_LEN> *buf, const unsigned int write_len, std::coroutine_handle<> handle, kernel_timespec *ts);
	int submit_close(int clientfd, std::coroutine_handle<> handle);

  private:
	io_uring ring{};
};
} // namespace coring_server
