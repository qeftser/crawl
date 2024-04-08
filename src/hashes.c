#include "hashes.h"
#include <string.h>

__uint64_t fnv1a_64(char * str) {
   __uint64_t hash = 0xcbf29ce484222325llu;

   while (*str) {
      hash ^= *str;
      hash *= 0x00000100000001b3llu;
      str++;
   }

   return hash;
}

__uint32_t murmer3_32(char * str, __uint32_t seed) {
   __uint32_t hash = seed; // seed

   size_t len = strlen(str);
   size_t lencpy = len;

   __uint32_t block;

   while (len > 3) {
      memcpy(&block,str,4);
      block *= 0xcc9e2d51;
      block = ((block >> 17) | (block << 15));
      block *= 0x1b873593;
      hash  ^= block;
      hash  = ((block >> 19) | (block << 13));
      hash = (hash * 5) + 0xe6546b64;
      len -= 4;
      str += 4;
   }

   if (len) {
      memcpy(&block,str,len);
      block *= 0xcc9e2d51;
      block = ((block >> 17) | (block << 15));
      block *= 0x1b873593;
      hash  ^= block;
   }

   hash ^= lencpy;
   hash ^= (hash >> 16);
   hash *= 0x85ebca6b;
   hash ^= (hash >> 13);
   hash *= 0xc2b2ae35;
   hash ^= (hash >> 16);

   return hash;
}

__uint64_t siphash_2_4_128(char * str, __uint64_t seed1, __uint64_t seed2) {
   __uint64_t v0 = (seed1)^0x736f6d6570736575llu;
   __uint64_t v1 = (seed2)^0x646f72616e646f6dllu;
   __uint64_t v2 = (seed1)^0x6c7967656e657261llu;
   __uint64_t v3 = (seed2)^0x7465646279746573llu;

   size_t sl = strlen(str);
   __uint8_t rounds = sl/8;
   __uint8_t extra = sl%8;
   __uint64_t block;

   while (rounds--) {
      memcpy(&block,str,8);
      v3 ^= block;
      {
         v0 += v1; v2 += v3;
         v1 = (v1 << 13) | (v1 >> 51);
         v3 = (v3 << 16) | (v3 >> 48);
         v1 ^= v0; v3 ^= v2;
         v0 = (v0 << 32) | (v0 >> 32);
         v2 += v1; v0 += v3;
         v1 = (v1 << 17) | (v1 >> 47);
         v3 = (v3 << 21) | (v3 >> 43);
         v1 ^= v2; v3 ^= v0;
         v2 = (v2 << 32) | (v2 >> 32);

         v0 += v1; v2 += v3;
         v1 = (v1 << 13) | (v1 >> 51);
         v3 = (v3 << 16) | (v3 >> 48);
         v1 ^= v0; v3 ^= v2;
         v0 = (v0 << 32) | (v0 >> 32);
         v2 += v1; v0 += v3;
         v1 = (v1 << 17) | (v1 >> 47);
         v3 = (v3 << 21) | (v3 >> 43);
         v1 ^= v2; v3 ^= v0;
         v2 = (v2 << 32) | (v2 >> 32);
      }
      v0 ^= block;
      str+=8;
   }

   memset(&block,0,8);
   memcpy(&block,str,extra);
   block |= (((sl)%256)<<56);
   v3 ^= block;
   {
      v0 += v1; v2 += v3;
      v1 = (v1 << 13) | (v1 >> 51);
      v3 = (v3 << 16) | (v3 >> 48);
      v1 ^= v0; v3 ^= v2;
      v0 = (v0 << 32) | (v0 >> 32);
      v2 += v1; v0 += v3;
      v1 = (v1 << 17) | (v1 >> 47);
      v3 = (v3 << 21) | (v3 >> 43);
      v1 ^= v2; v3 ^= v0;
      v2 = (v2 << 32) | (v2 >> 32);

      v0 += v1; v2 += v3;
      v1 = (v1 << 13) | (v1 >> 51);
      v3 = (v3 << 16) | (v3 >> 48);
      v1 ^= v0; v3 ^= v2;
      v0 = (v0 << 32) | (v0 >> 32);
      v2 += v1; v0 += v3;
      v1 = (v1 << 17) | (v1 >> 47);
      v3 = (v3 << 21) | (v3 >> 43);
      v1 ^= v2; v3 ^= v0;
      v2 = (v2 << 32) | (v2 >> 32);
   }
   v0 ^= block;
   v2 ^= 0x00000000000000ffllu;
   {
      v0 += v1; v2 += v3;
      v1 = (v1 << 13) | (v1 >> 51);
      v3 = (v3 << 16) | (v3 >> 48);
      v1 ^= v0; v3 ^= v2;
      v0 = (v0 << 32) | (v0 >> 32);
      v2 += v1; v0 += v3;
      v1 = (v1 << 17) | (v1 >> 47);
      v3 = (v3 << 21) | (v3 >> 43);
      v1 ^= v2; v3 ^= v0;
      v2 = (v2 << 32) | (v2 >> 32);

      v0 += v1; v2 += v3;
      v1 = (v1 << 13) | (v1 >> 51);
      v3 = (v3 << 16) | (v3 >> 48);
      v1 ^= v0; v3 ^= v2;
      v0 = (v0 << 32) | (v0 >> 32);
      v2 += v1; v0 += v3;
      v1 = (v1 << 17) | (v1 >> 47);
      v3 = (v3 << 21) | (v3 >> 43);
      v1 ^= v2; v3 ^= v0;
      v2 = (v2 << 32) | (v2 >> 32);

      v0 += v1; v2 += v3;
      v1 = (v1 << 13) | (v1 >> 51);
      v3 = (v3 << 16) | (v3 >> 48);
      v1 ^= v0; v3 ^= v2;
      v0 = (v0 << 32) | (v0 >> 32);
      v2 += v1; v0 += v3;
      v1 = (v1 << 17) | (v1 >> 47);
      v3 = (v3 << 21) | (v3 >> 43);
      v1 ^= v2; v3 ^= v0;
      v2 = (v2 << 32) | (v2 >> 32);

      v0 += v1; v2 += v3;
      v1 = (v1 << 13) | (v1 >> 51);
      v3 = (v3 << 16) | (v3 >> 48);
      v1 ^= v0; v3 ^= v2;
      v0 = (v0 << 32) | (v0 >> 32);
      v2 += v1; v0 += v3;
      v1 = (v1 << 17) | (v1 >> 47);
      v3 = (v3 << 21) | (v3 >> 43);
      v1 ^= v2; v3 ^= v0;
      v2 = (v2 << 32) | (v2 >> 32);
   }
   return v0^v1^v2^v3;
}
