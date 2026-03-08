#pragma once
#include <netinet/in.h>
#include <sys/socket.h>
#include <system_error>
#include <unistd.h>

class Socket {
  public:
	// See: https://github.com/behnamasadi/cpp_tutorials/blob/master/docs/RAII.md
	Socket(int domain, int type, int protocol) : sockfd(socket(domain, type, protocol)), domain(domain) {
		if (sockfd < 0)
			throw std::system_error(errno, std::generic_category(), "Failed to create socket");
	}

	int getFd() {
		return sockfd;
	}

	void bind(int port, u_long interface) {
		sockaddr_in addr{};
		addr.sin_family = domain;
		addr.sin_port = htons(port);
		addr.sin_addr.s_addr = htonl(interface);

		if (::bind(sockfd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) == -1)
			throw std::system_error(errno, std::generic_category(), "Failed to bind socket");
	}

	void setsockopt(int level, int optname, const void *optval, socklen_t optlen) {
		if (::setsockopt(sockfd, level, optname, optval, optlen) == -1)
			throw std::system_error(errno, std::generic_category(), "Failed to set socket option");
	}

	// Prepare to accept connections on socket FD.
	// maxConnection # of requests will be queued before further requests are refused.
	void listen(int maxConnections) {
		if (::listen(sockfd, maxConnections) == -1)
			throw std::system_error(errno, std::generic_category(), "Failed to listen on socket");
	}

	// Await a connection on socket FD.
	// When a connection arrives, open a new socket to communicate with it,
	int accept() {
		int clientfd = ::accept(sockfd, reinterpret_cast<sockaddr *>(&peeraddr), &peeraddr_len);
		if (clientfd < 0)
			throw std::system_error(errno, std::generic_category(), "Failed to accept on socket");
		return clientfd;
	}

	~Socket() noexcept {
		close(sockfd);
	}

	// satisfying rule of five
	Socket(const Socket &other) = delete;			 // copy constructor
	Socket(Socket &&other) = delete;				 // move constructor
	Socket &operator=(const Socket &other) = delete; // copy asssignment
	Socket &operator=(Socket &&other) = delete;		 // move assignment

  private:
	int sockfd;
	int domain;
	sockaddr_in peeraddr{};
	socklen_t peeraddr_len = 0;
};