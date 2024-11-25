#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "hashtable.h"

// the real hashtable interface.
// it uses 2 hashtables for progressive resizing.
typedef struct HMap {
  HTable ht1; // newer
  HTable ht2; // older
  size_t resizing_pos;
  uint64_t (*hash)(void *);
  uint64_t (*eq)(HNode *, HNode *);
} HMap;

HNode *hmap_lookup(HMap *map, HNode *key);
void hmap_insert(HMap *map, HNode *node);
HNode *hmap_pop(HMap *map, HNode *key);
size_t hmap_size(HMap *map);
void hmap_destroy(HMap *map);
