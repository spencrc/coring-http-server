#include "constants.hpp"
#include "parser.hpp"
#include "socket.hpp"
#include "task.hpp"
#include <iostream>
#include <liburing.h>
#include <netinet/in.h>
#include <sys/socket.h>

Task<Promise> handleConnectionAsync(int clientfd, io_uring *ring) noexcept {
	unsigned int exchanges = 0;
	std::array<char, BUFFER_LEN> buffer{};
	std::array<char, BUFFER_LEN> write_buffer{};
	__kernel_timespec ts{
		1,
		0,
	};

	while (++exchanges < MAX_NUM_EXCHANGES) {
		int res = co_await RecvAwaiter(clientfd, ring, &buffer, &ts);
		if (res <= 0) {
			break;
		}

		// needs to be handled async!
		// RequestParser p(HTTP_REQUEST);
		// const int parser_errno = p.execute();
		// if (parser_errno != HPE_OK) {
		// 	// Malformed request! We cannot do anything with this.
		// 	break;
		// }

		res = co_await WriteAwaiter(clientfd, ring, &write_buffer, &ts);
		if (res < 0) {
			break;
		}
	}
	co_await CloseAwaiter(clientfd, ring);
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
		int res = cqe->res;
		io_uring_cqe_seen(ring, cqe);

		if (!data) {
			fprintf(stderr, "res: %d  (timeout/null)\n", res);
			continue;
		} else {
			std::cout << "res: " << res << std::endl;
		}

		auto handle = std::coroutine_handle<Promise>::from_address(data);
		handle.promise().awaiter->setRes(res);
		handle.resume();
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

	auto serve = serveAsync(sock.getFd(), &ring);

	eventLoop(&ring);

	io_uring_queue_exit(&ring);

	return 0;
}