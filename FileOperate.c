#include "FileOperate.h"
#include "const.h"
#include <assert.h>
#include <string.h>

int checkDir(const char* filepath){
	int i,len;
	char dirname[PATH_MAX + 2];
	assert(filepath);
	strcpy(dirname,filepath);
	len=strlen(filepath);
	
	if(dirname[len-1] != '/')
		strcat(dirname,"/");
	for(i=0;i<len;i++){
		
	}
	return 1;
}
