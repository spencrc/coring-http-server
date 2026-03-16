#include "constants.hpp"
#include "evented.hpp"
#include "socket.hpp"
#include "task.hpp"
#include <chrono>
#include <iostream>
#include <liburing.h>

namespace sc = std::chrono;

Task<Promise> handleConnectionAsync(int clientfd, evented *ev) noexcept {
	unsigned int exchanges = 0;
	auto endtime = sc::steady_clock::now() + sc::seconds(KEEPALIVE_TIMEOUT);
	std::array<char, BUFFER_LEN> read_buffer{};
	std::array<char, BUFFER_LEN> write_buffer{};
	__kernel_timespec ts{
		1,
		0,
	};

	while (++exchanges < KEEPALIVE_REQUESTS and
		   sc::steady_clock::now() < endtime) {
		int res = co_await RecvAwaiter(clientfd, ev, &read_buffer, &ts);
		if (res <= 0) {
			break;
		}

		co_await ParseAwaiter(read_buffer.data());

		res = co_await WriteAwaiter(clientfd, ev, &write_buffer, &ts);
		if (res < 0) {
			break;
		}
	}
	co_await CloseAwaiter(clientfd, ev);
}

Task<Promise> serveAsync(int sockfd, evented *ev) noexcept {
	while (true) {
		int clientfd = co_await AcceptAwaiter(sockfd, ev);
		if (clientfd >= 0) {
			handleConnectionAsync(clientfd, ev);
		}
	}
}

void eventLoop(evented *ev) noexcept {
	io_uring_cqe *cqe = nullptr;

	while (true) {
		ev->wait_cqe(&cqe);
		void *data = io_uring_cqe_get_data(cqe);
		int res = cqe->res;
		ev->cqe_seen(cqe);

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
	evented ev(QUEUE_DEPTH);

	auto sock = Socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);

	constexpr unsigned int flag = 1;
	sock.setsockopt(SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &flag, sizeof(flag));

	sock.bind(PORT, INADDR_ANY);
	sock.listen(KERNEL_BACKLOG);

	auto serve = serveAsync(sock.getFd(), &ev);

	eventLoop(&ev);

	return 0;
}