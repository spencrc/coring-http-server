#pragma once

#include "coring_server/address.hpp"
#include <netinet/in.h>

namespace coring_server {
class socket {
  public:
	// See: https://github.com/behnamasadi/cpp_tutorials/blob/master/docs/RAII.md
	socket(address_format domain, int type, int protocol);
	~socket() noexcept;
	int get_fd() const;
	void bind(int port, address interface);
	void setsockopt(int level, int optname, const void *optval, socklen_t optlen);

	// Prepare to accept connections on socket FD.
	// maxConnection # of requests will be queued before further requests are refused.
	void listen(unsigned int maxConnections);

	// Await a connection on socket FD.
	// When a connection arrives, open a new socket to communicate with it,
	int accept();

  private:
	int sockfd;
	int domain;
	sockaddr_in peeraddr{};
	socklen_t peeraddr_len = 0;
};
} // namespace coring_server
