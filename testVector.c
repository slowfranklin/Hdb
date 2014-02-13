#include "Vector.h"
#include <stdio.h>
#include <mcheck.h>

#define LENLEN  10
int main(){
	Vector vector;
	int i;
	initVector(&vector,sizeof(int),0);
	for(i=0;i<LENLEN;i++){
		addVector(&vector,&i);
	}
	for(i=0;i<vector.size;i++){
		int a=*(int*)getVectorByIndex(&vector,i);
		if(a==0 ||a==5 || a==6 || a== 9){
			removeVectorByIndex(&vector,i);
			i--;
		}
		//printf("%d\n",a);
	}
//	removeVectorByIndex(&vector,5);
	
	for(i=0;i<vector.size;i++){
		int a=*(int*)getVectorByIndex(&vector,i);
		printf("%d\n",a);
	}
	for(i=0;i<vector.size;i++){
		removeVectorByIndex(&vector,i);
	}
	freeVector(&vector,false);
	return 1;
}
