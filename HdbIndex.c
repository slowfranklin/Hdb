#include "Hdb.h"
#include "util.h"
#include <assert.h>
#include <unistd.h>
#include <math.h>

extern char pageBuffer[DEFAULT_PAGE_SIZE]; 
int buildHdb(Hdb* pdb){
	int offset=0,a=0;
	if(lseek(pdb->index,0,SEEK_SET) == -1){
		traceWrite(&pdb->trace,TRACE_ERROR,__FILE__,__LINE__,"writeIndex");
		return 0;
	}
	
	if(read(pdb->index,pageBuffer,DEFAULT_PAGE_SIZE) == -1){
		traceWrite(&pdb->trace,TRACE_ERROR,__FILE__,__LINE__,"");
		return 0;
	}
	//read pageCount
	pdb->pageCount = getU32(pageBuffer,offset);
	offset += 4;
	//read dirDepth
	pdb->dirDepth = geti32(pageBuffer,offset);
	offset += 4;
	for(a=0;a<32;a++){
		pdb->depthCount[a] = geti32(pageBuffer,offset);
		offset += 4;
	}
	//read bucket
	pdb->bucket =(PageNo*)malloc(sizeof(PageNo) * pow(2,pdb->dirDepth));
	if(!pdb->bucket)
		return 0;
	for(a=0;a<pow(2,pdb->dirDepth);a++){
		pdb->bucket[a]=getU32(pageBuffer,offset);
		offset += 4;
	}
	return 1;
}

int buildHdb1(Hdb* pdb){
	int offset=0,a=0;
	char smallBuffer[4]={'\0'};
	if(lseek(pdb->index,0,SEEK_SET) == -1){
		traceWrite(&pdb->trace,TRACE_ERROR,__FILE__,__LINE__,"writeIndex");
		return 0;
	}
	read(pdb->index,smallBuffer,4);
	//read pageCount
	pdb->pageCount = getU32(smallBuffer,0);
	offset += 4;
	lseek(pdb->index,offset,SEEK_SET);
	//read dirDepth
	read(pdb->index,smallBuffer,4);
	pdb->dirDepth = geti32(smallBuffer,0);
	offset += 4;
	for(a=0;a<32;a++){
		lseek(pdb->index,offset,SEEK_SET);
		read(pdb->index,smallBuffer,4);
		pdb->depthCount[a] = geti32(smallBuffer,0);
		offset += 4;
	}
	//read bucket
	pdb->bucket =(PageNo*)malloc(sizeof(PageNo) * pow(2,pdb->dirDepth));
	if(!pdb->bucket)
		return 0;
	for(a=0;a<pow(2,pdb->dirDepth);a++){
		lseek(pdb->index,offset,SEEK_SET);
		read(pdb->index,smallBuffer,4);
		pdb->bucket[a]=getU32(smallBuffer,0);
		offset += 4;
	}
	return 1;
}
int writeIndex(Hdb* pdb){
	int offset=0,a;
	assert(pdb);
	//write pageCount
	setU32(pageBuffer,offset,pdb->pageCount);
	offset += 4;
	//write dirDepth
	seti32(pageBuffer,offset,pdb->dirDepth);
	offset +=4;
	//write depthCount
	for(a=0;a<32;a++){
		seti32(pageBuffer,offset,pdb->depthCount[a]);
		offset += 4;
	}
	
	if(lseek(pdb->index,0,SEEK_SET) == -1){
		traceWrite(&pdb->trace,TRACE_ERROR,__FILE__,__LINE__,"writeIndex");
		return 0;
	}
	//write bucket
	for(a=0;a<pow(2,pdb->dirDepth);a++){
		setU32(pageBuffer,offset,pdb->bucket[a]);
		
		offset += 4;
	}
	if(write(pdb->index,pageBuffer,DEFAULT_PAGE_SIZE) == -1){
		traceWrite(&pdb->trace,TRACE_ERROR,__FILE__,__LINE__,"writeIndex");
		return 0;
	}
	if(fsync(pdb->index) == -1){
		traceWrite(&pdb->trace,TRACE_ERROR,__FILE__,__LINE__,"writeIndex");
		return 0;
	}
	return 1;
}

int writeIndex2(Hdb* pdb){
	int a;
	char smallBuffer[4]={'\0'};
	//write pageCount
	setU32(smallBuffer,0,pdb->pageCount);
	write(pdb->index,smallBuffer,4);
	//write dirDepth
	seti32(smallBuffer,0,pdb->dirDepth);
	write(pdb->index,smallBuffer,4);
	//write depthCount
	for(a=0;a<32;a++){
		seti32(smallBuffer,0,pdb->depthCount[a]);
		write(pdb->index,smallBuffer,4);
	}
	//write bucket
	for(a=0;a<pow(2,pdb->dirDepth);a++){
		setU32(smallBuffer,0,pdb->bucket[a]);
		write(pdb->index,smallBuffer,4);
	}
	
	if(fsync(pdb->index) == -1){
		traceWrite(&pdb->trace,TRACE_ERROR,__FILE__,__LINE__,"writeIndex");
		return 0;
	}
	return 1;
}