
#include "parse.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

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
