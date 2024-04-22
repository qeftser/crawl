
#ifndef DISKQUEUE_GENERIC

#define DISKQUEUE_GENERIC

#define DISKQUEUE_BLOCKSIZE 4096
#define DISKQUEUE_MAX_ELEMENTS 4080
#define DISKQUEUE_DELIMINATOR '@'
#include <stdint.h>

struct diskqueue_blockdata { int np; int pp; int lh; int lt; };
struct diskqueue { int fptr; int size; int head; int tail;
                   __uint8_t hdata[DISKQUEUE_BLOCKSIZE/sizeof(__uint8_t)]; 
                   struct diskqueue_blockdata hmeta;
                   __uint8_t tdata[DISKQUEUE_BLOCKSIZE/sizeof(__uint8_t)];
                   struct diskqueue_blockdata tmeta;
                   struct diskstack * block_stack; };

#define DQ_DATA 4080

#endif
