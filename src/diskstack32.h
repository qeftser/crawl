
#ifndef DISKSTACK_32

#define DISKSTACK_32

#define DISKSTACK_BLOCKSIZE 4096
#define MAX_ELEMENTS 1023

struct diskstack { int fptr; int size; int data[DISKSTACK_BLOCKSIZE/sizeof(int)]; };

#define DISKSTACK_SIZE(x) x->data[0]

void init_diskstack(char * name, struct diskstack *);
void push_diskstack(int v, struct diskstack *);
int pop_diskstack(struct diskstack *);

void close_diskstack(struct diskstack *);
void print_diskstack(struct diskstack *);
void destroy_diskstack(struct diskstack *);

#endif
