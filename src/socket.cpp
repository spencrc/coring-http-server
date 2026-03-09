#include "socket.hpp"
#include <sys/socket.h>
#include <system_error>
#include <unistd.h>

Socket::Socket(int domain, int type, int protocol) : sockfd(socket(domain, type, protocol)), domain(domain) {
	if (sockfd < 0)
		throw std::system_error(errno, std::generic_category(), "Failed to create socket");
}

int Socket::getFd() {
	return sockfd;
}

void Socket::bind(int port, unsigned long interface) {
	sockaddr_in addr{};
	addr.sin_family = domain;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(interface);

	if (::bind(sockfd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) == -1)
		throw std::system_error(errno, std::generic_category(), "Failed to bind socket");
}

void Socket::setsockopt(int level, int optname, const void *optval, socklen_t optlen) {
	if (::setsockopt(sockfd, level, optname, optval, optlen) == -1)
		throw std::system_error(errno, std::generic_category(), "Failed to set socket option");
}

void Socket::listen(int maxConnections) {
	if (::listen(sockfd, maxConnections) == -1)
		throw std::system_error(errno, std::generic_category(), "Failed to listen on socket");
}

int Socket::accept() {
	int clientfd = ::accept(sockfd, reinterpret_cast<sockaddr *>(&peeraddr), &peeraddr_len);
	if (clientfd < 0)
		throw std::system_error(errno, std::generic_category(), "Failed to accept on socket");
	return clientfd;
}

Socket::~Socket() noexcept {
	close(sockfd);
}