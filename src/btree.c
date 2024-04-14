
#include "btree.h"
#include "skiplist.h"
#include "ringbuffer.h"
#include <stdint.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

void init_btree_cache(size_t size, struct btree_cache * c) {
   struct skip_list_bank * b = malloc(sizeof(struct skip_list_bank));
   struct skip_list  * l = malloc(sizeof(struct skip_list));
   struct ring_buffer * r = malloc(sizeof(struct ring_buffer));
   init_skip_list(size,b,l);
   init_ring_buffer(size,r);
   c->l = l;
   c->r = r;
   c->size = size;
}

void destroy_btree_cache(struct btree_cache * c) {
   struct skip_list_bank * b = c->l->bank;
   destroy_skip_list(c->l);
   destroy_ring_buffer(c->r);
   free(b);
   free(c->l);
   free(c->r);
}

void init_btree_node_bank(size_t size, struct btree_node_bank * b) {
   b->store = malloc(sizeof(struct btree_node *)*size);
   b->pos = size;
   struct btree_node * curr;
   for (size_t i = 0; i < size; ++i) {
      curr = malloc(sizeof(struct btree_node));
      memset(curr,0,sizeof(__uint8_t)+sizeof(__uint16_t)+sizeof(size_t));
      curr->data = malloc(sizeof(__uint64_t)*BTREE_DATASIZE);
      memset(curr->data,0,sizeof(__uint64_t)*BTREE_DATASIZE);
      b->store[i] = curr;
   }
}

void destroy_btree_node_bank(struct btree_node_bank * b) {
   struct btree_node * curr;
   for (size_t i = 0; i < b->pos; ++i) {
      curr = b->store[i];
      free(curr->data);
      free(curr);
   }
   free(b->store);
}

#define BT_NODE 0x00000100
#define BT_LEAF 0x00000001
#define BT_MOD  0x00000010
#define BT_CHECKOUT 0x00001000

#define IS_CHECKOUT_BT(x) ((x)&0x00001000)
#define SET_CHECKOUT_BT(x) (x|=0x00001000)
#define UNSET_CHECKOUT_BT(x) (x&=0xffff0fff)

#define IS_LEAF_BT(x) ((x)&0x00000001)
#define SET_LEAF_BT(x) (x|=0x00000001)
#define UNSET_LEAF_BT(x) (x&=0xfffffff0)

#define IS_MOD_BT(x) ((x)&0x00000010)
#define SET_MOD_BT(x) (x|=0x00000010)
#define UNSET_MOD_BT(x) (x&=0xffffff0f)

#define IS_NODE_BT(x) ((x)&0x00000100)
#define SET_NODE_BT(x) (x|=0x00000100)
#define UNSET_NODE_BT(x) (x&=0xfffff0ff)

#define CLEAR_BT(x) (x&=0xfffff000)
#define MULTI_SET_BT(x,flags) (x|=(flags))


struct btree_node * checkout_btree_node_bank(struct btree_node_bank * b) {
   if (b->pos == 0) return (struct btree_node *)-1;
   struct btree_node * ret = b->store[--(b->pos)];
   SET_CHECKOUT_BT(ret->flags);
   return ret;
}

void return_btree_node_bank(struct btree_node * n, struct btree_node_bank * b) {
   UNSET_CHECKOUT_BT(n->flags);
   b->store[b->pos] = n; ++(b->pos);
}


struct btree_node * load_node_btree(size_t pos, struct btree * b) {
   struct btree_node * node = checkout_btree_node_bank(b->bank);
   int read_total = 0;
   lseek(b->fptr,pos*BTREE_BLOCKSIZE,SEEK_SET);
   read_total += read(b->fptr,&node->flags,sizeof(__uint32_t));
   read_total += read(b->fptr,&node->size,sizeof(__uint32_t));
   read_total += read(b->fptr,node->data,sizeof(__uint64_t)*BTREE_DATASIZE);
   UNSET_MOD_BT(node->flags);
   node->pos = pos;
   return node;
}

struct btree_node * load_new_node_btree(size_t pos, struct btree * b) {
   struct btree_node * node = checkout_btree_node_bank(b->bank);
   UNSET_MOD_BT(node->flags);
   node->pos = pos;
   CLEAR_BT(node->flags);
   node->size = 0;
   return node;
}

