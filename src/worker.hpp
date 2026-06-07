#pragma once

#include "coring_server/io_uring_ctx.hpp"
#include "coring_server/server.hpp"
#include "coring_server/socket.hpp"
#include "coring_server/task.hpp"

namespace coring_server {
class server::worker {
  public:
	worker(address interface, const server_options &opts);
	void run();

  private:
	socket sock;
	io_uring_ctx ev;
	unsigned short port;
	address interface;
	unsigned int max_connections;
	unsigned int keepalive_requests;
	unsigned int keepalive_timeout;
	void event_loop() noexcept;
	task<promise> serve_async() noexcept;
	task<promise> handle_connection_async(int clientfd) noexcept;
};
}; // namespace coring_server
