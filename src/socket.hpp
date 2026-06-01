#pragma once

#include <netinet/in.h>

class Socket {
  public:
	// See: https://github.com/behnamasadi/cpp_tutorials/blob/master/docs/RAII.md
	Socket(int domain, int type, int protocol);
	~Socket() noexcept;
	int getFd();
	void bind(int port, unsigned long interface);
	void setsockopt(int level, int optname, const void *optval, socklen_t optlen);

	// Prepare to accept connections on socket FD.
	// maxConnection # of requests will be queued before further requests are refused.
	void listen(int maxConnections);

	// Await a connection on socket FD.
	// When a connection arrives, open a new socket to communicate with it,
	int accept();

  private:
	int sockfd;
	int domain;
	sockaddr_in peeraddr{};
	socklen_t peeraddr_len = 0;
};