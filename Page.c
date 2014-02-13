#include "Page.h"
#include "util.h"
#include "VCode.h"
#include "const.h"
#include "Vector.h"
#include "Trace.h"
#include "CommonType.h"
#include "murmur_hash.h"
#include "crc32.h"
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

typedef struct HdbMeta HdbMeta;
struct HdbMeta{
	time_t createTime;
	time_t lastTime;
	uint64_t records;
};

char pageBuffer[DEFAULT_PAGE_SIZE]={'\0'};
static int getDataRowSpace(DataRow dataRow){
	int space=0;
	space += dataRow.keySize;
	space +=1;
	if(dataRow.ofPageNo > 0)
		space+=4;
	if(dataRow.curValueSize > 0){
		space += dataRow.curValueSize;
	}
	space += getVSize(dataRow.valueSize);		
	return space;
}

int getOfpageFreeSpace(OfPage* ofPage){
	int freeSpace = 0;
	assert(ofPage);
	freeSpace = DEFAULT_OFPAGE_FREE_SPACE - getVSize(ofPage->valueSize) - ofPage->valueSize;
	assert(freeSpace >= 0);
	return freeSpace;
}

static int freeDataRow(DataRow* dataRow){
	if(dataRow){
		if(dataRow->key)
			free(dataRow->key);
		if(dataRow->value)
			free(dataRow->value);
	}
	return 1;
}

static int freeDataPage(DataPage* dataPage){
	int i;
	if(dataPage){
		for(i=0;i<dataPage->rows.size;i++){
			DataRow* dataRow = (DataRow*)getVectorByIndex(&dataPage->rows,i);
			freeDataRow(dataRow);
		}
		freeVector(&dataPage->rows,false);
		free(dataPage);
	}
	return 1;
}

static int freeOfPage(OfPage* ofPage){
	if(ofPage){
		free(ofPage->value);
		free(ofPage);
	}
	return 1;
}

int freePage(PPage page){
	assert(page);
	assert(page->pageType == PT_DATA || page->pageType == PT_OF);
	if(page->pageType == PT_DATA)
		return freeDataPage((DataPage*)page);
	else if(page->pageType == PT_OF)
		return freeOfPage((OfPage*)page);
	return 1;
}

int getDataPageFreeSpace(DataPage* dataPage){
	int usedSpace=0,i=0;
	if(dataPage->rows.size == 0){
		return DEFAULT_PAGE_FREE_SPACE;
	}
	else if(dataPage->reCalculate){
		for(;i < dataPage->rows.size;i++){
			DataRow dataRow = *(DataRow*)getVectorByIndex(&dataPage->rows,i);
			usedSpace += getDataRowSpace(dataRow);
		}
		dataPage->freeSpace=DEFAULT_PAGE_FREE_SPACE - usedSpace - dataPage->rows.size * 2;
		dataPage->reCalculate=false;
	}
	//assert(dataPage->freeSpace >= 0);
	return dataPage->freeSpace;
}

int getFreeSpace(PPage page){
	assert(page->pageType == PT_DATA || page->pageType == PT_OF);
	if(page->pageType == PT_DATA){
		return getDataPageFreeSpace((DataPage*)page);					
	}
	else if(page->pageType == PT_OF){
		return getOfpageFreeSpace((OfPage*)page);
	}
	return 1;
}

static int dataPageWriteToBuffer(DataPage* dataPage,char* pageBuffer,int startOffset,int endOffset){
	int a;
	setU16(pageBuffer,startOffset,dataPage->rows.size);//row size
	
	for(a=0;a<dataPage->rows.size;a++){
		DataRow dataRow = *(DataRow*)getVectorByIndex(&dataPage->rows,a);
		assert(dataRow.keySize <= MAX_KEY_SIZE);
		startOffset += 2;
		if(dataRow.ofPageNo != 0){
			//write ofpageno
			setU32(pageBuffer,endOffset - 4,dataRow.ofPageNo);
			endOffset -= 4;
		}
		{
			//write value
			int variableSize = getVSize(dataRow.valueSize);
			int vsize = setVCodei32(dataRow.valueSize,pageBuffer,endOffset - dataRow.curValueSize - variableSize);
			assert(variableSize == vsize);
			if(dataRow.curValueSize){
				assert(dataRow.value != NULL);
				memcpy(pageBuffer + endOffset - dataRow.curValueSize,dataRow.value,dataRow.curValueSize * sizeof(char));
			}
			endOffset = endOffset - dataRow.curValueSize - variableSize;
			assert(startOffset <= endOffset);
		}
		{
			//write key
			memcpy(pageBuffer + endOffset - dataRow.keySize,dataRow.key,dataRow.keySize * sizeof(char));
			endOffset -= dataRow.keySize;
			pageBuffer[endOffset -1] = (unsigned char)dataRow.keySize;
			endOffset -= 1;
			assert(startOffset <= endOffset);
		}
		setU16(pageBuffer,startOffset,(uint16_t)endOffset);
		//startOffset += 2;
	}
	assert(startOffset <= endOffset);
	return 1;
}

