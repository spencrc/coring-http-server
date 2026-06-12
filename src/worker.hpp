#pragma once

#include "io_uring_ctx.hpp"
#include "coring_server/server.hpp"
#include "socket.hpp"
#include "task.hpp"

namespace coring_server {
class server::worker {
  public:
	worker(address interface, const server_options &opts);
	void run();

  private:
	socket sock;
	std::optional<io_uring_ctx> ev;
	unsigned short port;
	address interface;
	unsigned int max_connections;
	unsigned int keepalive_requests;
	unsigned int keepalive_timeout;
	unsigned int queue_depth;
	void event_loop() noexcept;
	task<promise> serve_async() noexcept;
	task<promise> handle_connection_async(int clientfd) noexcept;
};
}; // namespace coring_server
