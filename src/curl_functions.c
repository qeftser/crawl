
#include "curl_functions.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void dye_by_http_code(long code) {
   switch(code/100) {
      case 1:
         printf("\033[31m");
      break;
      case 2:
         printf("\033[32m");
      break;
      case 3:
         printf("\033[33m");
      break;
      case 4:
         printf("\033[34m");
      break;
      case 5:
         printf("\033[35m");
      break;
      default:
         printf("\033[36m");
      break;
   }
}

size_t generic_write_callback(void * data, size_t size, size_t nmemb, void * userdata) {
   size_t realsize = size * nmemb;

   struct html_buffer * buf = (struct html_buffer *)userdata;

   char * ptr = realloc(buf->data,buf->size + realsize + 1);
   if (!ptr) {
      fprintf(stderr,"heap exhaused\n");
   }

   buf->data = ptr;
   memcpy(&(buf->data[buf->size]),data,realsize);
   buf->size +=realsize;
   buf->data[buf->size] = 0;

   printf("realsize: %ld\n",realsize);

   return realsize;
}

void print_good_link(char * domain, char * path, char * new) {
   if (!strncmp(new,"http",4)) {
      printf("%s\n",new);
   }
   else if (*new == '/') {
      printf("%s%s\n",domain,new);
   }
   else {
      printf("%s/%s/%s\n",domain,path,new);
   }
}
