
#ifndef BTREE_HEAVY

#define BTREE_HEAVY
#include <stdint.h>
#include <stdlib.h>

/* elementnum must be odd - pointernum must be even and one greater than elementnum */
#define BTREE_BLOCKSIZE 4096L
#define BTREE_ELEMENTNUM 3
#define BTREE_POINTERNUM 4
#define BTREE_DATASIZE 7
#define BTREE_CACHE_SIZE 5
#define BTREE_BANK_SIZE 8

struct btree { int size; int fptr; struct btree_node * root; struct btree_cache * cache; struct btree_node_bank * bank; };
struct btree_cache { int size; struct skip_list * l; struct ring_buffer * r; };
struct btree_node {  __uint32_t flags; __uint32_t size; size_t pos;  __uint64_t * data; };
struct btree_node_bank { struct btree_node ** store; size_t pos; };

#endif