static int ofPageWriteToBuffer(OfPage* ofPage,char* pageBuffer,int startOffset,int endOffset){
	int variableSize;
	setU32(pageBuffer,endOffset - 4,ofPage->nextPageNo);
	endOffset -= 4;
	variableSize = setVCodei16(ofPage->valueSize,pageBuffer,startOffset);
	startOffset += variableSize;
	memcpy(pageBuffer + startOffset,ofPage->value,ofPage->valueSize * sizeof(char));
	return 1;
}

int pageWriteToBuffer(PPage page,char* pageBuffer,int offset){
	int state;
	int endOffset,crc32Offset;
	uint32_t xsum=0;
	
	assert(page);
	assert(page->pageType == PT_DATA || page->pageType == PT_OF);
	assert(pageBuffer);
	endOffset = offset + DEFAULT_PAGE_SIZE;
	setU32(pageBuffer,offset,page->pageNo);//pageno
	offset+=4;
	pageBuffer[offset]=(char)page->pageType;//pagetype
	offset+=1;
	pageBuffer[offset]=(char)page->pgDepth;//pgDepth
	offset+=1;
	
	crc32Offset=offset;
	setU32(pageBuffer,offset,0x00000000);
	offset+=4;//reserced for crc32
	
	if(page->pageType == PT_DATA){
		state = dataPageWriteToBuffer((DataPage*)page,pageBuffer,offset,endOffset);	
	}
	else if(page->pageType == PT_OF){
		state = ofPageWriteToBuffer((OfPage*)page,pageBuffer,offset,endOffset);
	}
	xsum=crc32(pageBuffer,DEFAULT_PAGE_SIZE);
	setU32(pageBuffer,crc32Offset,xsum);
	return state;
}

static int parseDataRow(DataRow* dataRow,const char* pageBuffer,int startOffset,int endOffset){
	int keySize,valueSizeInPage;
	char* pKey,*pValue=NULL;
	uint64_t valueSize;//includ ofpage.value
	PageNo ofpageno=0;
	keySize = (u8)pageBuffer[startOffset];
	startOffset += 1;
	assert(keySize > 0);
	pKey = (char*)malloc(sizeof(char) *(keySize + 1));
	if(!pKey){
		goto PARSE_ERROR;
	}
	else{
		int vsize;//variable code size
		memcpy(pKey,pageBuffer+startOffset,(size_t)keySize * sizeof(char));
		pKey[keySize] = '\0';
		startOffset += keySize;
		assert(startOffset < endOffset);
		vsize = getVCode(pageBuffer,startOffset,&valueSize);
		startOffset += vsize;
		assert(startOffset < endOffset);
		valueSizeInPage = endOffset - startOffset;
		if(valueSize > valueSizeInPage){
		//have overlfow
			valueSizeInPage -= 4;
			if(valueSizeInPage == 0){
				assert(startOffset = endOffset - 4);
			}
			ofpageno = getU32(pageBuffer,endOffset - 4);
			endOffset -= 4;
		}
		if(valueSizeInPage > 0){
			pValue = (char*)malloc(sizeof(char) * (valueSizeInPage + 1));
			if(!pValue){
				goto PARSE_ERROR;	
			}
			else{
				memcpy(pValue,pageBuffer + startOffset,valueSizeInPage * sizeof(char));
				pValue[valueSizeInPage]='\0';
			}
		}
		assert(startOffset + valueSizeInPage == endOffset);
	}
	
	dataRow->key = pKey;
	dataRow->keySize = keySize;
	dataRow->value = pValue;
	dataRow->valueSize = valueSize;
	dataRow->curValueSize = valueSizeInPage;
	dataRow->ofPageNo = ofpageno;
	return PFE_OK;
PARSE_ERROR:
	if(pKey)
		free(pKey);
	if(pValue)
		free(pValue);
	return PTE_ROW_ERROR;
}

