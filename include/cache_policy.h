#ifndef CACHE_POLICY_H
#define CACHE_POLICY_H

#include <stddef.h>
#include "hdd.h"

/*
 * Pluggable cache replacement policy. Lets the HDD cache use LRU (default),
 * FIFO, MRU, or 2Q without changing hdd_cache.c.
 *
 * Invariants:
 *  - select_victim() must return an index in [0, HDD_CACHE_SIZE)
 *  - implementations are stateless w.r.t. the cache (read state from t_hdd_state)
 *  - on_access updates policy bookkeeping (e.g., LRU updates last_access)
 *  - on_insert is called after a new entry is placed at idx
 */
typedef struct {
	const char *name;
	size_t (*select_victim)(t_hdd_state *hdd);
	void (*on_access)(t_hdd_state *hdd, size_t idx);
	void (*on_insert)(t_hdd_state *hdd, size_t idx);
} cache_policy_t;

extern const cache_policy_t LRU_POLICY;
extern const cache_policy_t FIFO_POLICY;

const cache_policy_t *cache_policy_by_name(const char *name);

#endif
