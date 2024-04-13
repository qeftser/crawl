
#ifndef BTREE_HEAVY

#define BTREE_HEAVY
#include <stdint.h>
#include <stdlib.h>

#define BTREE_BLOCKSIZE 4096
#define BTREE_ELEMENTNUM 255
#define BTREE_POINTERNUM 256

struct btree { int size; struct btree_node * root; struct btree_cache * cache; };
struct btree_cache { int size; struct skip_list * l; struct ring_buffer * r; };
struct btree_node {  __uint8_t flags; __uint16_t size; size_t pos;  __uint64_t * data; };
struct btree_node_bank { struct btree_node ** store; size_t pos; };

#endif
