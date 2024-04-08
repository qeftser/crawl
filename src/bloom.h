
#ifndef BLOOM_FILTER

#define BLOOM_FILTER
#include <stdint.h>
#include <stdlib.h>

__uint8_t check_bloom(char *, __uint8_t *);
void insert_bloom(char * r, __uint8_t *);

void init_bloom(__uint8_t *, size_t);
int load_bloom(char *, __uint8_t *);
int unload_bloom(char *, __uint8_t *);

#endif
