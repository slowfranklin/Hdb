#include "Hashtable.h"
#include "CommonType.h"
#include "murmur_hash.h"
#include <malloc.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define HASH_FACTOR (10)
#define HASH_GROW_FACTOR (2)



typedef struct HashElement HashElement;

struct HashElement{
	HashKey key;
	void *value;
	struct HashElement *next;
};

struct Hashtable{
	int hashType;
	unsigned int capacity;
	unsigned int size;
	unsigned int factor;
	unsigned int growFactor;
	struct HashElement** elements;
	FreeDelegate freeFunc;
	ForEachDelegate foreachFunc;
	CompareDelegate compareKey;
};

static int reHashTable(Hashtable* hashTable){
	int i;
	HashElement** _oldElements=NULL;
	if(hashTable->size == hashTable->capacity){
		int oldCapacity=hashTable->capacity;
		int newCapacity = hashTable->capacity * 2;
		if(newCapacity == 0)
			newCapacity = hashTable->factor;
		_oldElements = hashTable->elements;
		hashTable->elements = (HashElement**)malloc(sizeof(HashElement*) * newCapacity);
		if(!hashTable->elements){
			hashTable->elements = _oldElements;
			return 0;
		}
		memset(hashTable->elements,'\0',sizeof(HashElement*) * newCapacity);
		hashTable->capacity = newCapacity;
		hashTable->size = 0;
		for(i=0;i<oldCapacity;i++){
			HashElement* element = _oldElements[i];
			while(element){
				HashElement* tempElement;
				insertHashtable(hashTable,element->key,element->value);
				tempElement=element;
				element = element->next;
				free(tempElement);
			}
		}
		
		if(hashTable->freeFunc){
			
		}
		else{
			free(_oldElements);
		}
	}
	return 1;
}

Hashtable* newHashtable(int hashType,int factor,int growfactor){
	Hashtable* hashTable=NULL;
	size_t size=sizeof(struct Hashtable);
	hashTable=(Hashtable*)malloc(size);
	if(hashTable){
		memset(hashTable,'\0',size);
		if(factor > 0)
			hashTable->factor = factor;
		else
			hashTable->factor = HASH_FACTOR;
		if(growfactor > 0)
			hashTable->growFactor = growfactor;
		else
			hashTable->growFactor = HASH_GROW_FACTOR;
		
		hashTable->hashType=hashType;
		if(!reHashTable(hashTable)){
			if(hashTable->elements)
				free(hashTable->elements);
			free(hashTable);
			return NULL;
		}
	}
	return hashTable;
}

void initHashtable(Hashtable* hashTable,int hashType,int factor,int growfactor){
	memset(hashTable,'\0',sizeof(Hashtable));
	if(factor > 0)
		hashTable->factor = factor;
	else
		hashTable->factor = HASH_FACTOR;
	if(growfactor > 0)
		hashTable->growFactor = growfactor;
	else
		hashTable->growFactor = HASH_GROW_FACTOR;
}

int insertHashtable(Hashtable* hashTable,HashKey key,char* value){
	int index;
	uint64_t hashcode;
	HashElement* element=NULL;
	assert(hashTable);
	assert(value);
	reHashTable(hashTable);
	if(hashTable->hashType == HASH_TYPE_NUMBER){
		hashcode=key.i;
	}
	else if(hashTable->hashType == HASH_TYPE_STRING){
		hashcode = (uint64_t)murmur_hash2(key.v,strlen(key.v)); 
	}
	index = hashcode % hashTable->capacity;
	element = hashTable->elements[index];
	if(element){
		while(element->next){
			element = element->next;
		}
		element->next = (HashElement*)malloc(sizeof(HashElement));
		if(!element->next)
			return 0;
		element->next->key=key;
		element->next->value=value;
		element->next->next=NULL;
		hashTable->size++;
	}
	else{
		hashTable->elements[index] = (HashElement*)malloc(sizeof(HashElement));
		if(!hashTable->elements[index])
			return 0;
		hashTable->elements[index]->key=key;
		hashTable->elements[index]->value=value;
		hashTable->elements[index]->next=NULL;
		hashTable->size++;
	}
	return 1;
}

void removeHashtable(Hashtable* hashTable,HashKey key){
	int index,hashcode;
	HashElement* element=NULL;
	HashElement* preElement=NULL;
	assert(hashTable);
	if(hashTable->hashType == HASH_TYPE_NUMBER){
		hashcode=key.i;
	}
	else if(hashTable->hashType == HASH_TYPE_STRING){
		hashcode = (uint64_t)murmur_hash2(key.v,strlen(key.v)); 
	}	
	index = hashcode % hashTable->capacity;
	element = hashTable->elements[index];
	while(element){
		bool equal;
		if(hashTable->hashType == HASH_TYPE_NUMBER){
			equal = element->key.i == key.i;
		}
		else if(hashTable->hashType == HASH_TYPE_STRING){
			equal = element->key.v == key.v;
		}
		if(equal){
			if(preElement)
				preElement->next = element->next;
			else{
				hashTable->elements[index] = element->next;
			}
			hashTable->size--;
			free(element);
			break;
		}
		else{
			preElement = element;
			element = element->next;
		}
	}
	//free element
}

void* getHashtable(Hashtable* hashTable,HashKey key){
	int index,hashcode;
	HashElement* element=NULL;
	assert(hashTable);
	if(hashTable->hashType == HASH_TYPE_NUMBER){
		hashcode=key.i;
	}
	else if(hashTable->hashType == HASH_TYPE_STRING){
		hashcode = (uint64_t)murmur_hash2(key.v,strlen(key.v)); 
	}
	index = hashcode % hashTable->capacity;
	element = hashTable->elements[index];
	while(element){
		bool equal;
		if(hashTable->hashType == HASH_TYPE_NUMBER){
			equal = element->key.i == key.i;
		}
		else if(hashTable->hashType == HASH_TYPE_STRING){
			equal = element->key.v == key.v;
		}
		if(equal)
			return element->value;
		else{
			element = element->next;
		}
	}
	return NULL;
}

void freeHashtable(Hashtable* hashTable,bool isnew){
	int i;
	HashElement* element=NULL;
	assert(hashTable);
	for(i=0;i<hashTable->capacity;i++){
		//loop free
		element=hashTable->elements[i];
		while(element){
			HashElement* nextElement;
			nextElement=element->next;
			free(element);
			element=nextElement;
		}
	}
	if(hashTable->elements)
		free(hashTable->elements);
	if(isnew && hashTable)
		free(hashTable);
}
