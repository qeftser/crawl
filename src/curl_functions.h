
#ifndef LOCAL_CURL_FUNCTIONS

#define LOCAL_CURL_FUNCTIONS
#include <stdlib.h>
#include <curl/curl.h>

struct html_buffer { char * data; size_t size; };
struct uri_info { char domain[32]; char dir[992]; };

void print_all_between(char *, char *, char *);
void print_all_between_html_tag(char *, char *);
void print_remove_tags_html(char *);
void print_formatted_links(char *, struct uri_info *);

void dye_by_http_code(long);
size_t generic_write_callback(void *, size_t, size_t, void *);
void get_uri_info(char *, struct uri_info *);
void construct_relative_link(char *, char *, struct uri_info *);
long handle_url_verbose(char * url, CURL * handle, CURLcode * ret, struct html_buffer * buf, struct uri_info * state);


#endif
