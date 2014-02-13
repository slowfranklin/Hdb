#include "Hdb.h"
#include "const.h"
#include "Page.h"
#include "PageCache.h"
#include "murmur_hash.h"
#include "Trace.h"
#include "Journal.h"
#include "util.h"
#include <stdio.h>
#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <execinfo.h>
#include <fcntl.h>

#define CREATE_FLAGS (O_RDWR | O_TRUNC | O_CREAT | O_LARGEFILE | O_NONBLOCK)
#define CREATE_APPEND_FLAGS (O_WRONLY | O_CREAT | O_APPEND)
#define OPEN_FLAGS   (O_RDWR | O_LARGEFILE)
char buffer[4096]={'\0'};
extern int recoverHdb(Hdb* pdb);
extern Config readConfig(const char*);
extern int compareKey(const char* key1,int key1Size,const char* key2,int key2Size);
extern int writeIndex(Hdb* pdb);
extern int writeIndex2(Hdb* pdb);

enum HdbSyncType{
	HST_THREAD,
	HST_ONE_RECORD
};
PDB newHdb(void){
	return NULL;
}
PDB openHdb(const char* filepath){
	int state;
	size_t size=sizeof(Hdb);
	Hdb* hdb = (Hdb*)malloc(size);
	if(!hdb){
		fprintf(stderr,"FILE:%s,FUNCTION:%s,LINE:%d,ERROR:%s\n",
			__FILE__,__FUNCTION__,__LINE__,strerror(errno));
		return NULL;
	}
	memset(hdb,'\0',sizeof(Hdb));
	hdb->config=readConfig(NULL);
	if(!filepath)
		strcpy(hdb->dbPath,"/home/weijunbao/Db/hdb.db");
	hdb->lsn=0;
	hdb->strategy=newStrategy(NULL,NULL);
	hdb->pageCache=newPageCache(hdb);
	hdb->freePages=newLinklist();
	hdb->compareKeyFunc=compareKey;
	{
		state=initTrace(&hdb->trace,hdb->config.tracepath);
		if(!state)
			goto ERROR;
		state=initJournal(&hdb->journal,hdb->config.journalpath);
		if(!state)
			goto ERROR;
	}
	if(access(hdb->dbPath,F_OK)!=0){
		PPage page1,page2;
		//no exist file
		hdb->fd = open(hdb->dbPath,CREATE_FLAGS,0644);
		if(hdb->fd == -1) {
			traceWrite(&hdb->trace,TRACE_ERROR,__FILE__,__LINE__,"");
			goto ERROR;
		}
		hdb->index = open("/home/weijunbao/Db/hdb.index",CREATE_APPEND_FLAGS,0644);
		if(hdb->index == -1){
			traceWrite(&hdb->trace,TRACE_ERROR,__FILE__,__LINE__,"");
			goto ERROR;
		}
		hdb->pageCount=0;
		hdb->dirDepth = 1;
		hdb->bucket =(PageNo*)malloc(sizeof(PageNo) * pow(2,hdb->dirDepth));
		if(!hdb->bucket)
			goto ERROR;
		memset(hdb->bucket,0,sizeof(PageNo) * pow(2,hdb->dirDepth));
		hdb->bucket[0]=0;
		hdb->bucket[1]=1;
		
		memset(hdb->depthCount,0,sizeof(hdb->depthCount));
		hdb->depthCount[1] = pow(2,hdb->dirDepth);
		
		page1 = newPage(hdb,PT_DATA);
		if(!page1) 
			goto ERROR;
		page2 = newPage(hdb,PT_DATA);
		if(!page2) 
			goto ERROR;
		
		addPage(hdb->pageCache,page1);
	 	traceWrite(&hdb->trace,TRACE_INFO,__FILE__,__LINE__,"");
		addPage(hdb->pageCache,page2);
		page1->pgDepth = page2->pgDepth = 1;
	}
	else{
		hdb->fd = open(hdb->dbPath,OPEN_FLAGS);
		if(hdb->fd == -1){
			traceWrite(&hdb->trace,TRACE_ERROR,__FILE__,__LINE__,"");
			goto ERROR;
		}
		hdb->index = open("/home/weijunbao/Db/hdb.index",OPEN_FLAGS);
		if(hdb->index == -1){
			traceWrite(&hdb->trace,TRACE_ERROR,__FILE__,__LINE__,"");
			goto ERROR;
		}
		state=recoverHdb(hdb);
		if(state == 0)
			goto ERROR;
	}
	return hdb;
ERROR:
	if(hdb){
		if(hdb->bucket)
			free(hdb->bucket);
		if(hdb->freePages)
			freeLinklist(hdb->freePages);
		if(hdb->pageCache)
			freePageCache(hdb->pageCache);
		if(hdb->strategy)
			free(hdb->strategy);
		free(hdb);
	}
	return NULL;
}

