#include "Vector.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define DEFAULT_VECTOR_FACTOR (10)

static int reVector(Vector* vector){
	if(vector->capacity == vector->size){
		void* data;
		int newCapacity = vector->capacity * 2;
		if(newCapacity == 0)
			newCapacity = vector->factor;
		data =(void*)realloc(vector->data,vector->dataSize * newCapacity);
		if(!data){
			return 0;
		}
		vector->data = data;
		vector->capacity = newCapacity;
	}
	return 1;
}

Vector* newVector(int dataSize,int factor){
	Vector* vector;
	vector=(Vector*)malloc(sizeof(Vector));
	if(vector){
		memset(vector,'\0',sizeof(Vector));
		vector->dataSize = dataSize;
		if(factor > 0) vector->factor = factor;
		else vector->factor = DEFAULT_VECTOR_FACTOR;
	}
	return vector;
}

void initVector(Vector* vector,int dataSize,int factor){
	assert(vector);
	if(vector){
		memset(vector,'\0',sizeof(*vector));
		vector->dataSize = dataSize;
		if(factor > 0) vector->factor = factor;
		else vector->factor = DEFAULT_VECTOR_FACTOR;
	}
}

int addVector(Vector* vector,void* data){
	assert(vector);
	assert(data);
	reVector(vector);
	memcpy(vector->data + vector->dataSize * vector->size,data,vector->dataSize);
	vector->size++;
	return 1;
}

void* getVectorByIndex(Vector* vector,int index){
	assert(vector);
	if(index >=0 && index < vector->size){
		return vector->data + vector->dataSize * index;
	}
	return NULL;
}

int removeVectorByIndex(Vector* vector,int index){
	assert(vector);
	if(vector->size > 0 && (index >=0 && index < vector->size)){
		memmove(vector->data + vector->dataSize * index,vector->data + vector->dataSize * (index + 1),(vector->size - index -1) * vector->dataSize);
		vector->size--;
	}
	return 1;
}

void freeVector(Vector* vector,bool isNew){
	assert(vector);
	if(vector){
		if(vector->data)
			free(vector->data);
		if(isNew)
			free(vector);
	}
}
