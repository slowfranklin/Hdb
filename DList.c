#include "DList.h"
#include <memory.h>
#include <assert.h>
#include <stdlib.h>

DList* newDList(void){
	DList* dlist = (DList*)malloc(sizeof(DList));
	if(dlist)
		memset(dlist,'\0',sizeof(DList));
	return dlist;
}

int addTailDList(DList* dlist,void* data){
	DListNode* listnode = (DListNode*)malloc(sizeof(DListNode));
	if(!listnode)
		return 0;
	listnode->next = NULL;
	listnode->prev = dlist->tail;
	listnode->data=data;

	if(dlist->tail)
		dlist->tail->next = listnode;
	if(!dlist->head)
		dlist->head = listnode;
	dlist->tail = listnode;
	dlist->size++;
	return 1;
}

int addHeadDList(DList* dlist,void* data){
	DListNode* listnode =(DListNode*)malloc(sizeof(DListNode));
	if(!listnode)
		return 0;
	listnode->prev = NULL;
	listnode->next = dlist->head;
	listnode->data = data;

	if(dlist->head)
		dlist->head->prev = listnode;
	if(!dlist->tail)
		dlist->tail = listnode;

	dlist->head = listnode;
	dlist->size++;
	return 1;
}

void* addAndFecthTailDList(DList* dlist,void* data){
	DListNode* listnode = (DListNode*)malloc(sizeof(DListNode));
	if(!listnode)
		return 0;
	listnode->next = NULL;
	listnode->prev = dlist->tail;
	listnode->data=data;

	if(dlist->tail)
		dlist->tail->next = listnode;
	if(!dlist->head)
		dlist->head = listnode;
	dlist->tail = listnode;
	dlist->size++;
	return listnode;
}

void* fetchTailDList(DList* dlist){
	assert(dlist);
	if(dlist->tail)
		return dlist->tail->data;
	return NULL;
}

void* fetchHeadDList(DList* dlist){
	assert(dlist);
	if(dlist->head)
		return dlist->head->data;
	return NULL;
}

void* fetchHeadNodeDList(DList* dlist){
	assert(dlist);
	return dlist->head;
}

void* removeTailDList(DList* dlist){
	void* data;
	DListNode* tailNode = dlist->tail;
	assert(dlist);
	if(tailNode){
		if(tailNode->prev){
			tailNode->prev->next = NULL;
			dlist->size--;
		}
		else{
			dlist->head=dlist->tail=NULL;
			dlist->size--;
			assert(dlist->size ==0);
		}
		data=tailNode->data;
		free(tailNode);
		return data;
	}
	return NULL;
}

void* removeHeadDList(DList* dlist){
	void* data;
	DListNode* headNode = dlist->head;
	assert(dlist);
	if(headNode){
		if(headNode->next){
			dlist->head=headNode->next;
			headNode->next->prev=NULL;
			dlist->size--;
		}
		else{
			dlist->head=dlist->tail=NULL;
			dlist->size--;
			assert(dlist->size ==0);
		}
		data=headNode->data;
		free(headNode);
		return data;
	}
	return NULL;
}

void* removeDListNode(DList* dlist,void* nodeAddress){
	void* data;
	DListNode* listnode;
	assert(dlist);
	assert(nodeAddress);
	listnode =(DListNode*)nodeAddress;
	if(listnode->prev)
		listnode->prev->next=listnode->next;
	else
		dlist->head=listnode->next;
	
	if(listnode->next)
		listnode->next->prev=listnode->prev;
	else
		dlist->tail=listnode->next;
	data=listnode->data;
	dlist->size--;
	free(listnode);
	return data;
}

void moveDListNodeToTail(DList* dlist,DListNode* node){
	if(dlist->tail != node){
		if(dlist->head == node){
			assert(node->next !=NULL);
			dlist->head = node->next;
			node->next->prev=NULL;
		}
		else{
			node->prev->next=node->next;
			node->next->prev=node->prev;
		}
		dlist->tail->next=node;
		node->next=NULL;
		node->prev=dlist->tail;
		dlist->tail=node;
	}
}

void freeDList(DList* dlist,bool isnew){
	DListNode* listnode,*nextnode;
	listnode = dlist->head;
	while(listnode){
		nextnode = listnode->next;
		free(listnode);
		listnode = nextnode;
	}
	if(isnew && dlist)
		free(dlist);
}
