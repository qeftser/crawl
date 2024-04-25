
#include <curl/curl.h>
#include "curl_functions.h"
#include "parse.h"
#include <stdlib.h>
#include <stdio.h>


int main(void) {

   curl_global_init(CURL_GLOBAL_DEFAULT);

   CURL * handle = curl_easy_init();
   CURLcode res;
   long http_code;
   struct html_buffer buffer = {0};
   curl_easy_setopt(handle,CURLOPT_URL,"https://curl.se/libcurl/c/CURLOPT_VERBOSE.html");
   curl_easy_setopt(handle,CURLOPT_VERBOSE,1L);
   curl_easy_setopt(handle,CURLOPT_WRITEFUNCTION,generic_write_callback);
   curl_easy_setopt(handle,CURLOPT_WRITEDATA, (void *)&buffer);

   res = curl_easy_perform(handle);

   printf("result: %d\n",res);

   res = curl_easy_getinfo(handle,CURLINFO_RESPONSE_CODE,&http_code);

   dye_by_http_code(http_code);
   printf("http_code: %ld\n\033[0m",http_code);
   //printf("data:\n%s",buffer.data);

   printf("result: %d\n",res);

   curl_easy_cleanup(handle);

   printf("\n\nParse\n\n");

   print_all_between(buffer.data,"href=\"","\"");

   /*
   printf("\n\n");

   //print_all_between_html_tag(buffer.data,"div");

   printf("\n\n");

   //print_remove_tags_html(buffer.data);

   printf("\n\n");
   */

   free(buffer.data);
   curl_global_cleanup();

   return 0;

}

