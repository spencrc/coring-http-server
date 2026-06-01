# Coring HTTP Server

## Dependencies
This project requires the following packages to be installed on your system:
- [clang](https://clang.llvm.org/)
- [liburing-dev](https://packages.debian.org/sid/liburing-dev) (or [liburing-devel](https://rpmfind.net/linux/rpm2html/search.php?query=liburing-devel))

Since liburing is required, you will not be able to run the project without [Docker](https://www.docker.com/) or [Podman](https://podman.io/) on a non-Unix operating system.

## Building
To build the project and create a binary file you can run in `bin/release/`, you can simply run:
```shell
make all
```

If you want all warnings, no optimization, and asan enabled for your binary in `bin/debug/`, you can run:
```shell
make debug
```

To clean the `bin/` directory, just run:
```shell
make clean
```
