#include "LinkList.h"
#include <assert.h>
#include <stdlib.h>

typedef struct LinkListNode LinkListNode;
typedef struct LinkListNode LinklistNode;

struct LinkListNode{
	void* data;
	struct LinkListNode* next;
};

struct LinkList{
	LinkListNode* head;
	int dataSize;
	int size;
};

Linklist* newLinklist(){
	return NULL;
}

void initLinklist(Linklist* linklist){

}

int pushLinklist(Linklist* linklist,void* data){
	assert(linklist);
	return 1;
}

void* popLinklist(Linklist* linklist){
	assert(linklist);
	return NULL;
}

void freeLinklist(Linklist* linklist){
	LinkListNode * node;
	assert(linklist);
	node=linklist->head;
	while(node){
		LinkListNode* tempNode=node->next;
		free(node);
		node=tempNode;
	}
	free(linklist);
}