#include "../include/stor3d.h"
#include "../include/ssd.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define DISK_PATH "/tmp/stor3d_ssd_ftl_test.img"

static int make_disk(void)
{
	int fd;
	char buf[BLOCK_SIZE];
	size_t i;

	fd = open(DISK_PATH, O_RDWR | O_CREAT | O_TRUNC, 0644);
	if (fd < 0)
		return (-1);
	memset(buf, 0, BLOCK_SIZE);
	for (i = 0; i < BLOCK_COUNT; i++)
	{
		if (write(fd, buf, BLOCK_SIZE) != BLOCK_SIZE)
		{
			close(fd);
			return (-1);
		}
	}
	lseek(fd, 0, SEEK_SET);
	return (fd);
}

static int test_allocate_does_not_overwrite_valid(void)
{
	t_ssd_state	*ssd;
	size_t		ppa1;
	size_t		ppa2;
	size_t		i;

	if (ssd_init(&ssd) != 0)
		return (1);
	ppa1 = ssd_allocate_page(ssd);
	if (ppa1 == (size_t) - 1)
	{
		ssd_cleanup(ssd);
		return (1);
	}
	for (i = 0; i < SSD_TOTAL_PAGES - 1; i++)
		ssd_allocate_page(ssd);
	if (ssd->free_pages != 0)
	{
		printf("FAIL: expected free_pages=0 got %zu\n", ssd->free_pages);
		ssd_cleanup(ssd);
		return (1);
	}
	ppa2 = ssd_allocate_page(ssd);
	if (ppa2 != (size_t) - 1)
	{
		printf("FAIL: allocate on full SSD must return -1, got %zu\n", ppa2);
		ssd_cleanup(ssd);
		return (1);
	}
	// page at ppa1 must still be VALID — wrap-around must not have overwritten it
	if (ssd->blocks[ssd_get_block_index(ppa1)].pages[ssd_get_page_index(ppa1)] != SSD_PAGE_VALID)
	{
		printf("FAIL: page at ppa1=%zu was overwritten by wrap-around\n", ppa1);
		ssd_cleanup(ssd);
		return (1);
	}
	ssd_cleanup(ssd);
	return (0);
}

static int test_gc_runs_when_threshold_hit(void)
{
	t_ssd_state	*ssd;
	int			fd;
	char		buf[BLOCK_SIZE];
	size_t		i;
	size_t		gc_before;

	if (ssd_init(&ssd) != 0)
		return (1);
	fd = make_disk();
	if (fd < 0)
	{
		ssd_cleanup(ssd);
		return (1);
	}
	memset(buf, 'a', BLOCK_SIZE);
	for (i = 0; i < BLOCK_COUNT; i++)
		ssd_write(ssd, fd, i % 1024, buf);
	gc_before = ssd->gc_count;
	for (i = 0; i < BLOCK_COUNT * 2; i++)
		ssd_write(ssd, fd, i % 1024, buf);
	if (ssd->gc_count <= gc_before)
	{
		printf("FAIL: GC should have run after sustained writes (gc=%zu)\n", ssd->gc_count);
		close(fd);
		unlink(DISK_PATH);
		ssd_cleanup(ssd);
		return (1);
	}
	if (ssd->erases == 0)
	{
		printf("FAIL: GC should have erased blocks (erases=%zu)\n", ssd->erases);
		close(fd);
		unlink(DISK_PATH);
		ssd_cleanup(ssd);
		return (1);
	}
	close(fd);
	unlink(DISK_PATH);
	ssd_cleanup(ssd);
	return (0);
}

static int test_read_after_write_consistency(void)
{
	t_ssd_state	*ssd;
	int			fd;
	char		w[BLOCK_SIZE];
	char		r[BLOCK_SIZE];

	if (ssd_init(&ssd) != 0)
		return (1);
	fd = make_disk();
	if (fd < 0)
	{
		ssd_cleanup(ssd);
		return (1);
	}
	memset(w, 'X', BLOCK_SIZE);
	if (ssd_write(ssd, fd, 42, w) != 0)
	{
		close(fd);
		unlink(DISK_PATH);
		ssd_cleanup(ssd);
		return (1);
	}
	memset(r, 0, BLOCK_SIZE);
	if (ssd_read(ssd, fd, 42, r) != 0)
	{
		close(fd);
		unlink(DISK_PATH);
		ssd_cleanup(ssd);
		return (1);
	}
	if (memcmp(w, r, BLOCK_SIZE) != 0)
	{
		printf("FAIL: read after write returned different content at lba=42\n");
		close(fd);
		unlink(DISK_PATH);
		ssd_cleanup(ssd);
		return (1);
	}
	close(fd);
	unlink(DISK_PATH);
	ssd_cleanup(ssd);
	return (0);
}

int	main(void)
{
	int	failed;

	failed = 0;
	if (test_allocate_does_not_overwrite_valid() != 0)
	{
		printf("FAIL: allocate-no-overwrite\n");
		failed = 1;
	}
	if (test_read_after_write_consistency() != 0)
	{
		printf("FAIL: read-after-write\n");
		failed = 1;
	}
	if (test_gc_runs_when_threshold_hit() != 0)
	{
		printf("FAIL: gc-trigger\n");
		failed = 1;
	}
	if (!failed)
		printf("PASS: ssd ftl/gc (3 cases)\n");
	return (failed);
}
