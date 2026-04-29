#include "../include/stor3d.h"
#include "../include/ssd.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define DISK_PATH "/tmp/stor3d_ssd_writes_test.img"

#define ASSERT_MSG(cond, msg) do { \
	if (!(cond)) { fprintf(stderr, "ASSERT: %s (%s:%d)\n", msg, __FILE__, __LINE__); abort(); } \
} while (0)

static int make_disk(void)
{
	int fd;
	char buf[BLOCK_SIZE];
	size_t i;

	fd = open(DISK_PATH, O_RDWR | O_CREAT | O_TRUNC, 0644);
	if (fd < 0) return (-1);
	memset(buf, 0, BLOCK_SIZE);
	for (i = 0; i < BLOCK_COUNT; i++)
		if (write(fd, buf, BLOCK_SIZE) != BLOCK_SIZE) { close(fd); return (-1); }
	lseek(fd, 0, SEEK_SET);
	return (fd);
}

static int test_distinct_lba_writes_preserve_data(void)
{
	t_ssd_state *ssd = NULL;
	int fd;
	char w[BLOCK_SIZE];
	char r[BLOCK_SIZE];
	size_t i;

	ASSERT_MSG(ssd_init(&ssd) == 0, "ssd_init");
	fd = make_disk();
	ASSERT_MSG(fd >= 0, "disk make");
	for (i = 0; i < 64; i++)
	{
		memset(w, (char)('a' + (i % 26)), BLOCK_SIZE);
		ASSERT_MSG(ssd_write(ssd, fd, i, w) == 0, "ssd_write");
	}
	for (i = 0; i < 64; i++)
	{
		memset(w, (char)('a' + (i % 26)), BLOCK_SIZE);
		memset(r, 0, BLOCK_SIZE);
		ASSERT_MSG(ssd_read(ssd, fd, i, r) == 0, "ssd_read");
		ASSERT_MSG(memcmp(w, r, BLOCK_SIZE) == 0, "data mismatch");
	}
	close(fd);
	unlink(DISK_PATH);
	ssd_cleanup(ssd);
	return (0);
}

static int test_overwrite_same_lba_invalidates_prev(void)
{
	t_ssd_state *ssd = NULL;
	int fd;
	char buf[BLOCK_SIZE];
	int total_invalid;
	int i;

	ASSERT_MSG(ssd_init(&ssd) == 0, "ssd_init");
	fd = make_disk();
	ASSERT_MSG(fd >= 0, "disk make");
	memset(buf, 'A', BLOCK_SIZE);
	for (i = 0; i < 5; i++)
		ASSERT_MSG(ssd_write(ssd, fd, 7, buf) == 0, "ssd_write");
	total_invalid = 0;
	for (i = 0; i < SSD_ERASE_BLOCK_COUNT; i++)
		total_invalid += ssd->blocks[i].invalid_count;
	ASSERT_MSG(total_invalid >= 4, "rewrites should invalidate previous PPAs");
	close(fd);
	unlink(DISK_PATH);
	ssd_cleanup(ssd);
	return (0);
}

int	main(void)
{
	if (test_distinct_lba_writes_preserve_data() != 0) return (1);
	if (test_overwrite_same_lba_invalidates_prev() != 0) return (1);
	printf("PASS: ssd writes/invalidate (2 cases)\n");
	return (0);
}
