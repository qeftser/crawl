
/* bloom filters seem nice */
#include "bloom.h"
#include "hashes.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#define BLOOM_FILTER_SIZE 0xffffffffllu

void init_bloom(__uint8_t * a, size_t alen) {
   if (alen != BLOOM_FILTER_SIZE) {
      fprintf(stderr, "array has incorrect size 0x%016lx - should be %016llx\n",alen,BLOOM_FILTER_SIZE);
      exit(1);
   }
   memset(a,0,alen);
}

int load_bloom(char * filename, __uint8_t * a) {
   if (access(filename,F_OK) == 0) {
      int fptr = open(filename,O_RDONLY);
      if (fptr == -1) {
         perror("open");
         return -1;
      }
      int ret = read(fptr,a,BLOOM_FILTER_SIZE);
      if (ret == -1) {
         perror("read");
         return -1;
      }
      close(fptr);
      return 0;
   }
   else {
      fprintf(stderr, "failed loading file\n");
      return -1;
   }
}

int unload_bloom(char * filename, __uint8_t * a) {
   int fptr = open(filename,O_WRONLY|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR);
   if (fptr == -1) {
      perror("open");
      return -1;
   }
   int ret = write(fptr,a,BLOOM_FILTER_SIZE);
   if (ret == -1) {
      perror("write");
      return -1;
   }
   close(fptr);
   return 0;
}

#define GETBIT(x) ((a[x/8llu]&(0x1<<(x%8)))>>(x%8))
#define SETBIT(x) (a[x/8llu]|=(0x1<<(x%8)))

__uint8_t check_bloom(char * str, __uint8_t * a) {
   __uint32_t idx0 = siphash_2_4_128(str,0xab2452b234a4e52c,0x20941cd432ff143f)%(BLOOM_FILTER_SIZE- 0);
   __uint32_t idx1 = siphash_2_4_128(str,0x811c9dc5811c9dc5,0x811c9dc5811c9dc5)%(BLOOM_FILTER_SIZE- 3);
   __uint32_t idx2 = siphash_2_4_128(str,0xb95e09d3fc509a34,0xe3a0b52c94d509f8)%(BLOOM_FILTER_SIZE- 5);
   __uint32_t idx3 = siphash_2_4_128(str,0xf0e1d2c3b4a59687,0x78695a4b3c2d1e0f)%(BLOOM_FILTER_SIZE-11);
   __uint32_t idx4 = siphash_2_4_128(str,0xaecf8acea8caef2a,0x2a2a4a3a9e83d9f3)%(BLOOM_FILTER_SIZE-13);
   __uint32_t idx5 = siphash_2_4_128(str,0xf98d89c87b9878a8,0x12f243a45d57f4b5)%(BLOOM_FILTER_SIZE-13);
   return
   GETBIT(idx0)& GETBIT(idx1)& GETBIT(idx2)& GETBIT(idx3)& GETBIT(idx4)& GETBIT(idx5);
}

void insert_bloom(char * str, __uint8_t * a) {
   __uint32_t idx0 = siphash_2_4_128(str,0xab2452b234a4e52c,0x20941cd432ff143f)%(BLOOM_FILTER_SIZE- 0);
   __uint32_t idx1 = siphash_2_4_128(str,0x811c9dc5811c9dc5,0x811c9dc5811c9dc5)%(BLOOM_FILTER_SIZE- 3);
   __uint32_t idx2 = siphash_2_4_128(str,0xb95e09d3fc509a34,0xe3a0b52c94d509f8)%(BLOOM_FILTER_SIZE- 5);
   __uint32_t idx3 = siphash_2_4_128(str,0xf0e1d2c3b4a59687,0x78695a4b3c2d1e0f)%(BLOOM_FILTER_SIZE-11);
   __uint32_t idx4 = siphash_2_4_128(str,0xaecf8acea8caef2a,0x2a2a4a3a9e83d9f3)%(BLOOM_FILTER_SIZE-13);
   __uint32_t idx5 = siphash_2_4_128(str,0xf98d89c87b9878a8,0x12f243a45d57f4b5)%(BLOOM_FILTER_SIZE-13);
   SETBIT(idx0); SETBIT(idx1); SETBIT(idx2); SETBIT(idx3); SETBIT(idx4); SETBIT(idx5);
}

void remove_newline_fgets(char * str) {
   size_t strl = strlen(str);
   str[strl-1]='\0';
}

/*
int main(void) {


   __uint8_t * bloom = malloc((__uint64_t)sizeof(__uint8_t)*BLOOM_FILTER_SIZE);
   printf("allocating %016llx\n",(__uint64_t)sizeof(__uint8_t)*BLOOM_FILTER_SIZE);
   init_bloom(bloom,BLOOM_FILTER_SIZE);

   const long long unsigned int cycles = 0x00ffffff;

   char in[256];

   
   //while (1) {
   //   printf("Enter string: \n");
   //   remove_newline_fgets(fgets(in,255,stdin));
   //   if (strlen(in) == 1 && *in == 'q') break;
   //   printf("\nGet: %d\n",check_bloom(in,bloom));
   //   insert_bloom(in,bloom);
   //}
   

   long long unsigned int collisions = 0;

   clock_t start = clock();


   for (long long unsigned int i = 0; i < cycles; i++) {
      printf("%08llx ",i);
      if (i%30 == 0) putchar('\n');
      in[0] = (i>> 0)&0xff;
      in[1] = (i>> 8)&0xff;
      in[2] = (i>>16)&0xff;
      in[3] = (i>>24)&0xff;
      in[4] = (i>>32)&0xff;
      in[5] = (i>>40)&0xff;
      in[6] = (i>>48)&0xff;
      in[7] = (i>>56)&0xff;
      if (check_bloom(in,bloom)) collisions++;
      insert_bloom(in,bloom);
   }
   putchar('\n');

   printf("total inserts: %016llx\n",cycles);
   printf("total collisi: %016llx\n",collisions);
   printf("collisi ratio: %f\n",(double)collisions/cycles);

   printf("\ntime elapsed   : %f\n",(double)(clock() - start)/CLOCKS_PER_SEC);
   printf("time per insert: %f\n",((double)(clock() - start)/CLOCKS_PER_SEC)/(double)(cycles/0xff));

   free(bloom);

   return 0;

}
*/
