#include <array>
#include <cstdint>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <linux/openat2.h>
#include <queue>
#include <sys/syscall.h>
#include <system_error>
#include <unistd.h>

struct linux_dirent {
	uint64_t d_ino;			 /* 64-bit Inode number */
	int64_t d_off;			 /* 64-bit Offset to next structure */
	unsigned short d_reclen; /* Size of this entire dirent */
	unsigned char d_type;	 /* File type (placed cleanly right here!) */
	char d_name[];			 /* Filename (null-terminated) */
};

using directory_info = std::pair<int, std::string>;

int main() {
	const open_how how{
		.flags = O_DIRECTORY,
		.mode = O_RDONLY,
		.resolve = RESOLVE_BENEATH | RESOLVE_NO_SYMLINKS | RESOLVE_NO_XDEV,
	};

	std::queue<directory_info> q;
	std::vector<int> dir_fds;
	const std::string root_name = "static";

	int root = syscall(SYS_openat2, AT_FDCWD, root_name.c_str(), &how, sizeof(how));
	if (root < 0)
		throw std::system_error(errno, std::generic_category(),
								"Failed to open directory");
	q.emplace(root, root_name);

	while (q.empty() == false) {
		auto [fd, path] = q.front();
		q.pop();
		dir_fds.push_back(fd);

		std::array<char, 1024> buf;
		std::size_t nread = syscall(SYS_getdents64, fd, buf.data(), buf.size());
		for (auto bpos = 0uz; bpos < nread;) {
			linux_dirent *d = (linux_dirent *)(buf.data() + bpos);
			bpos += d->d_reclen;

			if (std::strcmp(d->d_name, ".") == 0 or std::strcmp(d->d_name, "..") == 0) continue;
			if (d->d_type == DT_DIR) {
				int child = syscall(SYS_openat2, fd, d->d_name, &how, sizeof(how));
				if (child < 0)
					throw std::system_error(errno, std::generic_category(),
											"Failed to open subdirectory");
				q.emplace(child, path + "/" + d->d_name);
			}

			printf("%8ld  ", d->d_ino);
			printf("%-10s ", (d->d_type == DT_REG) ? "regular" : (d->d_type == DT_DIR) ? "directory"
															 : (d->d_type == DT_FIFO)  ? "FIFO"
															 : (d->d_type == DT_SOCK)  ? "socket"
															 : (d->d_type == DT_LNK)   ? "symlink"
															 : (d->d_type == DT_BLK)   ? "block dev"
															 : (d->d_type == DT_CHR)   ? "char dev"
																					   : "???");
			printf("%4d %10lld  %s\n", d->d_reclen,
				   (long long)d->d_off, d->d_name);
		}
	}

	while (dir_fds.empty() == false) {
		int fd = dir_fds.back();
		dir_fds.pop_back();
		close(fd);
	}

	return 0;
}
