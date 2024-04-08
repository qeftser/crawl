
#ifndef BTREE_STORAGE

#define BTREE_STORAGE
#include <stdint.h>

#define BTREE_HASH_SIZE 991

struct btree_node {  __uint64_t pos; __uint16_t size; __uint64_t elements[255]; __uint64_t pointers[256]; };
struct btree_hash { struct btree_node hash[BTREE_HASH_SIZE]; };
struct btree { int fptr; int size; struct btree_node root; struct btree_hash * hash; };


#endif