void unload_node_btree(struct btree_node * n, struct btree * b) {
   int write_total = 0;
   lseek(b->fptr,n->pos*BTREE_BLOCKSIZE,SEEK_SET);
   write_total += write(b->fptr,&n->flags,sizeof(__uint32_t));
   write_total += write(b->fptr,&n->size,sizeof(__uint32_t));
   write_total += write(b->fptr,n->data,sizeof(__uint64_t)*BTREE_DATASIZE);
   return_btree_node_bank(n,b->bank);
}

void write_node_btree(struct btree_node * n, struct btree * b) {
   lseek(b->fptr,n->pos*BTREE_BLOCKSIZE,SEEK_SET);
   write(b->fptr,&n->flags,sizeof(__uint32_t));
   write(b->fptr,&n->size,sizeof(__uint32_t));
   write(b->fptr,n->data,sizeof(__uint64_t)*BTREE_DATASIZE);
}

void shell_sort_btree_node(struct btree_node * n) {
   /* https://web.archive.org/web/20180923235211/http://sun.aei.polsl.pl/~mciura/publikacje/shellsort.pdf */
   static int gaps[5] = { 109, 41, 19, 5, 1 }; 
   __uint64_t temp;
   int gap;
   for (int i = 0; i < 5; ++i) {
      gap = gaps[i];
      for (int j = gap; j < n->size; ++j) {
         temp = n->data[j];
         for (int k = j; (k >= gap) && (n->data[k-gap] > temp); k -= gap) {
            n->data[k] = n->data[k - gap];
         }
         n->data[j] = temp;
      }
   }
}

void add_btree_cache(struct btree_node * n, struct btree * b) {
   if (query_skip_list(n->pos,NULL,b->cache->l) == (void *)-1) {
      struct btree_node * ret;
      ret = delete_skip_list(add_ring_buffer(n->pos,b->cache->r),b->cache->l);
      if (ret != (void *)-1) {
         if (IS_MOD_BT(ret->flags)) { 
            unload_node_btree(ret,b); 
         }
         else return_btree_node_bank(ret,b->bank);
      }
      insert_skip_list(n->pos,(void *)n,b->cache->l);
   }
}

struct btree_node * safe_add_btree_cache(struct btree_node * n, struct btree * b) {
   if (IS_CHECKOUT_BT(n->flags)) {
      add_btree_cache(n,b);
   }
   else {
      n = load_node_btree(n->pos,b);
      add_btree_cache(n,b);
   }
   return n;
}

struct btree_node * get_btree_cache(size_t pos, struct btree * b) {
   return query_skip_list(pos,NULL,b->cache->l);
}

void clear_btree_cache(struct btree * b) {
   struct btree_node * ret;
   for (int i = 0; i < b->cache->r->size; ++i) {
      ret = delete_skip_list(add_ring_buffer(0,b->cache->r),b->cache->l);
      if (ret != (struct btree_node *)-1) { unload_node_btree(ret,b); }
   }
}

struct btree_node * split_root_btree(struct btree * b) {
   struct btree_node * old_root = b->root;
   struct btree_node * new_sibling = load_new_node_btree(++(b->size),b);
   struct btree_node * new_root = load_new_node_btree(++(b->size),b);
   if (IS_LEAF_BT(old_root->flags)) {
      CLEAR_BT(old_root->flags); MULTI_SET_BT(old_root->flags,BT_MOD|BT_LEAF);
      MULTI_SET_BT(new_sibling->flags,BT_LEAF|BT_MOD);
      MULTI_SET_BT(new_root->flags,BT_NODE|BT_MOD);
      /* set new_root values */
      static int midpoint = BTREE_DATASIZE/2;
      new_root->size = 1;
      new_root->data[0] = old_root->data[midpoint];
      new_root->data[BTREE_ELEMENTNUM] = old_root->pos;
      new_root->data[BTREE_POINTERNUM] = new_sibling->pos;
      new_sibling->size = midpoint;
      old_root->size = midpoint;
      for (int i = 0; i < midpoint; ++i) {
         new_sibling->data[i] = old_root->data[i+midpoint+1];
      }
   }
   else {
      CLEAR_BT(old_root->flags); MULTI_SET_BT(old_root->flags,BT_MOD|BT_NODE);
      MULTI_SET_BT(new_sibling->flags,BT_MOD|BT_NODE); 
      MULTI_SET_BT(new_root->flags,BT_NODE|BT_MOD);
      static int midpoint = BTREE_ELEMENTNUM/2;
      new_root->size = 1;
      new_root->data[0] = old_root->data[midpoint];
      new_root->data[BTREE_ELEMENTNUM] = old_root->pos;
      new_root->data[BTREE_POINTERNUM] = new_sibling->pos;
      new_sibling->size = midpoint;
      old_root->size = midpoint;
      int i;
      for (i = 0; i < midpoint; ++i) {
         new_sibling->data[i] = old_root->data[i+midpoint+1];
         new_sibling->data[i+BTREE_ELEMENTNUM] = old_root->data[i+midpoint+BTREE_POINTERNUM];
      }
      new_sibling->data[i+BTREE_ELEMENTNUM] = old_root->data[i+midpoint+BTREE_POINTERNUM];
   }
   old_root = safe_add_btree_cache(old_root,b);
   new_sibling = safe_add_btree_cache(new_sibling,b);
   return new_root;
}

