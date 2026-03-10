#include "constants.hpp"
#include "parser.hpp"
#include "socket.hpp"
#include "task.hpp"
#include <liburing.h>
#include <netinet/in.h>
#include <sys/socket.h>

Task<Promise> handleConnectionAsync(int clientfd, io_uring *ring) noexcept {
	while (true) {
		std::optional<std::string> req = co_await RecvAwaiter(clientfd, ring);
		if (!req) {
			break;
		}

		// needs to be handled async!
		RequestParser p(HTTP_REQUEST);
		const int parser_errno = p.execute(*req);
		if (parser_errno != HPE_OK) {
			// Malformed request! We cannot do anything with this.
			break;
		}

		const int res = co_await WriteAwaiter(clientfd, ring);
		if (res < 0) {
			break;
		}
	}
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
		io_uring_cqe_seen(ring, cqe);

		auto handle = std::coroutine_handle<Promise>::from_address(data);
		if (handle) {
			handle.promise().awaiter->setRes(cqe->res);
			handle.resume();
		}
	}
}

int main() {
	io_uring ring{};
	io_uring_queue_init(QUEUE_DEPTH, &ring, 0);

	auto sock = Socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);

	constexpr unsigned int flag = 1;
	sock.setsockopt(SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &flag, sizeof(flag));

	sock.bind(PORT, INADDR_ANY);
	sock.listen(KERNEL_BACKLOG);

	serveAsync(sock.getFd(), &ring);

	eventLoop(&ring);

	io_uring_queue_exit(&ring);

	return 0;
}