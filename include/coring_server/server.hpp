#pragma once

#include "evented.hpp"
#include "socket.hpp"
#include "task.hpp"

namespace coring_server {
    struct server_options {
    	unsigned short port = 3000;
    	address interface = address::ANY;
    	address_family domain = address_family::IPV4;
    	unsigned int queue_depth = 2048; // io_uring queue size
    	unsigned int max_connections = 512; // socket listen queue size / kernel backlog
    };

    class server {
    public:
    	server(const server_options opts);
    	void run();

    private:
    	socket sock;
    	evented ev;
    	unsigned short port;
    	address interface;
    	unsigned int max_connections;
    	void event_loop() noexcept;
    	task<promise> serve_async() noexcept;
    	task<promise> handle_connection_async(int clientfd) noexcept;
    };
} // namespace coring_server
