#include "coring_server/server.hpp"

int main() {
    constexpr unsigned short PORT = 3123;
	coring_server::server s(
		coring_server::address::ANY,
		coring_server::server_options{
			.port = PORT,
			.domain = coring_server::address_format::IPV4,
		}
	);
	s.run();

	return 0;
}
