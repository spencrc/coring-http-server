#include <netinet/in.h>

namespace coring_server {
    enum class address : unsigned int {
        LOOPBACK = INADDR_LOOPBACK,
        ANY = INADDR_ANY,
        BROADCAST = INADDR_BROADCAST,
    };

    enum class address_format : unsigned long {
        IPV4 = AF_INET,
        IPV6 = AF_INET6,
        UNIX = AF_UNIX,
    };
} // namespace coring_server
