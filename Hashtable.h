#ifndef hashtable_h
#define hashtable_h
#include "CommonType.h"
#include <stdint.h>

typedef struct Hashtable Hashtable;
typedef union HashKey HashKey;
#define HASH_TYPE_NUMBER (0x00) 
#define HASH_TYPE_STRING (0x01)
union HashKey{
	uint64_t i;
	char* v;
};
Hashtable* newHashtable(int,int,int);
void initHashtable(Hashtable*,int,int,int);
int insertHashtable(Hashtable*,HashKey,char*);
void removeHashtable(Hashtable*,HashKey);
void* getHashtable(Hashtable*,HashKey);
void freeHashtable(Hashtable*,bool);
#endif
