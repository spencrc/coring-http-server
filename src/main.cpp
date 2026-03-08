#include "coro.hpp"
#include "parser.hpp"
#include "socket.hpp"
#include <netinet/in.h>
#include <sys/socket.h>

constexpr unsigned short PORT = 3003;
constexpr unsigned int KERNEL_BACKLOG = 128;
constexpr unsigned int QUEUE_DEPTH = 4096;

Task<Promise> handleConnectionAsync(int clientfd, io_uring *ring) noexcept {
	std::optional<std::string> req = co_await RecvAwaiter(clientfd, ring);
	if (!req) {
		close(clientfd);
		co_return;
	}

	RequestParser p(HTTP_REQUEST);
	const int ret = p.execute(*req);
	if (ret != HPE_OK) {
		// Malformed request! We cannot do anything with this.
		close(clientfd);
		co_return;
	}

	co_await WriteAwaiter(clientfd, ring);

	close(clientfd);
}

Task<Promise> serveAsync(int sockfd, io_uring *ring) noexcept {
	while (true) {
		int clientfd = co_await AcceptAwaiter(sockfd, ring);
		if (clientfd >= 0) {
			handleConnectionAsync(clientfd, ring);
		}
	}
}

void eventLoop(io_uring *ring) noexcept {
	io_uring_cqe *cqe = nullptr;

	while (true) {
		io_uring_wait_cqe(ring, &cqe);
		void *data = io_uring_cqe_get_data(cqe);
		auto handle = std::coroutine_handle<Promise>::from_address(data);
		handle.promise().awaiter->setRes(cqe->res);
		handle.resume();
		io_uring_cqe_seen(ring, cqe);
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