
#define _LARGEFILE64_SOURCE 1
#define _FILE_OFFSET_BITS 64

#include "diskqueue32.h"
#include "diskstack32.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
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


void read_metadata_diskqueue32(struct diskqueue32 * d) {
   lseek64(d->fptr,0,SEEK_SET);
   read(d->fptr,&(d->size),sizeof(int));
   read(d->fptr,&(d->head),sizeof(int));
   read(d->fptr,&(d->tail),sizeof(int));
}

void write_metadata_diskqueue32(struct diskqueue32 * d) {
   lseek64(d->fptr,0,SEEK_SET);
   write(d->fptr,&(d->size),sizeof(int));
   write(d->fptr,&(d->head),sizeof(int));
   write(d->fptr,&(d->tail),sizeof(int));
}

void write_block_diskqueue32(int next, int pos, int head, struct diskqueue32 * d) {
   lseek64(d->fptr,DISKQUEUE32_BLOCKSIZE*pos,SEEK_SET);
   if (head) {
      d->hdata[DQ_PP] = next;
      write(d->fptr,d->hdata,DISKQUEUE32_BLOCKSIZE);
   }
   else {
      d->tdata[DQ_NP] = next;
      write(d->fptr,d->tdata,DISKQUEUE32_BLOCKSIZE);
   }
}

void read_block_diskqueue32(int pos, int head, struct diskqueue32 * d) {
   lseek64(d->fptr,DISKQUEUE32_BLOCKSIZE*pos,SEEK_SET);
   if (head) {
      read(d->fptr,d->hdata,DISKQUEUE32_BLOCKSIZE);
   }
   else {
      read(d->fptr,d->tdata,DISKQUEUE32_BLOCKSIZE);
   }
}

void init_diskqueue32(char * name, struct diskqueue32 * d) {
   int exists = (access(name,F_OK) == 0);
   d->size = 0;
   d->fptr = open(name,O_CREAT|O_RDWR|O_LARGEFILE,S_IRWXU);
   if (d->fptr == -1) {
      perror("open");
      exit(1);
   }
   memset(d->hdata,0,DISKQUEUE32_BLOCKSIZE);
   memset(d->tdata,0,DISKQUEUE32_BLOCKSIZE);
   
   d->block_stack = malloc(sizeof(struct diskstack32));
   init_diskstack32("aux-diskstack.d",d->block_stack);

   if (exists) {
      read_metadata_diskqueue32(d);
      read_block_diskqueue32(d->head,1,d);
      read_block_diskqueue32(d->tail,0,d);
   }
   else {
      printf("CREATING NEW DISKQUEUE32\n");
      d->head = d->tail = d->size = 1;
      d->hdata[DQ_PP] = d->tdata[DQ_NP] = 0;
   }
}

void close_diskqueue32(struct diskqueue32 * d) {
   write_block_diskqueue32(0,d->tail,0,d);
   write_block_diskqueue32(0,d->head,1,d);
   write_metadata_diskqueue32(d);
   close_diskstack32(d->block_stack);
   free(d->block_stack);
   close(d->fptr);
}

void print_diskqueue32(struct diskqueue32 * d) {
   int buffer[DISKQUEUE32_BLOCKSIZE/sizeof(int)];
   for (int i = d->hdata[DQ_LH]; i < d->hdata[DQ_LT]; ++i) {
      printf("%d ",d->hdata[i]);
   }
   buffer[DQ_NP] = d->hdata[DQ_NP];
   while (buffer[DQ_NP] && buffer[DQ_NP] != d->tail) {
      lseek64(d->fptr,buffer[DQ_NP]*DISKQUEUE32_BLOCKSIZE,SEEK_SET);
      read(d->fptr,buffer,DISKQUEUE32_BLOCKSIZE);
      for (int i = 0; i < DISKQUEUE32_MAX_ELEMENTS; ++i) {
         printf("%d ",buffer[i]);
      }
   }
   if (d->tail != d->head) {
      for (int i = d->tdata[DQ_LH]; i < d->tdata[DQ_LT]; ++i) {
         printf("%d ",d->tdata[i]);
      }
   }
   putchar('\n');
   printf("==================\n");
   printf("Total Blocks: %d\n",d->size);
}

void load_new_head_diskqueue32(struct diskqueue32 * d) {
   int old_pos = d->head;
   int new_pos = pop_diskstack32(d->block_stack);
   if (new_pos == -1) new_pos = ++(d->size);
   write_block_diskqueue32(new_pos,d->head,1,d);
   memset(d->hdata,0,DISKQUEUE32_BLOCKSIZE);
   d->hdata[DQ_NP] = old_pos;
   d->head = new_pos;
}

void load_new_tail_diskqueue32(struct diskqueue32 * d) {
   int old_pos = d->tail;
   int new_pos = pop_diskstack32(d->block_stack);
   if (new_pos == -1) new_pos = ++(d->size);
   write_block_diskqueue32(new_pos,d->tail,0,d);
   memset(d->tdata,0,DISKQUEUE32_BLOCKSIZE);
   d->tdata[DQ_PP] = old_pos;
   d->tail = new_pos;
}

