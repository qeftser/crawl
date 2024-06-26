
#define _LARGEFILE64_SOURCE 1
#define _FILE_OFFSET_BITS 64

#include "diskqueue.h"
#include "diskstack32.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>

void read_metadata_diskqueue(struct diskqueue * d) {
   lseek64(d->fptr,0,SEEK_SET);
   read(d->fptr,&(d->size),sizeof(int));
   read(d->fptr,&(d->head),sizeof(int));
   read(d->fptr,&(d->tail),sizeof(int));
}

void write_metadata_diskqueue(struct diskqueue * d) {
   lseek64(d->fptr,0,SEEK_SET);
   write(d->fptr,&(d->size),sizeof(int));
   write(d->fptr,&(d->head),sizeof(int));
   write(d->fptr,&(d->tail),sizeof(int));
}

void write_block_diskqueue(int next, int pos, int head, struct diskqueue * d) {
   lseek64(d->fptr,DISKQUEUE_BLOCKSIZE*pos,SEEK_SET);
   if (head) {
      d->hmeta.pp = next;
      write(d->fptr,d->hdata,DISKQUEUE_MAX_ELEMENTS);
      write(d->fptr,&d->hmeta,sizeof(struct diskqueue_blockdata));
   }
   else {
      d->tmeta.np = next;
      write(d->fptr,d->tdata,DISKQUEUE_MAX_ELEMENTS);
      write(d->fptr,&d->tmeta,sizeof(struct diskqueue_blockdata));
   }
}

void read_block_diskqueue(int pos, int head, struct diskqueue * d) {
   lseek64(d->fptr,DISKQUEUE_BLOCKSIZE*pos,SEEK_SET);
   if (head) {
      read(d->fptr,d->hdata,DISKQUEUE_BLOCKSIZE);
      memcpy(&d->hmeta,d->hdata+DQ_DATA,sizeof(struct diskqueue_blockdata));
   }
   else {
      read(d->fptr,d->tdata,DISKQUEUE_BLOCKSIZE);
      memcpy(&d->tmeta,d->tdata+DQ_DATA,sizeof(struct diskqueue_blockdata));
   }
}

void init_diskqueue(char * name, struct diskqueue * d) {
   int exists = (access(name,F_OK) == 0);
   d->size = 0;
   d->fptr = open(name,O_CREAT|O_RDWR|O_LARGEFILE,S_IRWXU);
   if (d->fptr == -1) {
      perror("open");
      exit(1);
   }
   memset(d->hdata,0,DISKQUEUE_BLOCKSIZE);
   memset(d->tdata,0,DISKQUEUE_BLOCKSIZE);
   memset(&d->hmeta,0,sizeof(struct diskqueue_blockdata));
   memset(&d->tmeta,0,sizeof(struct diskqueue_blockdata));
   
   d->block_stack = malloc(sizeof(struct diskstack32));
   init_diskstack32("aux-diskstack.d",d->block_stack);

   if (exists) {
      read_metadata_diskqueue(d);
      read_block_diskqueue(d->head,1,d);
      read_block_diskqueue(d->tail,0,d);
   }
   else {
      printf("CREATING NEW DISKQUEUE\n");
      d->head = d->tail = d->size = 1;
      d->hmeta.pp = d->tmeta.np = 0;
   }
}

void close_diskqueue(struct diskqueue * d) {
   write_block_diskqueue(0,d->tail,0,d);
   write_block_diskqueue(0,d->head,1,d);
   write_metadata_diskqueue(d);
   close_diskstack32(d->block_stack);
   free(d->block_stack);
   close(d->fptr);
}


void print_diskqueue(struct diskqueue * d) {
   __uint8_t buffer[DISKQUEUE_BLOCKSIZE];
   struct diskqueue_blockdata bmeta = { .np = d->hmeta.np };
   for (int i = d->hmeta.lh; i < d->hmeta.lt; ++i) {
      putchar(d->hdata[i]);
   }
   while (bmeta.np && bmeta.np != d->tail) {
      lseek64(d->fptr,bmeta.np*DISKQUEUE_BLOCKSIZE,SEEK_SET);
      read(d->fptr,buffer,DISKQUEUE_BLOCKSIZE);
      memcpy(&bmeta,buffer+DQ_DATA,sizeof(struct diskqueue_blockdata));
      for (int i = 0; i < bmeta.lt; ++i) {
         putchar(buffer[i]);
      }
   }
   if (d->tail != d->head) {
      for (int i = d->tmeta.lh; i < d->tmeta.lt; ++i) {
         putchar(d->tdata[i]);
      }
   }
   putchar('\n');
   printf("==================\n");
   printf("Total Blocks: %d\n",d->size);
}

void load_new_head_diskqueue(struct diskqueue * d) {
   int old_pos = d->head;
   int new_pos = pop_diskstack32(d->block_stack);
   if (new_pos == -1) new_pos = ++(d->size);
   write_block_diskqueue(new_pos,d->head,1,d);
   memset(d->hdata,0,DISKQUEUE_BLOCKSIZE);
   memset(&d->hmeta,0,sizeof(struct diskqueue_blockdata));
   d->hmeta.np = old_pos;
   d->head = new_pos;
}

void load_new_tail_diskqueue(struct diskqueue * d) {
   int old_pos = d->tail;
   int new_pos = pop_diskstack32(d->block_stack);
   if (new_pos == -1) new_pos = ++(d->size);
   write_block_diskqueue(new_pos,d->tail,0,d);
   memset(d->tdata,0,DISKQUEUE_BLOCKSIZE);
   memset(&d->tmeta,0,sizeof(struct diskqueue_blockdata));
   d->tmeta.pp = old_pos;
   d->tail = new_pos;
}

