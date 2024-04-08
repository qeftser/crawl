
/* yeet btree */

#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "btree.h"
#include "hashes.h"

#define BTREE_NODE 0x1000
#define BTREE_LEAF 0x2000
#define BTREE_EDIT 0x4000
#define ENSURE_VALID(xv) if (xv < 1) { perror("i/o");  }

void load_node_btree(__uint64_t pos, struct btree_node * dest, struct btree * b) {
   lseek(b->fptr,pos*4096,SEEK_SET);
   ENSURE_VALID(read(b->fptr,&dest->size,2));
   ENSURE_VALID(read(b->fptr,&dest->elements,2040));
   ENSURE_VALID(read(b->fptr,&dest->pointers,2048));
   dest->pos = pos;
   dest->size &= 0x3fff; // set as not edited
}

void unload_node_btree(__uint64_t pos, struct btree_node * node, struct btree * b) {
   lseek(b->fptr,pos*4096,SEEK_SET);
   ENSURE_VALID(write(b->fptr,&node->size,2));
   ENSURE_VALID(write(b->fptr,&node->elements,2040));
   ENSURE_VALID(write(b->fptr,&node->pointers,2048));
}

__uint64_t get_attempt_btree(__uint64_t key, __uint8_t * is_pointer, struct btree_node * n) {
   int i = 0; *is_pointer = 1;
   for (; i < (n->size&0xfff); i++) {
      if (key < n->elements[i])  { *is_pointer = 1; break; }
      if (key == n->elements[i]) { *is_pointer = 0; break; }
   }
   if (*is_pointer) {
      return n->pointers[i];
   }
   return key;
}

__uint64_t add_attempt_btree(__uint64_t key, __uint8_t * is_pointer, struct btree_node * n) {
   int i = 0; *is_pointer = 1;
   for (; i < (n->size&0xfff); i++) {
      if (key < n->elements[i])  break;
      if (key == n->elements[i]) { *is_pointer = 0; break; }
   }
   if (*is_pointer) {
      if ((n->size&0x1000)) {
         return n->pointers[i];
      }
      else {
         *is_pointer = 0;
      }
   }
   for (int j = (n->size&0xfff); j > i; j--) {
      n->elements[j] = n->elements[j-1];
   }
   n->elements[i] = key;
   n->size = (n->size&0xf000)|((n->size&0x0fff)+1);
   n->size |= BTREE_EDIT;
   return key;
}

void add_internal_btree(__uint64_t key, __uint64_t pointer, struct btree_node * n) {
   int i = 0;
   for (; i < (n->size&0xfff); i++) {
      if (key > n->elements[i]) break;
   }
   for (int j = (n->size&0xfff); j > i; j--) {
      n->elements[j] = n->elements[j-1];
      n->pointers[j+1] = n->pointers[j];
   }
   n->elements[i] = key;
   n->pointers[i+1] = pointer;
   n->size = (BTREE_EDIT | (n->size&0xf000) | ((n->size&0x0fff)+1));
}

void copy_node_btree(struct btree_node * a, struct btree_node * b) {
   //memcpy(a,b,sizeof(struct btree_node));
   a->pos = b->pos;
   a->size = b->size;
   int i = 0;
   if (b->size&0x1000) {
      for (; i < (b->size&0xfff); i++) {
         a->elements[i] = b->elements[i];
         a->pointers[i] = b->pointers[i];
      }
      a->pointers[i] = b->pointers[i];
   }
   else {
      for (; i < (b->size&0xfff); i++) {
         a->elements[i] = b->elements[i];
      }
   }
}

/* returns a node if it exists NO MATTER THE COST!! */
void get_node_btree(__uint64_t pos, struct btree_node * to_load, struct btree * b) {
   __uint8_t temp[5];
   memcpy(temp,&pos,4); temp[4] = '\0';
   __uint32_t hash = (murmer3_32((char *)temp,0x34f23a42))%BTREE_HASH_SIZE;
   if (b->hash->hash[hash].pos == pos) {
      copy_node_btree(to_load,&b->hash->hash[hash]);
   }
   else {
      load_node_btree(pos,to_load,b);
      if (b->hash->hash[hash].size&0x4000) {
         unload_node_btree(b->hash->hash[hash].pos,&b->hash->hash[hash],b);
      }
      copy_node_btree(&b->hash->hash[hash],to_load);
   }
}

void add_node_btree_cache(__uint64_t pos, struct btree_node * to_add, struct btree * b) {
   __uint8_t temp[5];
   memcpy(temp,&pos,4); temp[4] = '\0';
   __uint32_t hash = (murmer3_32((char *)temp,0x34f23a42))%BTREE_HASH_SIZE;
   if (b->hash->hash[hash].pos == pos) {
      copy_node_btree(&b->hash->hash[hash],to_add);
      return;
   }
   else {
      if (b->hash->hash[hash].size&0x4000) {
         unload_node_btree(b->hash->hash[hash].pos,&b->hash->hash[hash],b);
      }
      copy_node_btree(&b->hash->hash[hash],to_add);
   }
}

