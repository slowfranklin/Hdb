#ifndef page_h
#define page_h

#include "CommonType.h"
#include "Vector.h"
#include <stdint.h>
#include <stdlib.h>

typedef uint32_t PageNo;
typedef uint32_t PageIndex;
typedef uint32_t BucketIndex;
typedef struct Page Page;
typedef struct Page* PPage;
typedef struct OfPage OfPage;
typedef struct DataPage DataPage;
typedef enum PageType PageType;
typedef enum PageFormatError PageFormatError;
typedef struct DataRow DataRow;
typedef struct Package Package;

#include "Hdb.h"
#define PAGE_STRUCT_SIZE (max(sizeof(DataPage),sizeof(OfPage)))

enum PageType{
	PT_NULL=0X00,
	PT_DATA=0x01,
	PT_OF  =0x02
};

enum PageFormatError{
	PFE_OK,
	PFE_PAGE_ERROR,
	PTE_ROW_ERROR
};

#define PAGE_STRUCT\
		PageNo pageNo;\
		PageType pageType;\
		bool dirty;\
		bool reCalculate;\
		int freeSpace;\
		int pgDepth;\
		long long pageLSN;

struct Page{
	PAGE_STRUCT
};

struct OfPage{
	PAGE_STRUCT
	char* value;
	int valueSize;
	PageNo nextPageNo;//next ofpage number
};

struct DataPage{
	PAGE_STRUCT
	Vector rows;
};

struct DataRow{
	char* key;
	int keySize;
	char* value;
	int curValueSize;
	int valueSize;
	PageNo ofPageNo;
	uint32_t hashcode;
	PageNo curPageNo;
};

struct Package{
	char* key;
	int keySize;
	char* value;
	int valueSize;
};

int getFreeSpace(PPage);
int getDataPageFreeSpace(DataPage*);
int getOfpageFreeSpace(OfPage*);
int getSpaceRoute(PPage);
bool canMerge(PPage,PPage);
int splitPage(PDB pdb,DataPage*,BucketIndex bucketIndex );
int splitDataPage(PDB pdb,DataPage* dataPage,BucketIndex bucketIndex);
int pageWriteToBuffer(PPage page,char* buffer,int offset);
int addRecordToPage(PDB hdb,DataPage* dataPage,const void* pKey,int keySize,const void* pValue,int valueSize);
int parsePage(Page* page,char* pageBuffer,int offset);
int freePage(PPage);
#endif
