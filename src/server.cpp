#include "coring_server/server.hpp"
#include "worker.hpp"

using namespace coring_server;

server::server(address interface, const server_options &opts) {
	const unsigned int num_threads = std::thread::hardware_concurrency();
	workers.reserve(num_threads);
	threads.reserve(num_threads);
	for (std::size_t i{0}; i < num_threads; ++i) {
		workers.emplace_back(interface, opts);
	}
}

server::~server() = default;

void server::run() {
	for (auto &worker : workers) {
		threads.emplace_back([&worker]() { worker.run(); });
	}

	for (auto &thread : threads) {
		thread.join();
	}
}
