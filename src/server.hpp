#pragma once

#include "socket.hpp"
#include "evented.hpp"
#include "task.hpp"

namespace coring_server {
    struct server_options {
        unsigned short port;
        unsigned long interface;
        unsigned int queue_depth;
        int domain;
        int type;
        int protocol;
        unsigned int max_connections;
    };

    class server {
    public:
        server(const server_options opts);
        void run();
    private:
        Socket sock;
        evented ev;
        unsigned short port;
        unsigned long interface;
        unsigned int max_connections;
        void eventLoop() noexcept;
        Task<Promise> serveAsync() noexcept;
        Task<Promise> handleConnectionAsync(int clientfd) noexcept;
    };
}
