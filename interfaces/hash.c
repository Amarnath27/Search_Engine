#include <string.h>
#include <stdlib.h>
#include "hash.h"

//this is the implementation of the hash function

unsigned long hash1(char* str) {
     unsigned long hash = 5381;
     int c;

     while ((c = *str++) != 0)
          hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
     return hash;
}
