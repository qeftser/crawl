
#define _LARGEFILE64_SOURCE 1
#define _FILE_OFFSET_BITS 64

#include "diskstack32.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#if __linux__
#include <unistd.h>
#include <fcntl.h>
#elif _WIN_32
#include <io.h>
#define access _access
#define open _open
#define read _read
#define write _write
#define F_OK 0
#define SEEK_SET 0
#endif

void read_metadata_diskstack32(struct diskstack32 * d) {
   lseek64(d->fptr,0,SEEK_SET);
   read(d->fptr,&(d->size),sizeof(int));
}

void write_metadata_diskstack32(struct diskstack32 * d) {
   lseek64(d->fptr,0,SEEK_SET);
   write(d->fptr,&(d->size),sizeof(int));
}

void push_block_diskstack32(struct diskstack32 * d) {
   ++(d->size);
   lseek64(d->fptr,d->size*DISKSTACK32_BLOCKSIZE,SEEK_SET);
   write(d->fptr,d->data,DISKSTACK32_BLOCKSIZE);
}

void pop_block_diskstack32(struct diskstack32 * d) {
   lseek64(d->fptr,d->size*DISKSTACK32_BLOCKSIZE,SEEK_SET);
   read(d->fptr,d->data,DISKSTACK32_BLOCKSIZE);
   --(d->size);
}

void init_diskstack32(char * name, struct diskstack32 * d) {
   int exists = (access(name,F_OK) == 0);
   d->size = 0;
   d->fptr = open(name,O_CREAT|O_RDWR|O_LARGEFILE,S_IRWXU);
   if (d->fptr == -1) {
      perror("open");
      exit(1);
   }
   memset(d->data,0,DISKSTACK32_BLOCKSIZE);
   if (exists) {
      read_metadata_diskstack32(d);
      pop_block_diskstack32(d);
      printf("current size %d\n",d->size);
   }
   else {
      printf("CREATING NEW DISKSTACK32\n");
   }
}

void close_diskstack32(struct diskstack32 * d) {
   push_block_diskstack32(d);
   write_metadata_diskstack32(d);
   ftruncate(d->fptr,(d->size+1)*DISKSTACK32_BLOCKSIZE);
   close(d->fptr);
}

void print_diskstack32(struct diskstack32 * d) {
   int buffer[DISKSTACK32_BLOCKSIZE/sizeof(int)];
   for (int i = 1; i <= d->size; ++i) {
      lseek64(d->fptr,DISKSTACK32_BLOCKSIZE*i,SEEK_SET);
      read(d->fptr,buffer,DISKSTACK32_BLOCKSIZE);
      printf("| %d |",buffer[0]);
      for (int j = 1; j <= buffer[0]; ++j) {
         printf("%d ",buffer[j]);
      }
   }
   printf("| %d |",d->data[0]);
   for (int i = 1; i <= DISKSTACK32_SIZE(d); ++i) {
      printf("%d ",d->data[i]);
   }
   putchar('\n');
   printf("===================\n");
   printf("Total blocks: %d\n",d->size);
}

void print_metadata_diskstack32(struct diskstack32 * d) {
   printf("===================\n");
   printf("Total blocks: %d\n",d->size);
}

void push_diskstack32(int v, struct diskstack32 * d) {
   if (DISKSTACK32_SIZE(d) == MAX_ELEMENTS) {
      push_block_diskstack32(d);
      memset(d->data,0,DISKSTACK32_BLOCKSIZE);
   }
   ++(DISKSTACK32_SIZE(d));
   d->data[DISKSTACK32_SIZE(d)] = v;
}

int pop_diskstack32(struct diskstack32 * d) {
   if (d->size == 0 && DISKSTACK32_SIZE(d) == 0) return -1;
   if (DISKSTACK32_SIZE(d) == 0) {
      pop_block_diskstack32(d);
   }
   --(DISKSTACK32_SIZE(d));
   return d->data[DISKSTACK32_SIZE(d)+1];
}

/*
int main(void) {


   struct diskstack32 d;
   init_diskstack32("diskstack32.d",&d);

   for (int i = 0; i < 0x3fffffff; i++) {
      push_diskstack32(i,&d);
   }

   //print_diskstack32(&d);

   for (int i = 0; i < 0x0fffffff; i++) {
      printf("popping: %d\n",pop_diskstack32(&d));
   }

   //print_diskstack32(&d);
   print_metadata_diskstack32(&d);

   close_diskstack32(&d);

   return 0;
}
*/
