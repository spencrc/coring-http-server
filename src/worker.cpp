#include "worker.hpp"
#include <chrono>

using namespace coring_server;

namespace sc = std::chrono;

io_uring_params params{
	.flags = IORING_SETUP_COOP_TASKRUN,
};

server::worker::worker(address interface, const server_options &opts) : sock(socket(opts.domain, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP)),
																		ev(opts.queue_depth, &params),
																		port(opts.port),
																		interface(interface),
																		max_connections(opts.max_connections),
																		keepalive_requests(opts.keepalive_requests),
																		keepalive_timeout(opts.keepalive_timeout) {
	constexpr unsigned int flag = 1;
	sock.setsockopt(SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
	sock.setsockopt(SOL_SOCKET, SO_REUSEPORT, &flag, sizeof(flag));
}

void server::worker::run() {
	sock.bind(port, interface);
	sock.listen(max_connections);

	[[maybe_unused]] auto serve = serve_async();

	event_loop();
}

task<promise> server::worker::handle_connection_async(int clientfd) noexcept {
	unsigned int exchanges = 0;
	auto endtime = sc::steady_clock::now() + sc::seconds(keepalive_timeout);
	// TODO: replace these buffers with io_uring buffers (see io_uring_register_buf_ring)
	std::array<char, BUFFER_LEN> read_buffer{};
	std::array<char, BUFFER_LEN> write_buffer{};
	// TODO: make the idle timeout (currently tv_sec) be in miliseconds,
	//  and make it defined by the user in server's constructor
	kernel_timespec ts{.tv_sec = 5, .tv_nsec = 0};

	while (++exchanges < keepalive_requests and
		   sc::steady_clock::now() < endtime) {
		int res = co_await recv_awaiter(clientfd, &ev, &read_buffer, &ts);
		// res = 0 indicates nothing was read, so there's nothing to parse.
		//  meanwhile, res < 0 indicates there was an error
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

task<promise> server::worker::serve_async() noexcept {
	while (true) {
		int clientfd = co_await accept_awaiter(sock.get_fd(), &ev);
		if (clientfd >= 0) {
			handle_connection_async(clientfd);
		}
	}
}

void server::worker::event_loop() noexcept {
	// TODO: obtain user-passed CQSIZE instead of DEFAULT_QUEUE_DEPTH * 2
	io_uring_cqe *cqes[DEFAULT_QUEUE_DEPTH * 2];

	while (true) {
		int ret = ev.submit_and_wait(1);
		if (ret == -EINTR) {
			continue;
		} else if (ret < 0) {
			fprintf(stderr, "submit and wait failed: %d\n", ret);
			break;
		}

		// TODO: obtain user-passed CQSIZE instead of DEFAULT_QUEUE_DEPTH * 2
		const unsigned int count = ev.peek_batch_cqe(&cqes[0], DEFAULT_QUEUE_DEPTH * 2);
		for (std::size_t i{0}; i < count; ++i) {
			io_uring_cqe *cqe = cqes[i];
			void *coroutine_address = io_uring_cqe_get_data(cqe);
			int res = cqe->res;
			// if (res < 0) {
			// 	fprintf(stderr, "res: %d  (error)\n", res);
			// }

			if (coroutine_address != nullptr) {
				auto handle = std::coroutine_handle<promise>::from_address(coroutine_address);
				handle.promise().awaiter->set_res(res);
				handle.resume();
			} // else {
			  // fprintf(stderr, "res: %d  (timeout/null)\n", res);
			//}
		}

		ev.cq_advance(count);
	}
}