static int parseDataPage(DataPage* dataPage,const char* pageBuffer,int startOffset,int endOffset){
	int a,rows,parseRowResult;
	rows = getU16(pageBuffer,startOffset);
	startOffset += 2;
	//assert(rows > 0);
	if(rows >0){
		initVector(&dataPage->rows,sizeof(DataRow),0);
		{
			int /*trueFreeSpace,*/rowStartOffset=0,rowEndOffset=0;
			DataRow dataRow = {0};
			rowStartOffset = getU16(pageBuffer, startOffset);
			rowEndOffset = endOffset;
			for(a=0;a<rows;a++){
				assert(rowStartOffset < rowEndOffset);
				assert(startOffset <= rowStartOffset);
				parseRowResult = parseDataRow(&dataRow,pageBuffer,rowStartOffset,rowEndOffset);	
				if(parseRowResult == PFE_OK){
					addVector(&dataPage->rows,&dataRow);
				}
				else{
					goto PARSE_ERROR;
				}
				startOffset += 2;
				rowEndOffset = rowStartOffset;
				rowStartOffset = getU16(pageBuffer, startOffset);
			}
		}
	}
	else{
		initVector(&dataPage->rows,sizeof(DataRow),0);//??? 需要初始化吗
	}
	return 1;
PARSE_ERROR:
	if(dataPage->rows.size > 0){
		for(a=0;a < dataPage->rows.size; a++){
			DataRow* dataRow = (DataRow*)getVectorByIndex(&dataPage->rows,a);
			if(dataRow->key)
				free(dataRow->key);
			if(dataRow->value)
				free(dataRow->value);
		}
		initVector(&dataPage->rows,sizeof(DataRow),0);//free rows.data
	}
	return 0;
}

static int parseOfPage(OfPage* ofPage,const char* pageBuffer,int startOffset,int endOffset){
	uint64_t vcode;
	int vsize;
	char* pValue=NULL;
	{//parse ofpage
		ofPage->nextPageNo = getU32(pageBuffer,endOffset - 4);
		endOffset -= 4;
	}
	{
		vsize = getVCode(pageBuffer,startOffset,&vcode);
		startOffset += vsize;
		assert(vcode >0);
		assert(startOffset + vcode <= endOffset);
		if(vcode >0){
			pValue =(char*)malloc(vcode + 1);
			if(!pValue)
				goto PARSE_ERROR;
			memcpy(pValue,pageBuffer+ startOffset,vcode * sizeof(char));
			pValue[vcode]='\0';
		}	
	}
	ofPage->value=pValue;
	ofPage->valueSize=vsize;
	return 1;
PARSE_ERROR:
	if(pValue)
		free(pValue);
	return 0;
}

int parsePage(Page* page,char* pageBuffer,int offset){
	int endOffset = offset + DEFAULT_PAGE_SIZE;
	uint32_t xsum=0;
	assert(page);
	assert(pageBuffer);
	
	xsum=getU32(pageBuffer,6);
	setU32(pageBuffer,6,0x00000000);
	
	if(crc32(pageBuffer,DEFAULT_PAGE_SIZE) != xsum){
		
		return 0;
	}
	
	page->pageNo = getU32(pageBuffer,offset);
	offset += 4;
	page->pageType=(char)pageBuffer[offset];
	offset += 1;
	page->pgDepth=(char)pageBuffer[offset];
	offset +=1;
	offset +=4;//skip crc32
	
	assert(page->pageType == PT_DATA || page->pageType == PT_OF);
	if(page->pageType == PT_DATA){
		return parseDataPage((DataPage*)page,pageBuffer,offset,endOffset);
	}
	else if(page->pageType == PT_OF){
		return parseOfPage((OfPage*)page,pageBuffer,offset,endOffset);
	}
	return 0;
}
/*
static void updateBucket(PDB pdb,int pgDepth,BucketIndex bucketIndex,PageNo pageNo,PageNo brother_pageNo){
	BucketIndex brother_bucketIndex,minBucketIndex;
	BucketIndex diff;
	int assertCount=0;
	static int updateCount=0;
	brother_bucketIndex = 0x00000001 << (pgDepth-1) ^ bucketIndex;
	diff = abs(bucketIndex - brother_bucketIndex);
	minBucketIndex=min(bucketIndex,brother_bucketIndex);
	if(minBucketIndex + diff < pow(2,pdb->dirDepth))
		updateCount++;
	while(minBucketIndex + diff < pow(2,pdb->dirDepth)){
		//assert(pdb->bucket[minBucketIndex] == pdb->bucket[minBucketIndex + diff]);
		pdb->bucket[minBucketIndex]=pageNo;
		assert(minBucketIndex + diff <pow(2,pdb->dirDepth));
		minBucketIndex +=diff;
		pdb->bucket[minBucketIndex]=brother_pageNo;
		minBucketIndex += diff;
		assertCount++;
	}
	//bug
	//assert(assertCount == pdb->dirDepth - pgDepth + 1);
}
*/
int expandBucket(PDB pdb){
	PageNo* bucket=NULL;
	pdb->dirDepth++;
	bucket=(PageNo*)realloc(pdb->bucket,sizeof(PageNo) *(unsigned int)pow(2,pdb->dirDepth));
	if(!bucket)
		return 0;
	pdb->bucket=bucket;
	memcpy(pdb->bucket +(uint32_t)pow(2,pdb->dirDepth-1),pdb->bucket,(uint32_t)pow(2,pdb->dirDepth-1) * sizeof(PageNo));
	return 1;
}

