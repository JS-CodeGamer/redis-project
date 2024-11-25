#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// hashtable node, should be embedded into the payload
typedef struct HNode {
  struct HNode *next;
  uint64_t hash;
} HNode;

HNode *hnode_new(uint64_t hash);
HNode *hnode_detach(HNode **from);
void hnode_destroy(HNode **from);
void hnode_destroy_chain(HNode **from);
