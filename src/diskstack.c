
#include "diskstack.h"
#include <stdint.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

int seek_next_diskstack(struct diskstack * d) {
   while (d->buffer[d->localpos] != DISKSTACK_SEPERATOR) {
      --(d->localpos);
   }
   return d->localpos+1;
}

int is_full_diskblock_diskstack(size_t elementsize, struct diskstack * d) {
   printf("%lu >= %lu\n",d->localpos+elementsize,DISKSTACK_BLOCKSIZE-sizeof(int));
   return (d->localpos+elementsize >= DISKSTACK_BLOCKSIZE-sizeof(int));
}

void pop_block_diskstack(struct diskstack * d) {
   if (d->size == 0) return;
   lseek(d->fptr,d->size*DISKSTACK_BLOCKSIZE,SEEK_SET);
   --(d->size);
   read(d->fptr,d->buffer,DISKSTACK_BLOCKSIZE);
   memcpy(&d->localsize,d->buffer+(DISKSTACK_BLOCKSIZE-sizeof(int)),sizeof(int));
   d->localpos = DISKSTACK_BLOCKSIZE-1;
   d->localpos = seek_next_diskstack(d)-1;
}

void psh_block_diskstack(struct diskstack * d) {
   ++(d->size);
   lseek(d->fptr,d->size*DISKSTACK_BLOCKSIZE,SEEK_SET);
   memcpy(d->buffer+(DISKSTACK_BLOCKSIZE-sizeof(int)),&d->localsize,sizeof(int));
   write(d->fptr,d->buffer,DISKSTACK_BLOCKSIZE);
   d->localsize = 0;
   d->localpos = 0;
   d->buffer[0] = DISKSTACK_SEPERATOR;
}

void write_metadata_diskstack(struct diskstack * d) {
   lseek(d->fptr,0,SEEK_SET);
   write(d->fptr,&d->size,sizeof(int));
}

void read_metadata_diskstack(struct diskstack * d) {
   lseek(d->fptr,0,SEEK_SET);
   read(d->fptr,&d->size,sizeof(int));
}

void pop_diskstack(char * outbuffer, struct diskstack * d) {
   if (d->localsize == 0) pop_block_diskstack(d);
   int endpos = d->localpos;
   int startpos = seek_next_diskstack(d);
   memcpy(outbuffer,d->buffer+startpos,endpos-startpos);
   outbuffer[endpos-startpos] = '\0';
   --(d->localsize);
}

void psh_diskstack(char * in, struct diskstack * d) {
   size_t sl = strlen(in);
   if (is_full_diskblock_diskstack(sl,d)) {
      psh_block_diskstack(d);
   }
   memcpy(d->buffer+d->localpos+1,in,sl);
   d->localpos+=sl+1;
   d->buffer[d->localpos] = DISKSTACK_SEPERATOR;
   ++(d->localsize);
}

void init_diskstack(char * filepath, struct diskstack * d) {
   d->size = 0;
   d->localsize = 0;
   d->localpos = 0;
   int exists = (access(filepath,F_OK) == 0);
   d->fptr = open(filepath,O_CREAT|O_RDWR,S_IRWXU); errno = 0;
   if (d->fptr == -1) {
      perror("open");
   }
   memset(d->buffer,0,sizeof(d->buffer));
   d->buffer[0] = DISKSTACK_SEPERATOR;
   if (exists) {
      read_metadata_diskstack(d);
      if (d->size) pop_block_diskstack(d);
   }
}

void close_diskstack(struct diskstack * d) {
   psh_block_diskstack(d);
   write_metadata_diskstack(d);
   ftruncate(d->fptr,(d->size+1)*DISKSTACK_BLOCKSIZE);
   close(d->fptr);
   d->fptr = 0;
   d->localpos = 0;
   d->localsize = 0;
   d->size = 0;
}

void print_partial_diskstack(struct diskstack * d) {
   printf("size: %d\n",d->size);
   printf("localsize: %d\n",d->localsize);
   printf("localpos: %d\n",d->localpos);
   printf("buffer:\n");
   for (int i = 0; i <= d->localpos; ++i) {
      putchar(d->buffer[i]);
   }
   putchar('\n');
}

int main(void) {

   struct diskstack d;

   init_diskstack("diskstack.d",&d);

   psh_diskstack("hi there",&d);
   psh_diskstack("hi there",&d);
   psh_diskstack("hi there",&d);
   psh_diskstack("hi there",&d);
   psh_diskstack("hi there",&d);

   print_partial_diskstack(&d);

   close_diskstack(&d);

   return 0;
}