int splitDataPage(PDB pdb,DataPage* dataPage,BucketIndex bucketIndex){
	int a;
	assert(pdb->bucket[bucketIndex] == dataPage->pageNo);
	if(dataPage->pgDepth == pdb->dirDepth){
		PageNo* bucket=NULL;
		pdb->dirDepth++;
		bucket = (PageNo*)realloc(pdb->bucket,sizeof(PageNo) * (unsigned int)pow(2,pdb->dirDepth));
		assert(bucket != NULL);
		if(bucket == NULL)
			return 0;
		pdb->bucket=bucket;
		memcpy(pdb->bucket +(unsigned int) pow(2,pdb->dirDepth - 1),pdb->bucket,(unsigned int)pow(2,pdb->dirDepth - 1) * sizeof(PageNo));
	}
	{
		DataPage* brother_page=NULL;
		BucketIndex brother_bucketIndex;
		//PageNo brother_pageNo;
		brother_page=(DataPage*)newPage(pdb,PT_DATA);
		if(!brother_page)
			return 0;
		if(!addPage(pdb->pageCache,(PPage)brother_page)){
			assert(1);
			return 0;
		}
		/*
		if(pdb->dirDepth - dataPage->pgDepth >= 2){
			int aa=0;
			a++;
		}
		*/
		//brother_pageNo=brother_page->pageNo;
		
		dataPage->pgDepth++;
		brother_page->pgDepth = dataPage->pgDepth;
		bucketIndex =bucketIndex & ((0x00000001 << dataPage->pgDepth) -1);
		assert(pdb->bucket[bucketIndex] == dataPage->pageNo);
		brother_bucketIndex = (0x00000001 << (dataPage->pgDepth-1)) ^ bucketIndex;
		//pdb->bucket[brother_bucketIndex]=brother_page->pageNo;
		//if(dataPage->pgDepth < pdb->dirDepth)
		//{
			//updateBucket(pdb,dataPage->pgDepth,bucketIndex,dataPage->pageNo,brother_pageNo);
		//}
		
	
		for(a=0;a<(unsigned int)pow(2,pdb->dirDepth);a++){
			if(pdb->bucket[a] == dataPage->pageNo){
				if((a &((0x00000001 << dataPage->pgDepth)-1)) ==bucketIndex)
					pdb->bucket[a]=dataPage->pageNo;
				else
					pdb->bucket[a]=brother_page->pageNo;
			}
		}
		
		for(a=0;a<dataPage->rows.size;a++){
			uint32_t hashcode;
			BucketIndex rowBucketIndex;
			DataRow dataRow=*(DataRow*)getVectorByIndex(&dataPage->rows,a);
			//assert(dataRow.curPageNo == dataPage->pageNo);
			hashcode = _hash((char*)dataRow.key,dataRow.keySize);
			//assert(hashcode == dataRow.hashcode);
			rowBucketIndex = hashcode & ((0x00000001 << dataPage->pgDepth) - 1);
			//assert(rowBucketIndex == bucketIndex || rowBucketIndex == brother_bucketIndex);
			if(rowBucketIndex == brother_bucketIndex){
				///////////////
				dataRow.curPageNo=brother_page->pageNo;
				///////////////
				addVector(&brother_page->rows,&dataRow);
				brother_page->reCalculate=true;
				
				removeVectorByIndex(&dataPage->rows,a);
				dataPage->reCalculate=true;
				
				a--;
			}
		}
		{
			int freeSpace;
			int brother_freeSpace;
			freeSpace=getDataPageFreeSpace(dataPage);
			brother_freeSpace=getDataPageFreeSpace(brother_page);
			if(freeSpace < 0){
				return splitDataPage(pdb,dataPage,bucketIndex);
			}
			else if(brother_freeSpace < 0){
				return splitDataPage(pdb,brother_page,brother_bucketIndex);
			}
		}
	}
	return 1;
}

