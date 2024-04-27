
#ifndef BTREE_HEAVY

#define BTREE_HEAVY
#include <stdint.h>
#include <stdlib.h>

/* elementnum must be odd - pointernum must be even and one greater than elementnum */
#define BTREE_BLOCKSIZE 4096LL
#define BTREE_ELEMENTNUM 255
#define BTREE_POINTERNUM 256
#define BTREE_DATASIZE 511
/* 241 991 9973 49999 99991 */
#define BTREE_CACHE_SIZE 99991
#define BTREE_BANK_SIZE 100000

struct btree { int size; int fptr; struct btree_node * root; struct btree_cache * cache; struct btree_node_bank * bank; };
struct btree_cache { int size; struct skip_list * l; struct ring_buffer * r; };
struct btree_node {  __uint32_t flags; __uint32_t size; size_t pos;  __uint64_t * data; };
struct btree_node_bank { struct btree_node ** store; size_t pos; };

void init_btree(char *, struct btree *);
void close_btree(struct btree *);
int add_btree(__uint64_t, struct btree *);

void in_order_print_btree(struct btree *);

#endif
