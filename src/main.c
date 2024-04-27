
#include <curl/curl.h>
#include "curl_functions.h"
#include <stdlib.h>
#include <stdio.h>


int main(void) {

   curl_global_init(CURL_GLOBAL_DEFAULT);

   CURL * handle = curl_easy_init();
   CURLcode res;
   long http_code;
   struct html_buffer buffer = {0};
   struct uri_info info;
   curl_easy_setopt(handle,CURLOPT_VERBOSE,1L);
   curl_easy_setopt(handle,CURLOPT_WRITEFUNCTION,generic_write_callback);

   handle_url_verbose("https://en.wikipedia.org/wiki/Computer_science",handle,&res,&buffer,&info);


   free(buffer.data);

   curl_easy_cleanup(handle);
   curl_global_cleanup();

   return 0;

}