int splitPage(PDB pdb,DataPage* dataPage,BucketIndex bucketIndex){
	int a,hashcode;
	PageNo* bucket=NULL;
	if(dataPage->pgDepth == pdb->dirDepth){
		pdb->dirDepth++;
		bucket = (PageNo*)realloc(pdb->bucket,sizeof(PageNo) * pow(2,pdb->dirDepth));
		assert(bucket != NULL);
		if(bucket == NULL)
			return 0;
		pdb->bucket=bucket;
		//memcpy(pdb->bucket +(long) pow(2,pdb->dirDepth - 1),pdb->bucket,pow(2,pdb->dirDepth - 1) * sizeof(PageNo));
		for(a=(unsigned int)powl(2,pdb->dirDepth-1);a < (unsigned int)(powl(2,pdb->dirDepth));a++){
			//int b=(unsigned int)powl(2,pdb->dirDepth-1);
			pdb->bucket[a]=pdb->bucket[a-(unsigned int)powl(2,pdb->dirDepth-1)];
		}
	}
	{
		PPage brother_page=NULL;
		PageNo brother_bucketIndex =(0x00000001 << (pdb->dirDepth - 1)) ^ bucketIndex;
		//PageNo brother_bucketIndex =(0x00000001 << (dataPage->pgDepth - 1)) ^ bucketIndex;
		PageNo brother_pageNo = pdb->bucket[brother_bucketIndex];
		assert(bucketIndex < pow(2,pdb->dirDepth));
		assert(brother_bucketIndex < pow(2,pdb->dirDepth));
		assert(brother_pageNo == dataPage->pageNo);
		if(dataPage->pageNo == brother_pageNo)
		{
			brother_page = newPage(pdb,PT_DATA);
			pdb->bucket[brother_bucketIndex]=brother_page->pageNo;
			brother_pageNo = brother_page->pageNo;
			addPage(pdb->pageCache,brother_page);
		}
		else{
			brother_page = getPage(pdb->pageCache,brother_pageNo);
		}
		if(brother_page == NULL)
			return 0;
		assert(brother_page->pageType == PT_DATA);
		dataPage->pgDepth++;
		brother_page->pgDepth = dataPage->pgDepth;
		//update depthCount
		pdb->depthCount[dataPage->pgDepth] += 2;
		pdb->depthCount[dataPage->pgDepth -1] -=1;
		
		for(a=0;a < dataPage->rows.size;a++){
			PageNo _bucketIndex;
			DataRow dataRow =*(DataRow*)getVectorByIndex(&dataPage->rows,a);
			hashcode = _hash(dataRow.key,dataRow.keySize);
			_bucketIndex = hashcode & ((0x00000001 << dataPage->pgDepth) - 1);
			//_bucketIndex = hashcode & ((1 << pdb->dirDepth) - 1);
			assert(_bucketIndex == bucketIndex || _bucketIndex == brother_bucketIndex);
			assert(pdb->bucket[_bucketIndex] == pdb->bucket[bucketIndex] || pdb->bucket[_bucketIndex] == pdb->bucket[brother_bucketIndex]);
			if(_bucketIndex == brother_bucketIndex){
				removeVectorByIndex(&dataPage->rows,a);
				dataPage->reCalculate=true;
				dataPage->dirty=true;
				a--;
				addVector(&((DataPage*)brother_page)->rows,&dataRow);
				brother_page->reCalculate=true;
				brother_page->dirty=true;
			}
		}
		{
			int freeSpace = getDataPageFreeSpace(dataPage);
			int brother_freeSpace = getFreeSpace(brother_page);
			if(freeSpace < 0){
				return splitPage(pdb,dataPage,bucketIndex);
			}
			else if(brother_freeSpace < 0){
				return splitPage(pdb,(DataPage*)brother_page,brother_bucketIndex);
			}
		}
	}
	
	return 1;
}

