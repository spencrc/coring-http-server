#include "coring_server/socket.hpp"
#include <sys/socket.h>
#include <system_error>
#include <unistd.h>

using namespace coring_server;

socket::socket(address_format domain, int type, int protocol) :
	sockfd(::socket(static_cast<int>(domain), type, protocol)),
	domain(static_cast<int>(domain)) {
	if (sockfd < 0)
		throw std::system_error(errno, std::generic_category(), "Failed to create socket");
}

int socket::get_fd() const {
	return sockfd;
}

void socket::bind(int port, address interface) {
	sockaddr_in addr{};
	addr.sin_family = domain;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(static_cast<unsigned int>(interface));

	if (::bind(sockfd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) == -1)
		throw std::system_error(errno, std::generic_category(), "Failed to bind socket");
}

void socket::setsockopt(int level, int optname, const void *optval, socklen_t optlen) {
	if (::setsockopt(sockfd, level, optname, optval, optlen) == -1)
		throw std::system_error(errno, std::generic_category(), "Failed to set socket option");
}

void socket::listen(unsigned int maxConnections) {
	if (::listen(sockfd, static_cast<int>(maxConnections)) == -1)
		throw std::system_error(errno, std::generic_category(), "Failed to listen on socket");
}

int socket::accept() {
	int clientfd = ::accept(sockfd, reinterpret_cast<sockaddr *>(&peeraddr), &peeraddr_len);
	if (clientfd < 0)
		throw std::system_error(errno, std::generic_category(), "Failed to accept on socket");
	return clientfd;
}

socket::~socket() noexcept {
	close(sockfd);
}
