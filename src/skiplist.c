
#include "skiplist.h"
#include "btree.h"
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <limits.h>

void init_skip_list_bank(size_t elements, struct skip_list_bank * b) {
   b->store = malloc(sizeof(struct skip_list_node *)*elements);
   b->pos = elements;
   for (size_t i = 0; i < elements; ++i) {
      b->store[i] = malloc(sizeof(struct skip_list_node));
   }
}

struct skip_list_node * checkout_skip_list_bank(struct skip_list_bank * b) {
   if (b->pos == 0) return (struct skip_list_node *)(-1);
   return b->store[--(b->pos)];
}

void return_skip_list_bank(struct skip_list_node * n, struct skip_list_bank * b) {
   b->store[(b->pos)] = n; ++(b->pos);
}

void init_skip_list(size_t max_size, struct skip_list_bank * b, struct skip_list * s) {
   srand(clock());
   init_skip_list_bank(max_size,b);
   s->bank = b;
   s->size = 0;
   struct skip_list_node * front = malloc(sizeof(struct skip_list_node));
   for (int i = 0; i < SKIP_LIST_LAYERS; i++) {
      front->next[i] = 0;
   }
   front->key = 0;         front->data = 0;
   s->head = front;
}

void destroy_skip_list(struct skip_list * s) {
   for (size_t i = 0; i < s->bank->pos; ++i) {
      free(s->bank->store[i]);
   }
   struct skip_list_node * yeet;
   struct skip_list_node * i = s->head;
   do {
      yeet = i; i = i->next[0];
      free(yeet);
   } while (i);
   free(s->bank->store);
}

/* name from sedgewick! */
__uint8_t randX(__uint8_t j) {
   static int chance = RAND_MAX / SKIP_LIST_LAYERS;
   __uint8_t sum = 0;
   do {
      if (rand()-chance < 0) sum++;
   } while (--j);
   return sum;
}

void insert_skip_list(__uint64_t key, void * data, struct skip_list * s) {
   struct skip_list_node * n = checkout_skip_list_bank(s->bank);
   n->key = key; n->data = data;
   memset(n->next,0,sizeof(struct skip_list_node *)*SKIP_LIST_LAYERS);
   __uint8_t depth = SKIP_LIST_LAYERS-1;
   __uint8_t link_depth = randX(SKIP_LIST_LAYERS-1);
   struct skip_list_node * i = s->head;
   while (1) {
      if (!i->next[depth] || i->next[depth]->key > key) {
         if (depth <= link_depth) {
            n->next[depth] = i->next[depth];
            i->next[depth] = n;
         }
         if (depth) --depth;
         else break;
      }
      else {
         i = i->next[depth];
      }
   }
   s->size++;
}

void * delete_skip_list(__uint64_t key, struct skip_list * s) {
   if (!key) return (void *)-1; /* we don't support deleting 0 */
   struct skip_list_node * i = s->head;
   struct skip_list_node * p[SKIP_LIST_LAYERS] = { 0 };
   __uint8_t depth = (SKIP_LIST_LAYERS-1);
   __uint8_t height = 0;
   while (1) {
      if (!i->next[depth]) {
         if (depth) depth--; else break;
      }
      else if (i->next[depth]->key == key) {
         if (!height) height = depth;
         p[depth] = i;
         if (depth) { --depth; continue; }
         i = i->next[0];
         for (int j = 0; j <= height; ++j) {
            p[j]->next[j] = i->next[j];
         }
         return_skip_list_bank(i,s->bank);
         return i->data;
      }
      else if (i->next[depth]->key >= key) {
         if (depth) --depth; else break;
      }
      else {
         i = i->next[depth];
      }
   }
   return (void *)-1;
}

/* will only ever return 0 for 0 */
void * query_skip_list(__uint64_t key, int * steps, struct skip_list * s) {
   struct skip_list_node * i = s->head;
   __uint8_t depth = (SKIP_LIST_LAYERS-1);
   while (1) {
      if (i->key == key) { return i->data;}
      else if (!i->next[depth] || i->next[depth]->key > key) {
         if (depth) depth--; else break;
      }
      else {
         i = i->next[depth];
         if (steps) *steps = *steps+1;
      }
   }
   return (void *)-1;
}

void print_skip_list(struct skip_list * s) {
   struct skip_list_node * n = s->head;
   int len;
   do {
      len = 1;
      for (int i = 1; i < SKIP_LIST_LAYERS; i++) if (n->next[i]) len++;
      printf(".________________.-"); for (int i = 1; i < len; i++) { printf(".__.-"); } putchar('\n');
      printf("|%-' '16lx|",n->key); for (int i = 1; i < len; i++) { printf("|   |"); } putchar('\n');
      printf(".________________.-"); for (int i = 1; i < len; i++) { printf(".__.-"); } putchar('\n');
      printf("        ||         "); for (int i = 1; i < len; i++) { printf(" ||  "); } putchar('\n');
      n = n->next[0];
   } while (n);
   len = SKIP_LIST_LAYERS;
   printf(".________________.-"); for (int i = 1; i < len; i++) { printf(".__.-"); } putchar('\n');
   printf("|       X        |"); for (int i = 1; i < len; i++) { printf("|   |"); } putchar('\n');
   printf(".________________.-"); for (int i = 1; i < len; i++) { printf(".__.-"); } putchar('\n');
}

/*
int main(void) {

   const long cycles = 1000000;

   int elements[cycles];

   for (int i = 0; i < cycles; ++i) {
      elements[i] = rand();
   }
   
   struct skip_list sk;
   struct skip_list_bank b;
   init_skip_list(cycles,&b,&sk);

   for (long i = 1; i < cycles; ++i) {
      insert_skip_list(elements[i],(void *)i,&sk);
   }
   int checks;
   long total = 0;

   for (long i = 1; i < cycles; ++i) {
      checks = 0;
      printf("checking: %ld\n",i);
      query_skip_list(elements[i],&checks,&sk);
      total += checks;
   }

   for (long i = 1; i < cycles; ++i) {
      delete_skip_list(i,&sk);
   }

   for (long i = 1; i < cycles; ++i) {
      insert_skip_list(i,(void *)i,&sk);
   }

   //print_skip_list(&sk);

   //printf("\nLinked List Nodes Visited: %lu\n",(long)(cycles*(cycles+1))/2);
   //printf("\nTotal Nodes Visited: %lu\n",total);
   //printf("\nAverage Nodes Visited: %f\n",(double)total/cycles);
   //printf("\nVisit Ratio Compared With Linked List: %f\n",(double)((double)total)/((double)(cycles*(cycles+1))/2));

   destroy_skip_list(&sk);


}
*/