int insertHdb(PDB pDb,const void* pKey,int keySize,const void* pValue,int valueSize){
	PageNo bucketIndex,pageNo;
	uint32_t hashcode,result;
	PPage page=NULL;
	assert(pKey);
	hashcode = _hash((char*)pKey,keySize);
	bucketIndex = hashcode & ((0x00000001 << pDb->dirDepth) - 1);
	pageNo = pDb->bucket[bucketIndex];
	page = getPage(pDb->pageCache,pageNo);
	assert(page !=NULL);
	assert(page->pageType == PT_DATA);
	assert(page->pageNo == pageNo);
	result = addRecordToPage(pDb,(DataPage*)page,pKey,keySize,pValue,valueSize);
	{
		JournalRecord journalRecord;
		journalRecord.journalType = JT_INSERT;
		journalRecord.key=(char*)pKey; journalRecord.keySize=keySize;
		journalRecord.value=(char*)pValue;journalRecord.valueSize=valueSize;
		writeJournal(&pDb->journal,journalRecord);
	}
	
	if(result == POT_SPLIT){
		//writeJournal JT_SPLIT
		return splitDataPage(pDb,(DataPage*)page,bucketIndex);
	}
	return 1;	
}

int deleteHdb(PDB pDb,const void* pKey,int keySize){
	PageNo bucketIndex,pageNo;
	uint32_t hashcode,a;
	PPage page=NULL;
	DataPage* dataPage=NULL;
	hashcode = _hash((char*)pKey,keySize);
	bucketIndex = hashcode & ((1 << pDb->dirDepth) - 1);
	pageNo = pDb->bucket[bucketIndex];
	page = getPage(pDb->pageCache,pageNo);
	assert(page !=NULL);
	assert(page->pageType == PT_DATA);
	{
		dataPage = (DataPage*)page;
		for(a=0;a<dataPage->rows.size;a++){
			DataRow* dataRow = getVectorByIndex(&dataPage->rows,a);
			if(pDb->compareKeyFunc(pKey,keySize,dataRow->key,dataRow->keySize) == 0){
				removeVectorByIndex(&dataPage->rows,a);
				break;
			}
		}
	}
	{
		JournalRecord journalRecord;
		journalRecord.journalType = JT_DELETE;
		journalRecord.key=(char*)pKey; journalRecord.keySize=keySize;
		writeJournal(&pDb->journal,journalRecord);
	}
	if(pDb->dirDepth == 1)
		return 1;
	{
		bool ismerge=false;
		DataPage* dataPage,*brother_dataPage;
		PPage brother_page=NULL;
		PageNo brother_bucketIndex = (0x00000001 <<(pDb->dirDepth - 1)) ^ bucketIndex;
		PageNo brother_pageNo = pDb->bucket[brother_bucketIndex];
		brother_page = getPage(pDb->pageCache,brother_pageNo);
		
		assert(brother_page != NULL);
		assert(brother_page->pageType == PT_DATA);
		dataPage =(DataPage*)page;
		brother_dataPage=(DataPage*)brother_page;
		if(dataPage->rows.size ==0)
			ismerge = true;
		else if(pageNo == brother_pageNo){
			ismerge = false;
		}
		else{
			ismerge = pDb->strategy->mergeStrategy(pDb,page,brother_page);
		}
		if(ismerge == true){
			//need be merged
			assert(page->pgDepth == brother_page->pgDepth);
			if(pageNo == brother_pageNo){
				//回收此页,并更新涉及到的 buckets
				
			}
			else{
				int depth;
				//merge,from big to small
				if(pageNo > brother_pageNo){
					for(a=0;a<dataPage->rows.size;a++){
						DataRow* dataRow = (DataRow*)getVectorByIndex(&dataPage->rows,a);
						addVector(&brother_dataPage->rows,dataRow);
					}
					brother_dataPage->dirty = true;
					assert(getDataPageFreeSpace(brother_dataPage) >= 0);
					brother_dataPage->pgDepth -=1;
					assert(brother_dataPage->pgDepth >= 1);
					depth = brother_page->pgDepth;
					pDb->bucket[bucketIndex] = pDb->bucket[brother_bucketIndex];
					//回收 dataPage 页
					{
						void* pageNoBuffer=NULL;
						setU32((char*)&pageNo,0,pageNo);
						pushLinklist(pDb->freePages,pageNoBuffer);
						//writeJournal JT_RECYCLE_PAGE
					}
				}
				else{
					for(a=0;a<brother_dataPage->rows.size;a++){
						DataRow* dataRow=(DataRow*)getVectorByIndex(&brother_dataPage->rows,a);
						addVector(&dataPage->rows,dataRow);
					}
					dataPage->dirty = true;
					assert(getDataPageFreeSpace(dataPage) >= 0);
					dataPage->pgDepth -= 1;
					assert(dataPage->pgDepth >= 1);
					depth = page->pgDepth;
					pDb->bucket[brother_bucketIndex] = pDb->bucket[bucketIndex];
					//回收 btother_pageNo 页
					{
						void* brother_pageNoBuffer=NULL;
						setU32((char*)&brother_pageNoBuffer,0,brother_pageNo);
						pushLinklist(pDb->freePages,brother_pageNoBuffer);
						//writeJournal JT_RECYCLE_PAGE
					}
				}
				pDb->depthCount[depth + 1] -= 2;
				pDb->depthCount[depth] += 1;
			}
		}
	}
	return 1;
}

