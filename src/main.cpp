#include "socket.hpp"
#include "coro.hpp"

constexpr unsigned short PORT = 3002;
constexpr unsigned int KERNEL_BACKLOG = 128;
constexpr unsigned int QUEUE_DEPTH = 4096;

// ConnectionCoroutine readAsync(int clientfd) noexcept {
//     std::string data = co_await RecvAwaiter(clientfd);
//     std::cout << "Message from client: " << data
//               << std::endl;
// } 

ConnectionCoroutine serveAsync(int sockfd, io_uring* ring) noexcept {
    while (true) {
        co_await AcceptAwaiter(sockfd, ring);
        
        // do more
    }
}

void eventLoop(io_uring* ring) noexcept {
    io_uring_cqe *cqe = nullptr;

    while (true) {
        io_uring_wait_cqe(ring, &cqe);
        void *data = io_uring_cqe_get_data(cqe);
        auto handle = std::coroutine_handle<>::from_address(data);
        handle.resume();
        io_uring_cqe_seen(ring, cqe);
    }
}

int main() {
    io_uring ring{};
    io_uring_queue_init(QUEUE_DEPTH, &ring, 0);

    auto sock = Socket(AF_INET, SOCK_STREAM, 0);

    constexpr unsigned int flag = 1;
    sock.setsockopt(SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
    sock.setsockopt(SOL_SOCKET, SO_REUSEPORT, &flag, sizeof(flag));

    sock.bind(PORT, INADDR_ANY);
    sock.listen(KERNEL_BACKLOG);

    serveAsync(sock.getFd(), &ring);

    eventLoop(&ring);

    io_uring_queue_exit(&ring);
    
    return 0;
}