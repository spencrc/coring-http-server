#include "socket.hpp"
#include <array>
#include <iostream>

constexpr unsigned short PORT = 3002;

int main() {
    auto sock = Socket(AF_INET, SOCK_STREAM, 0, PORT, INADDR_ANY);

    sock.listen(128);

    int clientfd = sock.accept();

    std::array<char, 1024> buffer{};
    recv(clientfd, &buffer, buffer.size(), 0);
    std::cout << "Message from client: " << buffer.data()
              << std::endl;

    return 0;
}