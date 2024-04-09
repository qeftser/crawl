
#ifndef HASHES

#define HASHES
#include <stdint.h>

__uint64_t fnv1a_64(char *);
__uint32_t murmer3_32(char *, __uint32_t);
__uint64_t siphash_2_4_128(char *, __uint64_t, __uint64_t);
__uint128_t md5(char *); /* only supports 448 character input sizes or less */

#endif
