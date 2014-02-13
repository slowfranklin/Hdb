#include "Hdb.h"
#include "const.h"
#include <unistd.h>

#define DEFAULT_TRACE_PATH 		"/trace"
#define DEFAULT_JOURNAL_PATH	"/journal"
#define MAX_CONFIG_KEY (30)

Config defaultConfig={DEFAULT_TRACE_PATH,DEFAULT_JOURNAL_PATH };
extern char pageBuffer[DEFAULT_PAGE_SIZE];
extern int buildHdb(Hdb* pdb);
extern int buildHdb1(Hdb* pdb);
Config readConfig(const char* filepath){
	Config config={'\0'};
	if(filepath && access(filepath,F_OK)!=0){
		/*
		char configKey[MAX_CONFIG_KEY]={'\0'};
		FILE* configFile = fopen(filepath,"r");
		if(!configFile){
			//traceWrite(&hdb->trace,TRACE_ERROR,__FILE__,__LINE__,"");
			//goto DEFAULT;
		}
		while(true){
			char* indexChar=NULL;
			if(fgets(pageBuffer,DEFAULT_PAGE_SIZE,configFile) == EOF){
				//traceWrite(&hdb->trace,TRACE_ERROR,__FILE__,__LINE__,"");
				break;
			}
			indexChar = strchr(pageBuffer,'=');
		}
		*/
	}
	else{
		char *tracepath = malloc(sizeof(char) * PATH_MAX);
		char *journalpath=malloc(sizeof(char) * PATH_MAX);
		getcwd(tracepath,PATH_MAX);
		getcwd(journalpath,PATH_MAX);
		config.tracepath=tracepath;
		config.journalpath=journalpath;
	}
	return config;
}

int compareKey(const char* key1,int key1Size,const char* key2,int key2Size){
	int a,compareResult=0;
	int minKeySize = min(key1Size,key2Size);
	for(a=0;a < minKeySize;a++){
		if(key1[a] > key2[a]){
			compareResult = 1;
			break;
		}
		else if(key1[a] < key2[a]){
			compareResult = -1;
			break;
		}
	}
	
	if(compareResult == 0){
		if(key1Size > key2Size)
			compareResult = 1;
		else if(key1Size < key2Size)
			compareResult = -1;
	}
	return compareResult;
}

int recoverHdb(Hdb* pdb){
	return buildHdb1(pdb);
}
