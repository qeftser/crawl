
#ifndef DISKSTACK32_32

#define DISKSTACK32_32

#define DISKSTACK32_BLOCKSIZE 4096LL
#define MAX_ELEMENTS 1023

struct diskstack32 { int fptr; int size; int data[DISKSTACK32_BLOCKSIZE/sizeof(int)]; };

#define DISKSTACK32_SIZE(x) x->data[0]

void init_diskstack32(char * name, struct diskstack32 *);
void push_diskstack32(int v, struct diskstack32 *);
int pop_diskstack32(struct diskstack32 *);

void close_diskstack32(struct diskstack32 *);
void print_diskstack32(struct diskstack32 *);
void destroy_diskstack32(struct diskstack32 *);

#endif
