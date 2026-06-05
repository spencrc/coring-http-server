#include "coring_server/server.hpp"
#include <chrono>

using namespace coring_server;

namespace sc = std::chrono;

server::server(address interface, const server_options& opts) :
	sock(socket(opts.domain, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP)),
	ev(opts.queue_depth),
	port(opts.port),
	interface(interface),
	max_connections(opts.max_connections),
	keepalive_requests(opts.keepalive_requests),
	keepalive_timeout(opts.keepalive_timeout)
{
	constexpr unsigned int flag = 1;
	sock.setsockopt(SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
	sock.setsockopt(SOL_SOCKET, SO_REUSEPORT, &flag, sizeof(flag));
}

void server::run() {
	sock.bind(port, interface);
	sock.listen(max_connections);

	[[maybe_unused]] auto serve = serve_async();

	event_loop();
}

task<promise> server::handle_connection_async(int clientfd) noexcept {
	unsigned int exchanges = 0;
	auto endtime = sc::steady_clock::now() + sc::seconds(keepalive_timeout);
	std::array<char, BUFFER_LEN> read_buffer{};
	std::array<char, BUFFER_LEN> write_buffer{};
	kernel_timespec ts{ .tv_sec = 1, .tv_nsec = 0 };

	while (++exchanges < keepalive_requests and
		   sc::steady_clock::now() < endtime) {
		int res = co_await recv_awaiter(clientfd, &ev, &read_buffer, &ts);
		if (res <= 0) {
			break;
		}

		co_await parse_awaiter(read_buffer.data());

		res = co_await write_awaiter(clientfd, &ev, &write_buffer, &ts);
		if (res < 0) {
			break;
		}
	}
	co_await close_awaiter(clientfd, &ev);
}

task<promise> server::serve_async() noexcept {
	while (true) {
		int clientfd = co_await accept_awaiter(sock.get_fd(), &ev);
		if (clientfd >= 0) {
			handle_connection_async(clientfd);
		}
	}
}

void server::event_loop() noexcept {
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

		auto handle = std::coroutine_handle<promise>::from_address(data);
		handle.promise().awaiter->set_res(res);
		handle.resume();
	}
}
