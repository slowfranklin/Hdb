#ifndef dlist_h
#define dlist_h
#include "CommonType.h"
typedef struct DList DList;
typedef struct DListNode DListNode;

struct DList{
	DListNode* head;
	DListNode* tail;
	int size;
};

struct DListNode{
	DListNode* prev;
	DListNode* next;
	void* data;
};

DList* newDList(void);
int addHeadDList(DList*,void*);
int addTailDList(DList*,void*);
void* addAndFecthTailDList(DList*,void*);
void* fetchHeadDList(DList*);
void* fetchTailDList(DList*);
void* fetchHeadNodeDList(DList*);
void* removeHeadDList(DList*);
void* removeTailDList(DList*);
void* removeDListByIndex(DList*,int);
void* removeDListNode(DList*,void*);

void moveDListNodeToTail(DList*,DListNode*);
void freeDList(DList* dlist,bool isnew);
#endif
