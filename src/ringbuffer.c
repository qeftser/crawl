
#include "ringbuffer.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

void init_ring_buffer(size_t buffer_size, struct ring_buffer * r) {
   r->pos = 0;
   r->size = buffer_size;
   r->buffer = malloc(sizeof(__uint64_t)*buffer_size);
   memset(r->buffer,0,sizeof(__uint64_t)*buffer_size);
}

void destroy_ring_buffer(struct ring_buffer * r) {
   free(r->buffer);
}

/* will return 0 until the ring starts filling up */
__uint64_t add_ring_buffer(__uint64_t value, struct ring_buffer * r) {
   __uint64_t ret = r->buffer[r->pos];
   r->buffer[r->pos] = value;
   r->pos = (r->pos+1)%r->size;
   return ret;
}

__uint8_t query_ring_buffer(__uint64_t value, struct ring_buffer * r) {
   for (__uint64_t i = 0; i < r->size; i++) {
      if (r->buffer[i] == value) return 1;
   }
   return 0;
}

__uint8_t delete_ring_buffer(__uint64_t value, struct ring_buffer * r) {
   for (__uint64_t i = 0; i < r->size; i++) {
      if (r->buffer[i] == value) { r->buffer[i] = 0; return 1; }
   }
   return 0;
}

void clear_ring_buffer(struct ring_buffer * r) {
   memset(r->buffer,0,sizeof(__uint64_t)*r->size);
   r->pos = 0;
}
