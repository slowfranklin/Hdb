#include "StringBuilder.h"
#define DEFAULT_CAPACITY 100

static void reExpand(StringBuilder* strBuilder){
	
}

PStringBuilder newStringBuilder(int capacity){
	StringBuilder* strBuilder=malloc(sizeof(StringBuilder));
	if(strBuilder==NULL){
		return NULL;
	}
	if(capacity==0)
		strBuilder->capacity=capacity;
	
	return strBuilder;
}

void appendStringBuilder(const StringBuilder* strBuilder,const char* value){
	if(strBuilder==NULL)
		return;
	
}

void insertStringBuilder(StringBuilder* strBuilder,int index,char* value){

}

void freeStringBuilder(StringBuilder* strBuilder){
	
}