void* searchHdb(PDB pdb,const void* pKey,int keySize){
	PageNo bucketIndex,pageNo;
	void* value=NULL;
	uint32_t hashcode,a;
	PPage page;
	hashcode = _hash((char*)pKey,keySize);
	bucketIndex =hashcode & ((1 << pdb->dirDepth) - 1);
	pageNo = pdb->bucket[bucketIndex];
	assert(pageNo < pdb->pageCount);
	page = getPage(pdb->pageCache,pageNo);
	assert(page != NULL);
	assert(page->pageType == PT_DATA);
	{
		DataPage* dataPage=(DataPage*)page;
		for(a=0;a < dataPage->rows.size;a++){
			DataRow* dataRow = getVectorByIndex(&dataPage->rows,a);
			if(pdb->compareKeyFunc(pKey,keySize,dataRow->key,dataRow->keySize) == 0){
				memset(buffer,'\0',4096);
				
				strncpy(buffer,dataRow->value,dataRow->curValueSize);
				if(dataRow->ofPageNo > 0){
					OfPage* ofPage=(OfPage*)getPage(pdb->pageCache,dataRow->ofPageNo);
					strncpy(buffer + dataRow->curValueSize,ofPage->value,ofPage->valueSize);
				}
				value=buffer;
				break;
			}
		}
	}
	
	return value;
}

int commitHdb(PDB pdb){
	DListNode* listNode=NULL;
	assert(pdb);
	listNode=(DListNode*)fetchHeadNodeDList(pdb->pageCache->pageStore);
	while(listNode){
		writePage(pdb,(PPage)listNode->data);
		listNode = listNode->next;
	}
	
	if(!writeIndex2(pdb)){
		return 0;
	}
	return 1;
}

int verifyHdb(PDB pdb){
	unsigned long size;
	assert(pdb);
	return 1;
}


int closeHdb(PDB pdb){
	assert(pdb);
	if(pdb){
		close(pdb->fd);
		close(pdb->index);
		if(pdb->strategy)
			free(pdb->strategy);
		if(pdb->freePages)
			freeLinklist(pdb->freePages);
		if(pdb->bucket)
			free(pdb->bucket);
		if(pdb->pageCache)
			freePageCache(pdb->pageCache);
		if(pdb->config.tracepath)
			free(pdb->config.tracepath);
		if(pdb->config.journalpath)
			free(pdb->config.journalpath);
		freeTrace(&pdb->trace,false);
		free(pdb);
	}
	return 1;
}
