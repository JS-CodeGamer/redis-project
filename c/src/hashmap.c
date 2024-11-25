#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "hashmap.h"
#include "hashtable.h"

const size_t HMAP__MAX_LOAD_FACTOR = 8;
const size_t HMAP__RESIZIN_WORK = 128; // constant work

void hmap_initialize(HMap *map, uint64_t (*hash)(void *),
                     uint64_t (*eq)(HNode *, HNode *)) {
  map->hash = hash;
  map->eq = eq;
  htable_initialize(&map->ht1, 4, hash, eq);
}

static void hmap_help_resizing(HMap *map) {
  size_t nwork = 0;
  while (nwork < HMAP__RESIZIN_WORK && map->ht2.size > 0) {
    // scan for nodes from ht2 and move them to ht1
    HNode **from = &map->ht2.tab[map->resizing_pos];
    if (!*from) {
      map->resizing_pos++;
      continue;
    }

    htable_insert(&map->ht1, htable_detach(&map->ht2, from));
    nwork++;
  }

  if (map->ht2.size == 0 && map->ht2.tab) {
    // done
    free(map->ht2.tab);
    map->ht2 = (HTable){0};
  }
}

static void hmap_start_resizing(HMap *map) {
  assert(map->ht2.tab == NULL);
  // create a bigger hashtable and swap them
  map->ht2 = map->ht1;
  htable_initialize(&map->ht1, (map->ht1.mask + 1) * 2, map->hash, map->eq);
  map->resizing_pos = 0;
}

HNode *hmap_lookup(HMap *map, HNode *key) {
  hmap_help_resizing(map);
  HNode **from = htable_lookup(&map->ht1, key);
  from = from ? from : htable_lookup(&map->ht2, key);
  return from ? *from : NULL;
}

void hmap_insert(HMap *map, HNode *node) {
  if (!map->ht1.tab) {
    htable_initialize(&map->ht1, 4, map->hash, map->eq);
  }
  htable_insert(&map->ht1, node);

  if (!map->ht2.tab) {
    // check whether we need to resize
    if (htable_load_factor(&map->ht1) >= HMAP__MAX_LOAD_FACTOR) {
      hmap_start_resizing(map);
    }
  }
  hmap_help_resizing(map);
}

HNode *hmap_pop(HMap *map, HNode *key) {
  hmap_help_resizing(map);
  HNode **from = htable_lookup(&map->ht1, key);
  if (from) {
    return htable_detach(&map->ht1, from);
  }
  from = htable_lookup(&map->ht2, key);
  if (from) {
    return htable_detach(&map->ht2, from);
  }
  return NULL;
}

size_t hmap_size(HMap *map) {
  return htable_size(&map->ht1) + htable_size(&map->ht2);
}

void hmap_destroy(HMap *map) {
  free(map->ht1.tab);
  free(map->ht2.tab);
  *map = (HMap){0};
}
