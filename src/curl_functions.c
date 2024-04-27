
#include "curl_functions.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <curl/curl.h>
#include <time.h>

#define newline() printf("\n")

void print_all_between(char * str, char * begin, char * end) {
   char * ot, * cp = str;
   int el = strlen(end);
   int sl = strlen(begin);
start:
   cp = strstr(cp,begin);
   if (cp) cp+=sl; else { newline(); return; }
   ot = cp;
   while (1) {
      if (!ot || !(*ot)) break;
      if (!strncmp(ot,end,el)) {
         fwrite(cp,1,(ot-cp),stdout);
         cp+=(ot-cp);
         //putchar(32);
         newline();
         goto start;
      }
      ++ot;
   }
   newline();
}

void print_formatted_links(char * data, struct uri_info * state) {
   static const int sl = 6;
   static const int el = 1;
   char * ot, * cp = data;
   char in_buf[2048], out_buf[2048];
start:
   cp = strstr(cp,"href=\"");
   if (cp) cp+=sl; else { newline(); return; }
   ot = strchr(cp,'"');
   memcpy(in_buf,cp,(ot-cp));
   in_buf[(ot-cp)] = 0;
   construct_relative_link(in_buf,out_buf,state);
   printf("[ %-120s ] => [ %-140s ]\n",in_buf,out_buf);
   cp = ot;
   goto start;
}

long handle_url_verbose(char * url, CURL * handle, CURLcode * ret, struct html_buffer * buf, struct uri_info * state) {
   clock_t start = clock();
   clock_t read;
   clock_t collect;

   
   printf("Parsing from: %s\n",url);

   long ret_val;
   get_uri_info(url,state);
   printf("Domain: %s\n",state->domain);
   printf("Directory: %s\n",state->dir);

   curl_easy_setopt(handle,CURLOPT_URL,url);
   curl_easy_setopt(handle,CURLOPT_WRITEDATA,(void *)buf);
   *ret = curl_easy_perform(handle);
   read = clock() - start;
   printf("CURLcode: %u\n",*ret);

   curl_easy_getinfo(handle,CURLINFO_RESPONSE_CODE,&ret_val);
   dye_by_http_code(ret_val);
   printf("Status: %ld\033[0m\n",ret_val);
   printf("Collected Text:\n%s\n",buf->data);

   collect = clock();
   printf("Collected links:\n");
   print_formatted_links(buf->data,state);
   collect = clock() - collect;


   printf("Processing finished in %f seconds\n",(double)(clock()-start)/CLOCKS_PER_SEC);
   printf("%f seconds reading from the internet\n",(double)(read)/CLOCKS_PER_SEC);
   printf("%f seconds collecting links\n",(double)(collect)/CLOCKS_PER_SEC);
   return ret_val;
}

void print_all_between_html_tag(char * str, char * tag) {
   char * ot, * cp = str;
   char begin[32];
   strcpy(begin,"<"); strcat(begin,tag);
   char end[32];
   strcpy(end,"</"); strcat(end,tag); strcat(end,">");
   int el = strlen(end);
   int sl = strlen(begin);
start:
   cp = strstr(cp,begin);
   if (cp) {
      cp+=sl;
      cp = strchr(cp,'>');
      ++cp;
   }
   else {
      newline(); return;
   }
   ot = cp;
   while (1) {
      if (!ot || !(*ot)) break;
      if (!strncmp(ot,end,el)) {
         fwrite(cp,1,(ot-cp),stdout);
         cp+=(ot-cp);
         putchar(32);
         goto start;
      }
      ++ot;
   }
   newline();
}

void print_remove_tags_html(char * str) {
   char * ot, * cp = str;
   int len = strlen(str);
   while (1) {
      ot = strchr(cp,'<');
      if (!ot) {
         fprintf(stdout,"%s",cp);
         break;
      }
      else {
         fwrite(cp,1,(ot-cp),stdout);
      }
      cp = strchr(cp,'>');
      if (cp) ++cp;
   }
   newline();
}

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
   return realsize;
}

void get_uri_info(char * uri, struct uri_info * info_struct) {
   char * cut = strstr(uri,"//");
   cut += 2;
   cut = strchr(cut,'/');
   if (!cut) {
      strcpy(info_struct->domain,uri);
      *info_struct->dir = 0;
      return;
   }
   char * cur = strrchr(uri,'/');
   int amt = (cut-uri);
   memcpy(info_struct->domain,uri,amt);
   info_struct->domain[amt] = 0;
   amt = (cur-cut)+1;
   memcpy(info_struct->dir,cut,amt);
   info_struct->dir[amt] = 0;
}

void construct_relative_link(char * url, char * out, struct uri_info * state) {
   if (!strncmp(url,"http",4)) {
      strcpy(out,url);
   }
   else if (*url == '/') {
      if (*(url+1) == '/') {
         strcpy(out,"https:"); strcat(out,url); return; // doesn't account for http resolves
      }
      strcpy(out,state->domain); strcat(out,url);
   }
   else {
      *out = 0;
   }
}
