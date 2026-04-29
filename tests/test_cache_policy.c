#include "../include/stor3d.h"
#include "../include/cache_policy.h"
#include <stdio.h>
#include <string.h>

#define ASSERT_MSG(cond, msg) do { \
	if (!(cond)) { fprintf(stderr, "ASSERT: %s (%s:%d)\n", msg, __FILE__, __LINE__); return 1; } \
} while (0)

int main(void)
{
	const cache_policy_t *lru;
	const cache_policy_t *fifo;
	const cache_policy_t *fallback;
	const cache_policy_t *unknown;

	lru = cache_policy_by_name("lru");
	fifo = cache_policy_by_name("fifo");
	fallback = cache_policy_by_name(NULL);
	unknown = cache_policy_by_name("nope");

	ASSERT_MSG(lru && strcmp(lru->name, "lru") == 0, "lru lookup");
	ASSERT_MSG(fifo && strcmp(fifo->name, "fifo") == 0, "fifo lookup");
	ASSERT_MSG(fallback && strcmp(fallback->name, "lru") == 0, "NULL defaults to lru");
	ASSERT_MSG(unknown == NULL, "unknown policy returns NULL");
	ASSERT_MSG(lru->select_victim != NULL, "lru.select_victim non-null");
	ASSERT_MSG(lru->on_access != NULL, "lru.on_access non-null");
	ASSERT_MSG(fifo->select_victim != NULL, "fifo.select_victim non-null");

	t_hdd_state *hdd = NULL;
	if (hdd_init(&hdd) != 0) return 1;
	/* mark all slots invalid; victim should be slot 0 (lowest last_access) */
	for (size_t i = 0; i < HDD_CACHE_SIZE; i++) {
		hdd->cache[i].valid = 1;
		hdd->cache[i].lba = i;
		hdd->cache[i].last_access = i;
	}
	size_t v = lru->select_victim(hdd);
	ASSERT_MSG(v == 0, "lru victim is slot with min last_access");

	hdd_cleanup(hdd);
	printf("PASS: cache policy SPI (7 cases)\n");
	return 0;
}
