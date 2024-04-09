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

inline __uint32_t F_md5(__uint32_t X, __uint32_t Y, __uint32_t Z) {
   return ((X & Y) | (~X & Z));
}

inline __uint32_t G_md5(__uint32_t X, __uint32_t Y, __uint32_t Z) {
   return ((X & Z) | (Y & ~Z));
}

inline __uint32_t H_md5(__uint32_t X, __uint32_t Y, __uint32_t Z) {
   return (X ^ Y ^ Z);
}

inline __uint32_t I_md5(__uint32_t X, __uint32_t Y, __uint32_t Z) {
   return (Y ^ (X | ~Z));
}

#define ROL32(x,n) (((x) << n) | ((x) >> (32-n)))

inline void FF_md5(__uint32_t * A, __uint32_t B, __uint32_t C, __uint32_t D, __uint16_t X, __uint8_t s, __uint32_t T) {
   *A = B + ROL32((*A + F_md5(B,C,D) + X + T),s);
}

inline void GG_md5(__uint32_t * A, __uint32_t B, __uint32_t C, __uint32_t D, __uint16_t X, __uint8_t s, __uint32_t T) {
   *A = B + ROL32((*A + G_md5(B,C,D) + X + T),s);
}

inline void HH_md5(__uint32_t * A, __uint32_t B, __uint32_t C, __uint32_t D, __uint16_t X, __uint8_t s, __uint32_t T) {
   *A = B + ROL32((*A + H_md5(B,C,D) + X + T),s);
}

inline void II_md5(__uint32_t * A, __uint32_t B, __uint32_t C, __uint32_t D, __uint16_t X, __uint8_t s, __uint32_t T) {
   *A = B + ROL32((*A + I_md5(B,C,D) + X + T),s);
}

