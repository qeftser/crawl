
#ifndef LOCAL_CURL_FUNCTIONS

#define LOCAL_CURL_FUNCTIONS
#include <stdlib.h>

struct html_buffer { char *data; size_t size; };

void dye_by_http_code(long);
size_t generic_write_callback(void *, size_t, size_t, void *);
void print_good_link(char *, char *, char *);


#endif
