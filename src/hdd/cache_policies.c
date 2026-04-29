#include "cache_policy.h"

#include <string.h>

static size_t lru_select_victim(t_hdd_state *hdd)
{
	size_t i;
	size_t lru_idx;
	size_t min_access;

	lru_idx = 0;
	min_access = hdd->cache[0].last_access;
	i = 1;
	while (i < HDD_CACHE_SIZE)
	{
		if (hdd->cache[i].last_access < min_access)
		{
			min_access = hdd->cache[i].last_access;
			lru_idx = i;
		}
		i++;
	}
	return lru_idx;
}

static void lru_touch(t_hdd_state *hdd, size_t idx)
{
	hdd->cache[idx].last_access = hdd->cache_clock++;
}

const cache_policy_t LRU_POLICY = {
	.name = "lru",
	.select_victim = lru_select_victim,
	.on_access = lru_touch,
	.on_insert = lru_touch,
};

static size_t fifo_select_victim(t_hdd_state *hdd)
{
	/* FIFO: evict the entry with the smallest last_access set at insert time
	 * (on_access is a no-op so insertion order is preserved). */
	return lru_select_victim(hdd);
}

static void fifo_no_op(t_hdd_state *hdd, size_t idx)
{
	(void)hdd;
	(void)idx;
}

static void fifo_on_insert(t_hdd_state *hdd, size_t idx)
{
	hdd->cache[idx].last_access = hdd->cache_clock++;
}

const cache_policy_t FIFO_POLICY = {
	.name = "fifo",
	.select_victim = fifo_select_victim,
	.on_access = fifo_no_op,
	.on_insert = fifo_on_insert,
};

const cache_policy_t *cache_policy_by_name(const char *name)
{
	if (!name)
		return &LRU_POLICY;
	if (strcmp(name, "lru") == 0)
		return &LRU_POLICY;
	if (strcmp(name, "fifo") == 0)
		return &FIFO_POLICY;
	return NULL;
}