void init_btree(char * filename, struct btree * b) {
   if (access(filename,F_OK) == 0) {
      b->fptr = open(filename,O_RDWR);
      lseek(b->fptr,0,SEEK_SET);
      ENSURE_VALID(read(b->fptr,&b->size,2));
      __uint64_t rootpos;
      ENSURE_VALID(read(b->fptr,&rootpos,8));
      load_node_btree(rootpos,&b->root,b);
   }
   else {
      b->fptr = open(filename,O_CREAT|O_RDWR|O_TRUNC,S_IRWXU);
      b->size = 1;
      b->root.pos = 1;
      b->root.size = 0x2000;
   }
   b->hash = malloc(sizeof(struct btree_hash));
   memset(b->hash,0,sizeof(struct btree_hash));
}

void split_node_btree(struct btree_node * parent, struct btree_node * child, struct btree * b) {
   struct btree_node new;
   new.size = (BTREE_EDIT | 127 | (child->size&0x3000));
   child->size = (BTREE_EDIT | 127 | (child->size&0x3000));
   parent->size = BTREE_EDIT | (parent->size&0xf000) | ((parent->size&0x0fff) + 1);
   add_internal_btree(child->elements[127],child->pointers[128],parent);
   new.pos = ++(b->size);
   for (int i = 0; i < 127; i++) {
      new.elements[i] = child->elements[i+127];
      new.pointers[i] = child->pointers[i+127];
   }
   new.pointers[128] = child->pointers[255];
   add_node_btree_cache(new.pos,&new,b);
   add_node_btree_cache(child->pos,child,b);
   add_node_btree_cache(parent->pos,parent,b);
}

void split_root_btree(struct btree * b) {
   struct btree_node newL;
   struct btree_node newR;
   newL.size = (BTREE_EDIT | 127 | (b->root.size&0x3000));
   newL.pos = ++(b->size);
   newR.size = 0x5001; // new root
   newR.pos = ++(b->size);
   b->root.size = (BTREE_EDIT | 127 | (b->root.size&0x3000));
   newR.elements[0] = b->root.elements[127];
   newR.pointers[0] = b->root.pos;
   newR.pointers[1] = (b->size-1);
   for (int i = 0; i < 127; i++) {
      newL.elements[i] = b->root.elements[i+127];
      newL.pointers[i] = b->root.pointers[i+127];
   }
   newL.pointers[128] = b->root.pointers[128];
   add_node_btree_cache(newL.pos,&newL,b);
   add_node_btree_cache(b->root.pos,&b->root,b);
   add_node_btree_cache(newR.pos,&newR,b);
   copy_node_btree(&b->root,&newR);
}

void unload_btree(struct btree * b) {
   if (b->root.size&0x4000) {
      unload_node_btree(b->root.pos,&b->root,b);
   }
   for (int i = 0; i < BTREE_HASH_SIZE; i++) {
      if (b->hash->hash[i].size&0x4000) {
         unload_node_btree(b->hash->hash[i].pos,&b->hash->hash[i],b);
      }
   }
   lseek(b->fptr,0,SEEK_SET);
   ENSURE_VALID(write(b->fptr,&b->size,2));
   ENSURE_VALID(write(b->fptr,&b->root.pos,8));
   printf("size: %u rootpos: %lu\n",b->size,b->root.pos);
   close(b->fptr);
   free(b->hash);
}

void add_btree(__uint64_t key, struct btree * b) {
   if ((b->root.size&0x0fff) == 255) {
      split_root_btree(b);
   }

   struct btree_node curr;
   struct btree_node prev;
   __uint8_t is_pointer;
   __uint64_t retval;

   copy_node_btree(&curr,&b->root);

   while (1) {
      if ((curr.size&0x0fff) == 255) {
         split_node_btree(&prev,&curr,b);
         //if (prev.pos == b->root.pos) copy_node_btree(&b->root,&prev);
         copy_node_btree(&curr,&prev);
         continue;
      }
      copy_node_btree(&prev,&curr);
      retval = add_attempt_btree(key,&is_pointer,&curr);
      if (is_pointer) {
         get_node_btree(retval,&curr,b);
      }
      else {
         if (b->root.pos == curr.pos)
            copy_node_btree(&b->root,&curr);
         else
            add_node_btree_cache(curr.pos,&curr,b);
         printf("node: %ld\n",curr.pos);
         for (int i = 0; i < (curr.size&0xfff); i++) printf("%ld ",curr.elements[i]);
         putchar('\n');
         return; // value added!
      }
   }
}

__uint64_t get_btree(__uint64_t key, __uint8_t * success, struct btree * b) {
   struct btree_node curr;
   struct btree_node prev;
   __uint8_t is_pointer;
   __uint64_t retval;

   copy_node_btree(&curr,&b->root);

   while (1) {
      copy_node_btree(&prev,&curr);
      retval = get_attempt_btree(key,&is_pointer,&curr);
      if (is_pointer) {
         if (curr.size&0x2000) {
            *success = 0;
            return retval;
         }
         get_node_btree(retval,&curr,b);
      }
      else {
         *success = 1;
         return retval;
      }
   }
}

int main(void) {

   struct btree b;
   init_btree("btree.b",&b);

   unsigned long int cycles = 1000;

   for (unsigned long int i = 0; i < cycles; i++) {
      add_btree(i,&b);
   }

   putchar('\n');

   __uint8_t success = 0;

   for (unsigned long int i = 0; i < cycles; i++) {
      get_btree(i,&success,&b);
      printf("%08ld:%d ",i,success);
      if (i%16==0) putchar('\n');
   }
   putchar('\n');

   for (int i = 0; i < 256; i++) {
      printf("%ld ",b.root.elements[i]);
   }

   unload_btree(&b);

   return 0;
}