void add_btree_node_btree(__uint64_t key, __uint64_t pointer, struct btree_node * n) {
   int i = 0;
   while (i < n->size) {
      if (n->data[i] > key) break;
      ++i;
   }
   for (int j = n->size; j > i; --j) {
      n->data[j] = n->data[j-1];
      n->data[j+BTREE_POINTERNUM] = n->data[j+BTREE_ELEMENTNUM];
   }
   n->data[i] = key; 
   n->data[i+BTREE_POINTERNUM] = pointer;
   ++(n->size);
   SET_MOD_BT(n->flags);
}

void add_btree_leaf_btree(__uint64_t key, struct btree_node * n) {
   int i = 0;
   while (i < n->size) {
      if (n->data[i] > key) break;
      ++i;
   }
   for (int j = n->size; j > i; --j) {
      n->data[j] = n->data[j-1];
   }
   n->data[i] = key;
   ++(n->size);
   SET_MOD_BT(n->flags);
}

void split_node_btree(struct btree_node * parent, struct btree_node * n, struct btree * b) {
   struct btree_node * new_sibling = load_new_node_btree(++(b->size),b);
   MULTI_SET_BT(new_sibling->flags,BT_NODE|BT_MOD);
   SET_MOD_BT(n->flags);
   static int midpoint = BTREE_ELEMENTNUM/2;

   new_sibling->size = midpoint;
   n->size = midpoint;

   add_btree_node_btree(n->data[midpoint],new_sibling->pos,parent);
   int i;
   for (i = 0; i < midpoint; ++i) {
      new_sibling->data[i] = n->data[i+midpoint+1];
      new_sibling->data[i+BTREE_ELEMENTNUM] = n->data[i+midpoint+BTREE_ELEMENTNUM];
   }
   new_sibling->data[i+BTREE_ELEMENTNUM] = n->data[i+midpoint+BTREE_ELEMENTNUM];
   new_sibling = safe_add_btree_cache(new_sibling,b);
}

void split_leaf_btree(struct btree_node * parent, struct btree_node * n, struct btree * b) {
   struct btree_node * new_sibling = load_new_node_btree(++(b->size),b);
   MULTI_SET_BT(new_sibling->flags,BT_LEAF|BT_MOD);
   SET_MOD_BT(n->flags);
   static int midpoint = BTREE_DATASIZE/2;

   new_sibling->size = midpoint;
   n->size = midpoint;

   add_btree_node_btree(n->data[midpoint],new_sibling->pos,parent);
   for (int i = 0; i < midpoint; ++i) {
      new_sibling->data[i] = n->data[i+midpoint+1];
   }
   new_sibling = safe_add_btree_cache(new_sibling,b);
}

void get_metadata_btree(struct btree * b) {
   lseek(b->fptr,0,SEEK_SET);
   read(b->fptr,&b->size,sizeof(int));
   size_t root_pos;
   read(b->fptr,&root_pos,sizeof(size_t));
   b->root = load_node_btree(root_pos,b);
}

void write_metadata_btree(struct btree * b) {
   lseek(b->fptr,0,SEEK_SET);
   int write_count = 0;
   write_count += write(b->fptr,&b->size,sizeof(int));
   write_count += write(b->fptr,&b->root->pos,sizeof(size_t));
}

void init_btree(char * filepath, struct btree * b) {
   b->size = 0;
   int exists = (access(filepath,F_OK) == 0);
   b->fptr = open(filepath,O_CREAT|O_RDWR,S_IRWXU); errno = 0;
   if (b->fptr == -1) {
      perror("open");
   }
   struct btree_cache * c = malloc(sizeof(struct btree_cache));
   struct btree_node_bank * n = malloc(sizeof(struct btree_node_bank));
   init_btree_cache(BTREE_CACHE_SIZE,c);
   init_btree_node_bank(BTREE_BANK_SIZE,n);
   b->cache = c;
   b->bank = n;
   if (exists) get_metadata_btree(b);
   else {
      b->root = load_new_node_btree(++(b->size),b);
      MULTI_SET_BT(b->root->flags,BT_LEAF|BT_MOD);
   }
}

