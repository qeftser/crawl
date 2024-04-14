
#ifndef DISKSTACK

#define DISKSTACK

#define DISKSTACK_BLOCKSIZE 4096
#define DISKSTACK_SEPERATOR '#'

struct diskstack { int localpos; int localsize; int size; int fptr; char buffer[DISKSTACK_BLOCKSIZE]; };

void init_diskstack(char *, struct diskstack *);
void psh_diskstack(char *, struct diskstack *);
void pop_diskstack(char *, struct diskstack *);

void close_diskstack(struct diskstack *);

void print_partial_diskstack(struct diskstack *);



#endif
