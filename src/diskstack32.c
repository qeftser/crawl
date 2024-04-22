
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

void read_metadata_diskstack(struct diskstack * d) {
   lseek(d->fptr,0,SEEK_SET);
   read(d->fptr,&(d->size),sizeof(int));
}

void write_metadata_diskstack(struct diskstack * d) {
   lseek(d->fptr,0,SEEK_SET);
   write(d->fptr,&(d->size),sizeof(int));
}

void push_block_diskstack(struct diskstack * d) {
   ++(d->size);
   lseek(d->fptr,d->size*DISKSTACK_BLOCKSIZE,SEEK_SET);
   write(d->fptr,d->data,DISKSTACK_BLOCKSIZE);
}

void pop_block_diskstack(struct diskstack * d) {
   lseek(d->fptr,d->size*DISKSTACK_BLOCKSIZE,SEEK_SET);
   read(d->fptr,d->data,DISKSTACK_BLOCKSIZE);
   --(d->size);
}

void init_diskstack(char * name, struct diskstack * d) {
   int exists = (access(name,F_OK) == 0);
   d->size = 0;
   d->fptr = open(name,O_CREAT|O_RDWR,S_IRWXU);
   if (d->fptr == -1) {
      perror("open");
      exit(1);
   }
   memset(d->data,0,DISKSTACK_BLOCKSIZE);
   if (exists) {
      read_metadata_diskstack(d);
      pop_block_diskstack(d);
      printf("current size %d\n",d->size);
   }
   else {
      printf("CREATING NEW DISKSTACK\n");
   }
}

void close_diskstack(struct diskstack * d) {
   push_block_diskstack(d);
   write_metadata_diskstack(d);
   ftruncate(d->fptr,(d->size+1)*DISKSTACK_BLOCKSIZE);
   close(d->fptr);
}

void print_diskstack(struct diskstack * d) {
   int buffer[DISKSTACK_BLOCKSIZE/sizeof(int)];
   for (int i = 1; i <= d->size; ++i) {
      lseek(d->fptr,DISKSTACK_BLOCKSIZE*i,SEEK_SET);
      read(d->fptr,buffer,DISKSTACK_BLOCKSIZE);
      printf("| %d |",buffer[0]);
      for (int j = 1; j <= buffer[0]; ++j) {
         printf("%d ",buffer[j]);
      }
   }
   printf("| %d |",d->data[0]);
   for (int i = 1; i <= DISKSTACK_SIZE(d); ++i) {
      printf("%d ",d->data[i]);
   }
   putchar('\n');
   printf("===================\n");
   printf("Total blocks: %d\n",d->size);
}

void print_metadata_diskstack(struct diskstack * d) {
   printf("===================\n");
   printf("Total blocks: %d\n",d->size);
}

void push_diskstack(int v, struct diskstack * d) {
   if (DISKSTACK_SIZE(d) == MAX_ELEMENTS) {
      push_block_diskstack(d);
      memset(d->data,0,DISKSTACK_BLOCKSIZE);
   }
   ++(DISKSTACK_SIZE(d));
   d->data[DISKSTACK_SIZE(d)] = v;
}

int pop_diskstack(struct diskstack * d) {
   if (d->size == 0 && DISKSTACK_SIZE(d) == 0) return -1;
   if (DISKSTACK_SIZE(d) == 0) {
      pop_block_diskstack(d);
   }
   --(DISKSTACK_SIZE(d));
   return d->data[DISKSTACK_SIZE(d)+1];
}

/*
int main(void) {


   struct diskstack d;
   init_diskstack("diskstack.d",&d);

   for (int i = 0; i < 0x1fffffff; i++) {
      push_diskstack(i,&d);
   }

   //print_diskstack(&d);

   for (int i = 0; i < 0x0fffffff; i++) {
      printf("popping: %d\n",pop_diskstack(&d));
   }

   //print_diskstack(&d);
   print_metadata_diskstack(&d);

   close_diskstack(&d);

   return 0;
}
*/
