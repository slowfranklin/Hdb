#include "List.h"
#include <malloc.h>
#include <assert.h>
#include <stddef.h>

static int reList(){
	void* data = (void*)calloc(list->data,list->capacity * 2 * list->dataSize);
	if(data==NULL)
		return 0;
	list->data = data;
	list->capacity *= 2;
	return 1;
}

PList createList(int dataSize){
	List* list=NULL;
	assert(dataSize > 0);
	list = malloc(sizeof(List));
	if(list != NULL){
		memset(list,0,sizeof(List));
		list->data = (void*)malloc(list->factor * dataSize);
		if(list->data ==NULL)
			goto ERROR;
		list->dataSize = dataSize;
		list->capacity = list->factor;
	}
	return list;
ERROR:
	if(list)
		free(list);
	return list;
}

int addList(PList list,void* data){
	assert(list);
	if(list->size == list->capacity){
		if(!reList())
			return 0;
	}
	memcpy(list->data + list->dataSize * list->size,data,list->dataSize);
	return 1;
}
int delListByIndex(PList list,int index){
	assert(index >=0 && index < list->size);
	if(index >0 && index < list->size){
		
		return 1;
	}
}

void freeList(PList list){
	if(list){
		free(list->data);
		free(list);
		list =NULL;
	}
}