struct btree_node * get_node_btree(size_t pos, struct btree * b) {
   struct btree_node * ret;
   if ((ret = get_btree_cache(pos,b)) == (struct btree_node *)-1) {
      ret = load_node_btree(pos,b);
      ret = safe_add_btree_cache(ret,b);
   }
   return ret;
}

/* Correctly deallocates all the allocated memory.
 * There is a lot of memory allocated...
 */
void close_btree(struct btree * b) {
   write_metadata_btree(b);
   b->root = safe_add_btree_cache(b->root,b);
   clear_btree_cache(b);
   destroy_btree_node_bank(b->bank);
   destroy_btree_cache(b->cache);
   free(b->cache);
   free(b->bank);
   close(b->fptr);
   b->size = 0;
}

int get_attempt_btree_node(__uint64_t key, struct btree_node * n, __uint64_t * pointer) {
   int i;
   for (i = 0; i < n->size; ++i) {
      if (n->data[i] == key) return 1;
      else if (n->data[i] > key) {
         *pointer = n->data[i+BTREE_ELEMENTNUM];
         return 0;
      }
   }
   *pointer = n->data[i+BTREE_ELEMENTNUM];
   return 0;
}

int get_attempt_btree_leaf(__uint64_t key, struct btree_node * n) {
   for (int i = 0; i < n->size; ++i) {
      if (n->data[i] == key) return 1;
   }
   return 0;
}

int is_full_btree_node(struct btree_node * n) {
   if (IS_LEAF_BT(n->flags)) {
      return n->size == BTREE_DATASIZE;
   }
   else return n->size == BTREE_ELEMENTNUM;
}

int add_btree(__uint64_t key, struct btree * b) {
   struct btree_node * p = NULL;
   struct btree_node * c = b->root;
   int res;
   __uint64_t pointer;

   if (is_full_btree_node(c)) {
      b->root = split_root_btree(b);
      c = b->root;
   }

   while (1) {
      if (is_full_btree_node(c)) {
         if (IS_LEAF_BT(c->flags)) 
            split_leaf_btree(p,c,b);
         else
            split_node_btree(p,c,b);
         safe_add_btree_cache(c,b);
         safe_add_btree_cache(p,b);
         c = p;
      }
      if (IS_LEAF_BT(c->flags)) {
         res = get_attempt_btree_leaf(key,c);
         if (res) return 1;
         add_btree_leaf_btree(key,c); 
         return 0;
      }
      else {
         res = get_attempt_btree_node(key,c,&pointer);
         if (res) return 1;
         p = c;
         c = get_node_btree(pointer,b);
      }
   }
}

void in_order_print_helper_btree(struct btree_node * n, struct btree * b) {
   if (n == -1) return;
   if (IS_LEAF_BT(n->flags)) {
      printf("<%ldl> ",n->pos);
      for (int i = 0; i < n->size; ++i) {
         printf("%ld ",n->data[i]);
      }
      return;
   }
   else {
      struct btree_node * np;
      int i;
      printf("<%ldn> ",n->pos);
      for (i = 0; i < n->size; ++i) {
         np = get_node_btree(n->data[i+BTREE_ELEMENTNUM],b);
         putchar('\\');
         in_order_print_helper_btree(np,b);
         putchar('/');
         printf("%ld ",n->data[i]);
      }
      np = get_node_btree(n->data[i+BTREE_ELEMENTNUM],b);
      putchar('\\');
      in_order_print_helper_btree(np,b);
      putchar('/');
   }
}

void in_order_print_btree(struct btree * b) {
   in_order_print_helper_btree(b->root,b);
   putchar('\n');
}

/*
int main(void) {

   struct btree b;

   init_btree("btree.b",&b);
    
   const int cycles = 300000000;

   int toadd;

   for (int i = cycles; i > 0; --i) {
      toadd = rand();
      printf("adding %d %d\n",i,toadd);
      add_btree(toadd,&b);
   }

   printf("blocks : %d\n",b.size);
   printf("bankpos: %d\n",b.bank->pos);

   //in_order_print_btree(&b);

   close_btree(&b);


}
*/
