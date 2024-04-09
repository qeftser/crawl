
#ifndef SKIP_LIST

#define SKIP_LIST
#include <stdint.h>
#include <stdlib.h>

#define SKIP_LIST_LAYERS 7

struct skip_list { size_t size; struct skip_list_node * head; struct skip_list_bank * bank; };
struct skip_list_node { struct skip_list_node * next[SKIP_LIST_LAYERS]; __uint64_t key; void * data; };
struct skip_list_bank { struct skip_list_node ** store; size_t pos; };

void init_skip_list(size_t, struct skip_list_bank *, struct skip_list *);
void insert_skip_list(__uint64_t, void *, struct skip_list *);
void destroy_skip_list(struct skip_list *);
void print_skip_list(struct skip_list *);

#endif
