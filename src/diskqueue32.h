
#ifndef DISKQUEUE_32

#define DISKQUEUE_32

#define DISKQUEUE32_BLOCKSIZE 4096
#define DISKQUEUE32_MAX_ELEMENTS 1020

struct diskqueue32 { int fptr; int size; int head; int tail; 
                   int hdata[DISKQUEUE32_BLOCKSIZE/sizeof(int)];
                   int tdata[DISKQUEUE32_BLOCKSIZE/sizeof(int)]; 
                   struct diskstack * block_stack; };



/*
#define DQ_NP 1022
#define DQ_PP 1023
#define DQ_LH 1020
#define DQ_LT 1021
*/
#define DQ_NP 1020
#define DQ_PP 1021
#define DQ_LH 1022
#define DQ_LT 1023

#endif


