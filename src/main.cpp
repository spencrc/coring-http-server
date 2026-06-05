#include "constants.hpp"
#include "coring_server/server.hpp"
#include "coring_server/socket.hpp"
#include <liburing.h>

int main() {
	const coring_server::server_options opts{
		.port = PORT,
		.interface = coring_server::address::ANY,
		.domain = coring_server::address_family::IPV4,
		.queue_depth = QUEUE_DEPTH,
		.max_connections = KERNEL_BACKLOG,
	};

	coring_server::server s(opts);
	s.run();

	return 0;
}
