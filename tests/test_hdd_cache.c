#include "../include/stor3d.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define ASSERT_MSG(cond, msg) do { \
	if (!(cond)) { fprintf(stderr, "ASSERT FAILED: %s (%s:%d)\n", msg, __FILE__, __LINE__); abort(); } \
} while (0)

static int make_disk(const char *path)
{
	int fd;
	char buf[BLOCK_SIZE];
	size_t i;

	fd = open(path, O_RDWR | O_CREAT | O_TRUNC, 0644);
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

int main(void)
{
	t_hdd_state *hdd;
	int fd;
	char buf[BLOCK_SIZE];
	int failed = 0;

	if (hdd_init(&hdd) != 0)
	{
		printf("FAIL: hdd_init\n");
		return (1);
	}
	fd = make_disk("/tmp/stor3d_test_disk.img");
	if (fd < 0)
	{
		printf("FAIL: disk image create\n");
		hdd_cleanup(hdd);
		return (1);
	}

	if (hdd_read(hdd, fd, 0, buf) != 0)
	{
		printf("FAIL: read lba=0\n");
		failed = 1;
	}
	if (hdd->cache_misses != 1 || hdd->cache_hits != 0)
	{
		printf("FAIL: first read should be a miss got hits=%zu misses=%zu\n",
			hdd->cache_hits, hdd->cache_misses);
		failed = 1;
	}

	if (hdd_read(hdd, fd, 0, buf) != 0)
	{
		printf("FAIL: re-read lba=0\n");
		failed = 1;
	}
	if (hdd->cache_hits != 1 || hdd->cache_misses != 1)
	{
		printf("FAIL: second read should be a hit got hits=%zu misses=%zu\n",
			hdd->cache_hits, hdd->cache_misses);
		failed = 1;
	}

	if (hdd_write(hdd, fd, 0, buf) != 0)
	{
		printf("FAIL: write lba=0\n");
		failed = 1;
	}
	if (hdd_read(hdd, fd, 0, buf) != 0)
	{
		printf("FAIL: read after write\n");
		failed = 1;
	}
	if (hdd->cache_misses != 2)
	{
		printf("FAIL: write should invalidate cache (misses=%zu)\n", hdd->cache_misses);
		failed = 1;
	}

	if (hdd->total_reads != 3 || hdd->total_writes != 1)
	{
		printf("FAIL: total counters reads=%zu writes=%zu\n",
			hdd->total_reads, hdd->total_writes);
		failed = 1;
	}

	// regression: LRU eviction must mark new entry valid (was a real bug — stayed valid=0)
	{
		t_hdd_state *h2 = NULL;
		int fd2;
		hdd_init(&h2);
		fd2 = open("/tmp/stor3d_test_disk.img", O_RDWR | O_CREAT | O_TRUNC, 0644);
		if (fd2 >= 0)
		{
			char zero[BLOCK_SIZE]; memset(zero, 0, BLOCK_SIZE);
			for (size_t k = 0; k < BLOCK_COUNT; k++) write(fd2, zero, BLOCK_SIZE);
			lseek(fd2, 0, SEEK_SET);
			// fill cache + force LRU eviction by reading HDD_CACHE_SIZE+1 distinct lbas
			for (size_t k = 0; k <= HDD_CACHE_SIZE; k++) hdd_read(h2, fd2, k, zero);
			// every cache slot must now be valid (after eviction filled them all)
			int all_valid = 1;
			for (size_t k = 0; k < HDD_CACHE_SIZE; k++)
				if (!h2->cache[k].valid) { all_valid = 0; break; }
			ASSERT_MSG(all_valid, "after LRU eviction every cache entry must be valid");
			close(fd2);
		}
		hdd_cleanup(h2);
	}

	unlink("/tmp/stor3d_test_disk.img");
	hdd_cleanup(hdd);

	if (!failed)
		printf("PASS: hdd cache miss/hit/invalidate/lru-valid (5 cases)\n");
	return (failed);
}
