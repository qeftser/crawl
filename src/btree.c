
#include "btree.h"
#include "skiplist.h"
#include "ringbuffer.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

void init_btree_cache(size_t size, struct btree_cache * c) {
   struct skip_list_bank * b = malloc(sizeof(struct skip_list_bank));
   struct skip_list  * l = malloc(sizeof(struct skip_list));
   struct ring_buffer * r = malloc(sizeof(struct ring_buffer));
   init_skip_list(size,b,l);
   init_ring_buffer(size,r);
   c->l = l;
   c->r = r;
   c->size = size;
}

void destroy_btree_cache(struct btree_cache * c) {
   destroy_ring_buffer(c->r);
   struct skip_list_bank * b = c->l->bank;
   destroy_skip_list(c->l);
   destroy_ring_buffer(c->r);
   free(b);
   free(c->l);
   free(c->r);
}

void init_btree_node_bank(size_t size, struct btree_node_bank * b) {
   b->store = malloc(sizeof(struct btree_node *)*size);
   b->pos = size;
   struct btree_node * curr;
   for (size_t i = 0; i < size; i++) {
      curr = malloc(sizeof(struct btree_node));
      memset(curr,0,sizeof(__uint8_t)+sizeof(__uint16_t)+sizeof(size_t));
      curr->data = malloc(sizeof(__uint64_t)*(BTREE_ELEMENTNUM+BTREE_POINTERNUM));
   }
}

void destroy_btree_node_bank(struct btree_node_bank * b) {
   struct btree_node * curr;
   for (size_t i = 0; i < b->pos; i++) {
      curr = b->store[i];
      free(curr->data);
      free(curr);
   }
}


