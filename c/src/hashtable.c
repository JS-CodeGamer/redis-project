#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "hashtable.h"
#include "hnode.h"

void htable_initialize(HTable *table, size_t capacity, uint64_t (*hash)(void *),
                       uint64_t (*eq)(HNode *, HNode *)) {
  if (capacity <= 0) {
    capacity = 4;
  }
  if (capacity & (capacity - 1)) {
    while (capacity & (capacity - 1)) {
      capacity &= capacity - 1;
    }
    if (capacity << 1) {
      capacity <<= 1;
    }
  }
  *table = (HTable){.mask = capacity - 1,
                    .size = 0,
                    .tab = calloc(capacity, sizeof(HNode *)),
                    .hash = hash,
                    .eq = eq};
}

HTable *htable_new(size_t capacity, uint64_t (*hash)(void *),
                   uint64_t (*eq)(HNode *, HNode *)) {
  HTable *table = malloc(sizeof(HTable));
  htable_initialize(table, capacity, hash, eq);
  return table;
}

HNode **htable_lookup(HTable *table, HNode *key) {
  if (!table->tab) {
    return NULL;
  }

  size_t pos = key->hash & table->mask;
  HNode **from = &table->tab[pos];
  for (HNode *curr; (curr = *from) != NULL; from = &curr->next) {
    if (table->eq(curr, key)) {
      return from;
    }
  }
  return NULL;
}

void htable_insert(HTable *table, HNode *node) {
  size_t pos = node->hash & table->mask;
  node->next = table->tab[pos];
  table->tab[pos] = node;
  table->size++;
}

HNode *htable_insert_hash(HTable *table, uint64_t hash) {
  HNode *node = hnode_new(hash);
  htable_insert(table, node);
  return node;
}

HNode *htable_detach(HTable *table, HNode **from) {
  HNode *node = hnode_detach(from);
  table->size--;
  return node;
}

HNode *htable_pop(HTable *table, HNode *key) {
  HNode **from = htable_lookup(table, key);
  if (from) {
    return htable_detach(table, from);
  }
  return NULL;
}

size_t htable_size(HTable *table) { return table->size; }

void htable_destroy(HTable *table) {
  for (size_t ind = 0; ind < table->size; ind++) {
    hnode_destroy_chain(&table->tab[ind]);
  }
  free(table->tab);
  free(table);
}

size_t htable_load_factor(HTable *table) {
  return table->size / (table->mask + 1);
}
