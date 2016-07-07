/*
 * libdfu, usage sample (programming the stm32 via serial port under linux)
 * Author Davide Ciminaghi, 2016
 * Public domain
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <dfu.h>
#include <dfu-linux.h>
#include <dfu-stm32.h>

static void help(int argc, char *argv[])
{
	fprintf(stderr, "Use %s <fname> <serial_port>\n", argv[0]);
}

static void *map_file(const char *path, size_t len)
{
	int fd = open(path, O_RDONLY);
	void *out;

	if (fd < 0) {
		perror("open");
		return NULL;
	}
	out = mmap(NULL, len, PROT_READ, MAP_PRIVATE, fd, 0);
	if (out == MAP_FAILED) {
		perror("mmap");
		return NULL;
	}
	close(fd);
	return out;
}

int main(int argc, char *argv[])
{
	const char *fpath;
	char *port;
	int ret;
	struct stat s;
	struct dfu_data *dfu;
	struct dfu_binary_file *f;
	void *ptr;
	

	if (argc < 3) {
		help(argc, argv);
		exit(127);
	}
	fpath = argv[1];
	port = argv[2];

	/* Check whether file and port exist */
	ret = stat(fpath, &s);
	if (ret < 0) {
		perror("stat");
		exit(127);
	}
	dfu = dfu_init(&linux_serial_interface_ops,
		       port,
		       NULL,
		       &stm32_dfu_target_ops,
		       &linux_dfu_host_ops);
	if (!dfu) {
		fprintf(stderr, "Error initializing libdfu\n");
		exit(127);
	}
	ptr = map_file(fpath, s.st_size);
	f = dfu_new_binary_file(ptr, s.st_size, s.st_size, dfu, 0);
	if (!f) {
		fprintf(stderr, "Error setting up binary file struct\n");
		exit(127);
	}
	if (dfu_target_reset(dfu) < 0) {
		fprintf(stderr, "Error resetting target\n");
		exit(127);
	}
	/* Start programming data */
	if (dfu_binary_file_flush_start(f) < 0) {
		fprintf(stderr, "Error programming file\n");
		exit(127);
	}
	/* Loop around waiting for events */
	while (!dfu_binary_file_written(f))
		dfu_idle(dfu);
	/* Let target run */
	exit(dfu_target_go(dfu));
}
