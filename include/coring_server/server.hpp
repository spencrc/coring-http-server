#pragma once

#include "address.hpp"
#include <vector>
#include <thread>

namespace coring_server {
constexpr unsigned short DEFAULT_PORT = 3000;
constexpr unsigned int DEFAULT_QUEUE_DEPTH = 2048;
constexpr unsigned int DEFAULT_MAX_CONNECTIONS = 512;
constexpr unsigned int DEFAULT_KEEPALIVE_REQUESTS = 100;
constexpr unsigned int DEFAULT_KEEPALIVE_TIMEOUT = 10;

struct server_options {
   	unsigned short port = DEFAULT_PORT;
   	address_format domain = address_format::IPV4;
   	unsigned int queue_depth = DEFAULT_QUEUE_DEPTH; // io_uring queue size
   	unsigned int max_connections = DEFAULT_MAX_CONNECTIONS; // socket listen queue size / kernel backlog
   	unsigned int keepalive_requests = DEFAULT_KEEPALIVE_REQUESTS; // number of requests before fd is closed
   	unsigned int keepalive_timeout = DEFAULT_KEEPALIVE_TIMEOUT; // # of seconds before fd is closed
};

class server {
  public:
	server(address interface, const server_options &opts);
	~server();
	void run();

  private:
    class worker;
	std::vector<worker> workers;
	std::vector<std::jthread> threads;
};
} // namespace coring_server
