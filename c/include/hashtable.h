#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "hnode.h"

// a simple fixed-sized hashtable
typedef struct HTable {
  HNode **tab;
  size_t mask;
  size_t size;
  uint64_t (*hash)(void *);
  uint64_t (*eq)(HNode *, HNode *);
} HTable;

void htable_initialize(HTable *table, size_t capacity, uint64_t (*hash)(void *),
                       uint64_t (*eq)(HNode *, HNode *));
HTable *htable_new(size_t capacity, uint64_t (*hash)(void *),
                   uint64_t (*eq)(HNode *, HNode *));
HNode **htable_lookup(HTable *table, HNode *key);
void htable_insert(HTable *table, HNode *node);
HNode *htable_insert_hash(HTable *table, uint64_t hash);
HNode *htable_detach(HTable *table, HNode **from);
HNode *htable_pop(HTable *table, HNode *key);
size_t htable_size(HTable *table);
void htable_destroy(HTable *table);
size_t htable_load_factor(HTable *table);
