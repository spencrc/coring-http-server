#include "coring_server/server.hpp"
#include "constants.hpp"
#include <chrono>

using namespace coring_server;

namespace sc = std::chrono;

server::server(const server_options opts) : port(opts.port),
											interface(opts.interface),
											max_connections(opts.max_connections),
											sock(Socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP)),
											ev(opts.queue_depth) {
	constexpr unsigned int flag = 1;
	sock.setsockopt(SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
	sock.setsockopt(SOL_SOCKET, SO_REUSEPORT, &flag, sizeof(flag));
}

void server::run() {
	sock.bind(port, interface);
	sock.listen(max_connections);

	auto serve = serveAsync();

	eventLoop();
}

Task<Promise> server::handleConnectionAsync(int clientfd) noexcept {
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
		int res = co_await RecvAwaiter(clientfd, &ev, &read_buffer, &ts);
		if (res <= 0) {
			break;
		}

		co_await ParseAwaiter(read_buffer.data());

		res = co_await WriteAwaiter(clientfd, &ev, &write_buffer, &ts);
		if (res < 0) {
			break;
		}
	}
	co_await CloseAwaiter(clientfd, &ev);
}

Task<Promise> server::serveAsync() noexcept {
	while (true) {
		int clientfd = co_await AcceptAwaiter(sock.getFd(), &ev);
		if (clientfd >= 0) {
			handleConnectionAsync(clientfd);
		}
	}
}

void server::eventLoop() noexcept {
	io_uring_cqe *cqe = nullptr;

	while (true) {
		ev.wait_cqe(&cqe);
		void *data = io_uring_cqe_get_data(cqe);
		int res = cqe->res;
		ev.cqe_seen(cqe);

		if (!data) {
			continue;
		}

		// if (!data) {
		// 	fprintf(stderr, "res: %d  (timeout/null)\n", res);
		// 	continue;
		// } else {
		// 	std::cout << "res: " << res << std::endl;
		// }

		auto handle = std::coroutine_handle<Promise>::from_address(data);
		handle.promise().awaiter->setRes(res);
		handle.resume();
	}
}
