#include "PageCache.h"
#include "CommonType.h"
#include "Hashtable.h"
#include "Hdb.h"
#include "DList.h"
#include "util.h"
#include "const.h"
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <malloc.h>
#include <sys/types.h>
#include <unistd.h>
extern char pageBuffer[DEFAULT_PAGE_SIZE]; 
PageCache* newPageCache(Hdb* hdb){
	PageCache* pageCache=NULL;
	pageCache=(PageCache*)malloc(sizeof(PageCache));
	if(pageCache){
		pageCache->pageFinder=newHashtable(HASH_TYPE_NUMBER,0,0);
		pageCache->pageStore=newDList();
		pageCache->pdb=hdb;
	}
	return pageCache;
}

PPage getPage(PageCache* pageCache,PageNo pageNo){
	HashKey hashKey;
	void* nodeAddress;
	assert(pageCache);
	hashKey.i=pageNo;
	nodeAddress = getHashtable(pageCache->pageFinder,hashKey);
	if(nodeAddress){
		void* data;
		DListNode* listnode =(DListNode*)nodeAddress;
		data=listnode->data;
		moveDListNodeToTail(pageCache->pageStore,listnode);
		assert(pageNo == ((PPage)data)->pageNo);
		return data;
	}
	else if(pageNo < pageCache->pdb->pageCount){
		memset(pageBuffer,'\0',DEFAULT_PAGE_SIZE);
		if(lseek(pageCache->pdb->fd,DEFAULT_PAGE_SIZE * pageNo,SEEK_SET) == -1){
			traceWrite(&pageCache->pdb->trace,TRACE_ERROR,__FILE__,__LINE__,"");
			return NULL;
		}
		if(read(pageCache->pdb->fd,pageBuffer,DEFAULT_PAGE_SIZE) == -1){
			traceWrite(&pageCache->pdb->trace,TRACE_ERROR,__FILE__,__LINE__,"");
			return NULL;
		}
		
		PPage page =(PPage)malloc(PAGE_STRUCT_SIZE);
		if(!page) return NULL;
		memset(page,'\0',PAGE_STRUCT_SIZE);
		if(!parsePage(page,pageBuffer,0)){
			free(page);
			return NULL;
		}
		assert(page->pageNo == pageNo);
		if(!addPage(pageCache,page)){
			free(page);
			return NULL;
		}
		page->dirty=false;
		page->reCalculate=true;
		return page;
	}
	return NULL;
}

int addPage(PageCache* pageCache,PPage page){
	int rv;//return value
	void* pageAddress=NULL;
	HashKey hashKey;
	hashKey.i=page->pageNo;
	assert(pageCache);
	assert(page);
	if(getHashtable(pageCache->pageFinder,hashKey))
		return 1;
		
	pageAddress=addAndFecthTailDList(pageCache->pageStore,page);
	if(!pageAddress)
		return 0;
	
	rv=insertHashtable(pageCache->pageFinder,hashKey,pageAddress);
	if(!rv){
		removeTailDList(pageCache->pageStore);
		return 0;
	}
	return 1;
}
/*
int removePage(PageCache* pageCache,PageNo pageNo){
	HashKey hashKey;
	assert(pageCache);
	if(pageCache){
		void* nodeAddress;
		void* pageData;
		DListNode* listnode;
		hashKey.i=pageNo;
		nodeAddress = getHashtable(pageCache->pageFinder,hashKey);
		listnode=(DListNode*)nodeAddress;
		pageData=removeDListNode(pageCache->pageStore,listnode);
		removeHashtable(pageCache->pageFinder,hashKey);
		freePage((PPage)pageData);
	}
	return 1;
}
*/
PPage newPage(PDB pdb,PageType pageType){
	PPage page=NULL;
	assert(pdb);
	if(pdb->pageCount < INT32_MAX){
		while(pdb->pageCache->pageStore->size > PAGE_CACHE_SIZE){//have question about the word "while",if replace "if",then error throw
			DListNode* listnode;
			listnode=(DListNode*)fetchHeadNodeDList(pdb->pageCache->pageStore);
			page=(PPage)listnode->data;
			assert(page != NULL);
			//printf("write page=%d\n",page->pageNo);
			writePage(pdb,page);
			{
				HashKey hashKey;
				hashKey.i=page->pageNo;
				removeHashtable(pdb->pageCache->pageFinder,hashKey);
				removeHeadDList(pdb->pageCache->pageStore);
				freePage(page);
			}
			
			page=NULL;
		}
		page =(PPage)malloc(PAGE_STRUCT_SIZE);
		if(page){
			memset(page,'\0',PAGE_STRUCT_SIZE);
			page->pageNo=pdb->pageCount++;
			page->pageType=pageType;
			page->dirty=false;
			if(pageType == PT_DATA){
				DataPage* dataPage=(DataPage*)page;
				initVector(&dataPage->rows,sizeof(DataRow),0);
			}
		}
	}
	return page;
}

int writePage(PDB pdb,PPage page){
	assert(page);
	//if(page->dirty == true){
		memset(pageBuffer,'\0',DEFAULT_PAGE_SIZE);
		pageWriteToBuffer(page,pageBuffer,0);
		/*
		{
			uint32_t xsum=getU32(pageBuffer,6);
			setU32(pageBuffer,6,0x00000000);
			assert(crc32(pageBuffer,DEFAULT_PAGE_SIZE) == xsum);
		}
		*/
		if(lseek(pdb->fd,page->pageNo * DEFAULT_PAGE_SIZE,SEEK_SET) == -1){//has error
			traceWrite(&pdb->trace,TRACE_ERROR,__FILE__,__LINE__,"");
			return 0;
		}
		if(write(pdb->fd,pageBuffer,DEFAULT_PAGE_SIZE) < 0){
			traceWrite(&pdb->trace,TRACE_ERROR,__FILE__,__LINE__,"");
			return 0;
		}
		
		if(fsync(pdb->fd) == -1){
			traceWrite(&pdb->trace,TRACE_ERROR,__FILE__,__LINE__,"");
			return 0;
		}
		
		page->dirty=0;//need be think about
	//}
	return 1;
}

int freePageCache(PageCache* pageCache){
	if(pageCache){
		DListNode* listNode=NULL;
		listNode=fetchHeadNodeDList(pageCache->pageStore);
		while(listNode){
			DListNode* nextNode;
			nextNode=listNode->next;
			freePage((PPage)listNode->data);
			listNode=nextNode;
		}
		
		if(pageCache->pageFinder)
			freeHashtable(pageCache->pageFinder,true);
		if(pageCache->pageStore)
			freeDList(pageCache->pageStore,true);
		free(pageCache);
	}
	return 1;
}
