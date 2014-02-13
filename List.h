#ifndef list_h
#define list_h
#include "CommonType.h"

typedef struct List List;
typedef struct List* PList;

struct List{
	int dataSize;
	int size;
	int capacity;
	int factor;
	void* data;
};

PList createList(int dataSize);
int addList(PList,void*);
#endif
