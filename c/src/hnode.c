#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "hnode.h"

HNode *hnode_new(uint64_t hash) {
  HNode *node = malloc(sizeof(HNode));
  node->next = NULL;
  node->hash = hash;
  return node;
}

// from is pointer to location storing ref to node
// so we can safely destroy the node without having a
// dangling pointer
HNode *hnode_detach(HNode **from) {
  HNode *node = *from;
  *from = node->next;
  node->next = NULL;
  return node;
}

void hnode_destroy(HNode **from) {
  HNode *node = hnode_detach(from);
  free(node);
}

void hnode_destroy_chain(HNode **from) {
  while (*from) {
    hnode_destroy(from);
  }
}
