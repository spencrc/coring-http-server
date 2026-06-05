#include "constants.hpp"
#include <liburing.h>
#include "server.hpp"

int main() {
    const coring_server::server_options opts{
        .port = PORT,
        .interface = INADDR_ANY,
        .queue_depth = QUEUE_DEPTH,
        .domain = AF_INET,
        .type = SOCK_STREAM | SOCK_NONBLOCK,
        .protocol = IPPROTO_TCP,
        .max_connections = KERNEL_BACKLOG,
    };

    coring_server::server s(opts);
    s.run();

	return 0;
}
