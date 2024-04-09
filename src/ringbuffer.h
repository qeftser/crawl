
#ifndef RINGBUFFER

#define RINGBUFFER
#include <stdint.h>
#include <stdlib.h>

struct ring_buffer { __uint64_t * buffer; __uint64_t pos; __uint64_t size; };

void init_ring_buffer(size_t, struct ring_buffer *);
void destroy_ring_buffer(struct ring_buffer *);

/* returns removed value */
__uint64_t add_ring_buffer(__uint64_t value, struct ring_buffer *);
__uint8_t query_ring_buffer(__uint64_t value, struct ring_buffer *);

__uint8_t delete_ring_buffer(__uint64_t value, struct ring_buffer *);
void clear_ring_buffer(struct ring_buffer *);

#endif