void load_next_head_diskqueue32(struct diskqueue32 * d) {
   int next_pos = d->hdata[DQ_NP];
   int curr_pos = d->head;
   memset(d->hdata,0,DISKQUEUE32_BLOCKSIZE);
   read_block_diskqueue32(next_pos,1,d);
   d->head = next_pos;
   d->hdata[DQ_PP] = 0;
   push_diskstack32(curr_pos,d->block_stack);
}

void load_prev_tail_diskqueue32(struct diskqueue32 * d) {
   int prev_pos = d->tdata[DQ_PP];
   int curr_pos = d->tail;
   memset(d->tdata,0,DISKQUEUE32_BLOCKSIZE);
   read_block_diskqueue32(prev_pos,0,d);
   d->tail = prev_pos;
   d->tdata[DQ_NP] = 0;
   push_diskstack32(curr_pos,d->block_stack);

}

#define align_low_head32(x) memmove(x->hdata,x->hdata+x->hdata[DQ_LH],(x->hdata[DQ_LT]-x->hdata[DQ_LH])*sizeof(int)); \
                          x->hdata[DQ_LT] = (x->hdata[DQ_LT]-x->hdata[DQ_LH]); \
                          x->hdata[DQ_LH] = 0
#define align_high_head32(x) memmove(x->hdata+(DISKQUEUE32_MAX_ELEMENTS-x->hdata[DQ_LT]),x->hdata+x->hdata[DQ_LH],(x->hdata[DQ_LT]-x->hdata[DQ_LH])*sizeof(int)); \
                           x->hdata[DQ_LH] = (DISKQUEUE32_MAX_ELEMENTS-x->hdata[DQ_LT]); \
                           x->hdata[DQ_LT] = DISKQUEUE32_MAX_ELEMENTS
#define align_low_tail32(x) memmove(x->tdata,x->tdata+x->tdata[DQ_LT],(x->tdata[DQ_LT]-x->tdata[DQ_LH])*sizeof(int)); \
                          x->tdata[DQ_LT] = (x->tdata[DQ_LT]-x->tdata[DQ_LH]); \
                          x->tdata[DQ_LH] = 0
#define align_high_tail32(x) memmove(x->tdata+(DISKQUEUE32_MAX_ELEMENTS-x->tdata[DQ_LT]),x->tdata+x->tdata[DQ_LH],(x->tdata[DQ_LT]-x->tdata[DQ_LH])*sizeof(int)); \
                           x->tdata[DQ_LH] = (DISKQUEUE32_MAX_ELEMENTS-x->tdata[DQ_LT]); \
                           x->tdata[DQ_LT] = DISKQUEUE32_MAX_ELEMENTS


void enqueue_diskqueue32(int v, struct diskqueue32 * d) {
   if (d->tail == d->head) {
      if (d->hdata[DQ_LT] == DISKQUEUE32_MAX_ELEMENTS) {
         if (d->hdata[DQ_LH] == 0) {
            load_new_tail_diskqueue32(d);
            d->hdata[DQ_NP] = d->tail;
            goto new_tail;
         }
         else {
            align_low_head32(d);
         }
      }
      d->hdata[d->hdata[DQ_LT]] = v;
      ++(d->hdata[DQ_LT]);
   }
   else {
new_tail:
      if (d->tdata[DQ_LT] == DISKQUEUE32_MAX_ELEMENTS) {
         if (d->tdata[DQ_LH] == 0) {
            load_new_tail_diskqueue32(d);
         }
         else {
            align_low_tail32(d);
         }
      }
      d->tdata[d->tdata[DQ_LT]] = v;
      ++(d->tdata[DQ_LT]);
   }
}

int dequeue_diskqueue32(struct diskqueue32 * d) {
   if (d->tail == d->head) {
      if (d->hdata[DQ_LH] == d->hdata[DQ_LT]) {
         return INT_MIN;
      }
      else {
         ++(d->hdata[DQ_LH]);
         return d->hdata[d->hdata[DQ_LH]-1];
      }
   }
   else {
      if (d->hdata[DQ_LH] == d->hdata[DQ_LT]) {
         if (d->hdata[DQ_NP] == d->tail) {
            write_block_diskqueue32(0,d->tail,0,d);
         }
         load_next_head_diskqueue32(d);
      }
   }
   ++(d->hdata[DQ_LH]);
   return d->hdata[d->hdata[DQ_LH]-1];
}

/*
int main(void) {

   int cycles = 1000000000;

   struct diskqueue32 d;
   init_diskqueue32("diskqueue32.d",&d);

   for (int i = 0; i < cycles; ++i) {
      enqueue_diskqueue32(i,&d);
   }

   for (int i = 0; i < cycles/2; ++i) {
      dequeue_diskqueue32(&d);
   }

   for (int i = 0; i < cycles/2; ++i) {
      enqueue_diskqueue32(i,&d);
   }

   for (int i = 0; i < cycles; ++i) {
      dequeue_diskqueue32(&d);
   }

   print_diskqueue32(&d);

   close_diskqueue32(&d);

   return 0;
}
*/
