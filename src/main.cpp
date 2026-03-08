#include "coro.hpp"
#include "socket.hpp"
#include <csignal>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>

constexpr unsigned short PORT = 3003;
constexpr unsigned int KERNEL_BACKLOG = 128;
constexpr unsigned int QUEUE_DEPTH = 4096;

// need to create another function for async handling connection

ConnectionCoroutine serveAsync(int sockfd, io_uring *ring) noexcept {
	while (true) {
		co_await AcceptAwaiter(sockfd, ring);
		break;
	}
}

void eventLoop(io_uring *ring) noexcept {
	io_uring_cqe *cqe = nullptr;

	while (true) {
		std::cout << "Waiting..." << std::endl;
		io_uring_wait_cqe(ring, &cqe);
		std::cout << "Got it!" << std::endl;
		void *data = io_uring_cqe_get_data(cqe);
		auto handle = std::coroutine_handle<ConnectionCoroutine::Promise>::from_address(data);
		handle.resume();
		io_uring_cqe_seen(ring, cqe);
		break;
	}
}

int main() {
	io_uring ring{};
	io_uring_queue_init(QUEUE_DEPTH, &ring, 0);

	auto sock = Socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);

	constexpr unsigned int flag = 1;
	sock.setsockopt(SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
	sock.setsockopt(SOL_SOCKET, SO_REUSEPORT, &flag, sizeof(flag));

	sock.bind(PORT, INADDR_ANY);
	sock.listen(KERNEL_BACKLOG);

	serveAsync(sock.getFd(), &ring);

	eventLoop(&ring);

	io_uring_queue_exit(&ring);

	return 0;
}