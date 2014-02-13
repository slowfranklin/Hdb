#include "Hashtable.h"
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
int main(){
	uint32_t i=1,state;
	Hashtable* hashTable=newHashtable(0,0,0);
	HashKey hashKey;
	
	for(;i < 1000;i++){
		void* data=NULL;
		hashKey.i=i;
		setU32(&data,0,i);
		state=insertHashtable(hashTable,hashKey,data);
		assert(state == 1);
	}
	for(i=1;i < 1000;i++){
		int key;
		void* data=NULL;
		hashKey.i=i;
		data=getHashtable(hashTable,hashKey);
		assert(data != NULL);
		key=getU32(&data,0);
		assert(i == key);
	}
	
	
	for(i=300;i<500;i++){
		hashKey.i=i;
		removeHashtable(hashTable,hashKey);
	}
	
	/*
	for(i=0;i < 1000;i++){
		int key;
		void* data=NULL;
		hashKey.i=i;
		data=getHashtable(&hashTable,hashKey);
		assert(data != NULL);
		key=geti32(&data,0);
		assert(i == key);
	}
	*/
	return 1;
}