void load_next_head_diskqueue(struct diskqueue * d) {
   int next_pos = d->hmeta.np;
   int curr_pos = d->head;
   memset(d->hdata,0,DISKQUEUE_BLOCKSIZE);
   memset(&d->hmeta,0,sizeof(struct diskqueue_blockdata));
   read_block_diskqueue(next_pos,1,d);
   d->head = next_pos;
   d->hmeta.pp = 0;
   push_diskstack32(curr_pos,d->block_stack);
}

void load_prev_tail_diskqueue(struct diskqueue * d) {
   int prev_pos = d->tmeta.pp;
   int curr_pos = d->tail;
   memset(d->tdata,0,DISKQUEUE_BLOCKSIZE);
   memset(&d->tmeta,0,sizeof(struct diskqueue_blockdata));
   read_block_diskqueue(prev_pos,0,d);
   d->tail = prev_pos;
   d->tmeta.np = 0;
   push_diskstack32(curr_pos,d->block_stack);

}

#define align_low_head(x) memmove(x->hdata,x->hdata+x->hmeta.lh,(x->hmeta.lt-x->hmeta.lh)); \
                          x->hmeta.lt = (x->hmeta.lt-x->hmeta.lh); \
                          x->hmeta.lh = 0
#define align_high_head(x) memmove(x->hdata+(DISKQUEUE_MAX_ELEMENTS-x->hmeta.lt),x->hdata+x->hmeta.lh,(x->hmeta.lt-x->hmeta.lh)); \
                           x->hmeta.lh = (DISKQUEUE_MAX_ELEMENTS-x->hmeta.lt); \
                           x->hmeta.lt = DISKQUEUE_MAX_ELEMENTS
#define align_low_tail(x) memmove(x->tdata,x->tdata+x->tmeta.lt,(x->tmeta.lt-x->tmeta.lh)); \
                          x->tmeta.lt = (x->tmeta.lt-x->tmeta.lh); \
                          x->tmeta.lh = 0
#define align_high_tail(x) memmove(x->tdata+(DISKQUEUE_MAX_ELEMENTS-x->tmeta.lt),x->tdata+x->tmeta.lh,(x->tmeta.lt-x->tmeta.lh)); \
                           x->tmeta.lh = (DISKQUEUE_MAX_ELEMENTS-x->tmeta.lt); \
                           x->tmeta.lt = DISKQUEUE_MAX_ELEMENTS

void enqueue_diskqueue(void * data, int dsize, struct diskqueue * d) {
start:
   if (d->tail == d->head) {
      if (d->hmeta.lt + dsize + 1 >= DISKQUEUE_MAX_ELEMENTS) {
         if (d->hmeta.lh == 0) {
            load_new_tail_diskqueue(d);
            d->hmeta.np = d->tail;
            goto new_tail;
         }
         else {
            align_low_head(d);
            goto start;
         }
      }
      memcpy(d->hdata+d->hmeta.lt,data,dsize);
      d->hmeta.lt += dsize+2;
      d->hdata[d->hmeta.lt-1] = DISKQUEUE_DELIMINATOR;
   }
   else {
new_tail:
      if (d->tmeta.lt + dsize + 1 >= DISKQUEUE_MAX_ELEMENTS) {
         if (d->tmeta.lh == 0) {
            load_new_tail_diskqueue(d);
         }
         else {
            align_low_tail(d);
            goto new_tail;
         }
      }
      memcpy(d->tdata+d->tmeta.lt,data,dsize);
      d->tmeta.lt += dsize+2;
      d->tdata[d->tmeta.lt-1] = DISKQUEUE_DELIMINATOR;
   }
}

int dequeue_diskqueue(void * buf, int bsize, struct diskqueue * d) {
   if (d->tail == d->head) {
      if (d->hmeta.lh == d->hmeta.lt) {
         return -1;
      }
      else {
         memccpy(buf,d->hdata,DISKQUEUE_DELIMINATOR,bsize);
         while (d->hdata[d->hmeta.lh] != DISKQUEUE_DELIMINATOR) {
            ++(d->hmeta.lh);
         }
         ++(d->hmeta.lh);
         return 0;
      }
   }
   else {
      if (d->hmeta.lh == d->hmeta.lt) {
         if (d->hmeta.np == d->tail) {
            write_block_diskqueue(0,d->tail,0,d);
         }
         load_next_head_diskqueue(d);
      }
   }
   memccpy(buf,d->hdata,DISKQUEUE_DELIMINATOR,bsize);
   while (d->hdata[d->hmeta.lh] != DISKQUEUE_DELIMINATOR) {
      ++(d->hmeta.lh);
   }
   ++(d->hmeta.lh);
   return 0;
}

/*
int main(void) {

   __uint8_t buffer[120];

   int cycles = 0xfffffff;
   struct diskqueue d;


   init_diskqueue("diskqueue.d",&d);

   for (int j = 0; j < cycles; ++j) {
      enqueue_diskqueue("hiiiiiiiiiiiiiiiiih",20,&d);
   }

   for (int i = 0; i < cycles; ++i) {
      dequeue_diskqueue(buffer,120,&d);
   }

   print_diskqueue(&d);

   close_diskqueue(&d);

   return 0;
}
*/
