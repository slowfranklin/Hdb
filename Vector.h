#ifndef vector_h
#define vector_h
#include "CommonType.h"
typedef struct Vector Vector;

struct Vector{
	void* data;
	int size;
	int capacity;
	int factor;
	int dataSize;
};

Vector* newVector(int dataSize,int factor);
void initVector(Vector* vector,int dataSize,int factor);
int addVector(Vector* vector,void* data);
void* getVectorByIndex(Vector* vector,int index);
int removeVectorByIndex(Vector* vector,int index);
void freeVector(Vector*,bool isNew);
#endif