__uint128_t md5(char * str) {

   size_t sl = strlen(str);

   /* 4096 bit max key size */
   __uint16_t M[256];
   __uint16_t X[16];
   memcpy(M,str,sl);
   M[sl] = 0x01;
   memset(M+(sl+1),0,256-(sl+1+4));
   memcpy(M+252,&sl,4);

   __uint32_t A = 0x67452301;
   __uint32_t B = 0xefcdab89;
   __uint32_t C = 0x98badcfe;
   __uint32_t D = 0x10325476;
   __uint32_t a, b, c, d;

   __uint8_t iteration = 0;

start:
   {
      memcpy(X,M+(16*iteration),32);
      a = A; b = B; c = C; d = D;

      FF_md5(&a,b,c,d,X[ 0], 7,0xd76aa478);
      FF_md5(&d,a,b,c,X[ 1],12,0xe8c7b756);
      FF_md5(&c,d,a,b,X[ 2],17,0x242070db);
      FF_md5(&b,c,d,a,X[ 3],22,0xc1bdceee);
      FF_md5(&a,b,c,d,X[ 4], 7,0xf57c0faf);
      FF_md5(&d,a,b,c,X[ 5],12,0x4787c62a);
      FF_md5(&c,d,a,b,X[ 6],17,0xa8304613);
      FF_md5(&b,c,d,a,X[ 7],22,0xfd469501);
      FF_md5(&a,b,c,d,X[ 8], 7,0x698098d8);
      FF_md5(&d,a,b,c,X[ 9],12,0x8b44f7af);
      FF_md5(&c,d,a,b,X[10],17,0xffff5bb1);
      FF_md5(&b,c,d,a,X[11],22,0x895cd7be);
      FF_md5(&a,b,c,d,X[12], 7,0x6b901122);
      FF_md5(&d,a,b,c,X[13],12,0xfd987193);
      FF_md5(&c,d,a,b,X[14],17,0xa679438e);
      FF_md5(&b,c,d,a,X[15],22,0x49b40821);

      GG_md5(&a,b,c,d,X[ 1], 5,0xf61e2562);
      GG_md5(&d,a,b,c,X[ 6], 9,0xc040b340);
      GG_md5(&c,d,a,b,X[11],14,0x265e5a51);
      GG_md5(&b,c,d,a,X[ 0],20,0xe9b6c7aa);
      GG_md5(&a,b,c,d,X[ 5], 5,0xd62f105d);
      GG_md5(&d,a,b,c,X[10], 9,0x02441453);
      GG_md5(&c,d,a,b,X[15],14,0xd8a1e681);
      GG_md5(&b,c,d,a,X[ 4],20,0xe7d3fbc8);
      GG_md5(&a,b,c,d,X[ 9], 5,0x21e1cde6);
      GG_md5(&d,a,b,c,X[14], 9,0xc33707d6);
      GG_md5(&c,d,a,b,X[ 3],14,0xf4d50d87);
      GG_md5(&b,c,d,a,X[ 8],20,0x455a14ed);
      GG_md5(&a,b,c,d,X[13], 5,0xa9e3e905);
      GG_md5(&d,a,b,c,X[ 2], 9,0xfcefa3f8);
      GG_md5(&c,d,a,b,X[ 7],14,0x676f02d9);
      GG_md5(&b,c,d,a,X[12],20,0x8d2a4c8a);

      HH_md5(&a,b,c,d,X[ 5], 4,0xfffa3942);
      HH_md5(&d,a,b,c,X[ 8],11,0x8771f681);
      HH_md5(&a,b,c,d,X[11],16,0x6d9d6122);
      HH_md5(&a,b,c,d,X[14],23,0xfde5380c);
      HH_md5(&a,b,c,d,X[ 1], 4,0xa4beea44);
      HH_md5(&d,a,b,c,X[ 4],11,0x4bdecfa9);
      HH_md5(&a,b,c,d,X[ 7],16,0xf6bb4b60);
      HH_md5(&a,b,c,d,X[10],23,0xbebfbc70);
      HH_md5(&a,b,c,d,X[13], 4,0x289b7ec6);
      HH_md5(&d,a,b,c,X[ 0],11,0xeaa127fa);
      HH_md5(&a,b,c,d,X[ 3],16,0xd4ef3085);
      HH_md5(&a,b,c,d,X[ 6],23,0x04881d05);
      HH_md5(&a,b,c,d,X[ 9], 4,0xd9d4d039);
      HH_md5(&d,a,b,c,X[12],11,0xe6db99e5);
      HH_md5(&a,b,c,d,X[15],16,0x1fa27cf8);
      HH_md5(&a,b,c,d,X[ 2],23,0xc4ac5665);

      II_md5(&a,b,c,d,X[ 0], 6,0xf4292244);
      II_md5(&d,a,b,c,X[ 7],10,0x432aff97);
      II_md5(&c,d,a,b,X[14],15,0xab9423a7);
      II_md5(&b,c,d,a,X[ 5],21,0xfc93a039);
      II_md5(&a,b,c,d,X[12], 6,0x655b59c3);
      II_md5(&d,a,b,c,X[ 3],10,0x8f0ccc92);
      II_md5(&c,d,a,b,X[10],15,0xffeff47d);
      II_md5(&b,c,d,a,X[ 1],21,0x85845dd1);
      II_md5(&a,b,c,d,X[ 8], 6,0x6fa87e4f);
      II_md5(&d,a,b,c,X[15],10,0xfe2ce6e0);
      II_md5(&c,d,a,b,X[ 6],15,0xa3014314);
      II_md5(&b,c,d,a,X[13],21,0x4e0811a1);
      II_md5(&a,b,c,d,X[ 4], 6,0xf7537e82);
      II_md5(&d,a,b,c,X[11],10,0xbd3af235);
      II_md5(&c,d,a,b,X[ 2],15,0x2ad7d2bb);
      II_md5(&b,c,d,a,X[ 9],21,0xeb86d391);

      A += a;
      B += b;
      C += c;
      D += d;
      if (iteration ^ 8) goto start;
   }

   __uint32_t outA[4];
   outA[0] = A; outA[1] = B; outA[2] = C; outA[3] = D;
   __uint128_t out;
   memcpy(&out,outA,8);
   return out;
}

