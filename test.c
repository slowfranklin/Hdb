#include "Hdb.h"
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

char* DATA="abcdefghigklmnopqrstuvwxyzABCDEFGHIGKLMNOPQRSTUVWXYZ1234567890~!@#$%^&*()_+";
#define KEY_FILE "/home/weijunbao/Db/keyfile.txt"
char* randomKey1(int valueSize){
	int i;
	char* v=malloc(sizeof(char) * (valueSize+1));
	for(i=0;i<valueSize;i++){
		int randNumber = rand() % (strlen(DATA)-1);
		v[i]=DATA[randNumber];
	}
	v[valueSize]='\0';
	return v;
}
#define LEN 10000
#define KEYLEN 30
void addTest(){
	int i=0;
	int failcount=0;
	int total=0;
	char* keys[LEN];
	int valueSize;
	srand((unsigned)time(NULL));
	Hdb* hdb=openHdb(NULL);
	
	//write
	for(;i<LEN;i++){
		char* key=randomKey1(KEYLEN);
	//	printf("%s\n",key);
		while(true){
			valueSize=rand() % 3000;
			if(valueSize >0)
				break;
		}
		
		keys[i]=key;
		char* value=randomKey1(valueSize);
		total++;
		insertHdb(hdb,key,KEYLEN,value,valueSize);
		if(i % 1000 ==0)
			printf("i=%d\n",i);
		free(value);
	}
	commitHdb(hdb);
	for(i=0;i<LEN;i++){
		char* value;
		char* key=keys[i];
		//printf("%s\n",key);
		value=searchHdb(hdb,key,KEYLEN);
		if(value){
			//printf("index=%d,key=%s,value=%s\n",i,key,value);
		}
		else{
			failcount++;
			printf("key=%s,value=NULL\n",key);
		}
	}
	/*
	for(i=0;i<LEN;i++){
		if(keys[i])
			free(keys[i]);
	}
	*/
	closeHdb(hdb);
	printf("failcount=%d\n",failcount);
	
	hdb=openHdb(NULL);
	printf("open again\n");
	//scanf("%d",&i);
	for(i=0;i<LEN;i++){
		char* value;
		char* key=keys[i];
		value=searchHdb(hdb,key,KEYLEN);
		if(value){
			printf("index=%d,key=%s,value=%s\n",i,key,value);
		}
		else{
			failcount++;
			printf("key=%s,value=NULL\n",key);
		}
	}
	printf("failcount=%d\n",failcount);
	for(i=0;i<LEN;i++){
		if(keys[i])
			free(keys[i]);
	}
	closeHdb(hdb);
}
void gettime (struct timespec *a)
{
	struct timeval tv;
	gettimeofday(&tv, 0);
	a->tv_sec  = tv.tv_sec;
	a->tv_nsec = tv.tv_usec * 1000LL;
}

struct timespec diff_timespec(struct timespec start, struct timespec end)
{
	struct timespec result;

	if (end.tv_nsec < start.tv_nsec) {
		result.tv_nsec = 1000000000 + end.tv_nsec -
			start.tv_nsec;
		result.tv_sec = end.tv_sec - 1 -
			start.tv_sec;
	}
	else {
		result.tv_nsec = end.tv_nsec -
			start.tv_nsec;
		result.tv_sec = end.tv_sec -
			start.tv_sec;
	}

	return result;
}

static inline long long millisec_elapsed(struct timespec diff)
{
	return ((long long)diff.tv_sec * 1000) + (diff.tv_nsec / 1000000);
}

static inline long long microsec_elapsed(struct timespec diff)
{
	return ((long long)diff.tv_sec * 1000000) + (diff.tv_nsec / 1000);
}

static inline long long nanosec_elapsed(struct timespec diff)
{
	return ((long long)diff.tv_sec * 1000000000) + diff.tv_nsec;
}

long long time_diff_ms(struct timespec start, struct timespec end)
{
	return millisec_elapsed(diff_timespec(start, end));
}

int main(){
	long long cost_ms;
	struct timespec start;
	struct timespec end;
	gettime(&start);
	addTest();
	gettime(&end);
	cost_ms=time_diff_ms(start,end)+1;
	printf("cost:%d(ms)",(int)cost_ms);
	printf("ok\n\n");
	return 1;
}