static int addRowToPage(PDB pHdb,DataPage* dataPage,DataRow dataRow){
	int dataRowSpace = getDataRowSpace(dataRow);
	int pageFreeSpace = getDataPageFreeSpace(dataPage);
	dataPage->dirty=true;
	dataPage->reCalculate=true;
	dataRow.curPageNo=dataPage->pageNo;
	if(pageFreeSpace > dataRowSpace){
		addVector(&dataPage->rows,&dataRow);
	}
	else{
		//split
		addVector(&dataPage->rows,&dataRow);
		return POT_SPLIT;
	}
	return 1;
}

int addRecordToPage(PDB pHdb,DataPage* dataPage,
				const void* pKey,int keySize,
				const void* pValue,int valueSize){
	OfResult ofResult;
	OfPage* ofPage=NULL;
	Package package;
	DataRow dataRow={0};
	package.key=(char*)pKey;package.keySize=keySize;
	package.value=(char*)pValue;package.valueSize=valueSize;
	ofResult = pHdb->strategy->ofStrategy(pHdb,(PPage)dataPage,package);
	if(ofResult.ofType == OFT_NOT){
		dataRow.key=(char*)malloc(sizeof(char) * (keySize + 1));
		if(!dataRow.key)
			goto ERROR;
		strncpy(dataRow.key,pKey,keySize);
		dataRow.key[keySize]='\0';
		dataRow.keySize=keySize;
		dataRow.value=(char*)malloc(sizeof(char*) * (valueSize + 1));
		if(!dataRow.value)
			goto ERROR;
		strncpy(dataRow.value,pValue,valueSize);
		dataRow.value[valueSize]='\0';
		dataRow.curValueSize=valueSize;
		dataRow.valueSize=valueSize;
		dataRow.ofPageNo=0;
		dataRow.hashcode =_hash((char*)pKey,keySize);
		return addRowToPage(pHdb,dataPage,dataRow);
	}
	else{
		//OFT_Mandatory has same operate with OFT_Condition
		PageNo ofPageNo=0;
		if(valueSize > ofResult.remainValues){
			ofPage = (OfPage*)newPage(pHdb,PT_OF);
			if(!ofPage){
				goto ERROR;
			}
			ofPageNo=ofPage->pageNo;
			ofPage->dirty=true;
			ofPage->reCalculate=true;
			//assert(ofResult.remainValues > 0);
			assert(valueSize - ofResult.remainValues < 4000);
			ofPage->value =(char*)malloc(sizeof(char) * (valueSize - ofResult.remainValues + 1));
			if(!ofPage->value){
				return 0;
			}
			//strncpy(ofPage->value,pValue + ofResult.remainValues,valueSize - ofResult.remainValues);
			strcpy(ofPage->value,pValue + ofResult.remainValues);
			ofPage->valueSize = valueSize - ofResult.remainValues;
			addPage(pHdb->pageCache,(PPage)ofPage);
		}
		{
			dataRow.key =(char*)malloc(sizeof(char) * (keySize + 1));
			if(!dataRow.key)
				goto ERROR;
			strncpy(dataRow.key,pKey,keySize);
			dataRow.key[keySize]='\0';
			dataRow.keySize=keySize;
			if(ofResult.remainValues > 0){
				dataRow.value=(char*)malloc(sizeof(char) * (ofResult.remainValues + 1));
				if(!dataRow.value)
					goto ERROR;
				strncpy(dataRow.value,pValue,ofResult.remainValues);
				dataRow.value[ofResult.remainValues]='\0';
			}
			else
				dataRow.value=NULL;
			dataRow.curValueSize=ofResult.remainValues;
			dataRow.valueSize=valueSize;
			dataRow.ofPageNo=ofPageNo;
			dataRow.hashcode= _hash((char*)pKey,keySize);
			return addRowToPage(pHdb,dataPage,dataRow);
		}
	}
	return 1;
ERROR:
	if(dataRow.key)
		free(dataRow.key);
	if(dataRow.value)
		free(dataRow.value);
	if(ofPage)
		freeOfPage(ofPage);
	return 0;
}
