#include <array>
#include <dirent.h>
#include <fcntl.h>
#include <linux/openat2.h>
#include <print>
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
int main() {
	const open_how how{
		.flags = O_DIRECTORY,
		.mode = O_RDONLY,
		.resolve = RESOLVE_BENEATH | RESOLVE_NO_SYMLINKS | RESOLVE_NO_XDEV,
	};
	int fd = openat2(AT_FDCWD, "static", &how, sizeof(how));
	if (fd < 0)
		throw std::system_error(errno, std::generic_category(),
								"Failed to open directory");

	std::array<char, 1024> buf;
	std::size_t nread = syscall(SYS_getdents64, fd, buf.data(), buf.size());
	for (auto bpos = 0uz; bpos < nread;) {
		linux_dirent *d = (linux_dirent *)(buf.data() + bpos);
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
		bpos += d->d_reclen;
	}
	return 0;
}