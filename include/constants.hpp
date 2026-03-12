#pragma once

constexpr unsigned short PORT = 3123;
constexpr unsigned int KERNEL_BACKLOG = 512; // socket listen queue size
constexpr unsigned int QUEUE_DEPTH = 2048;	 // io_uring queue size
constexpr unsigned int BUFFER_LEN = 1024;
constexpr unsigned int KEEPALIVE_REQUESTS = 100;
constexpr unsigned int KEEPALIVE_TIMEOUT = 10;