#ifndef page_cache_h
#define page_cache_h

typedef struct PageCache PageCache;

#include "Page.h"
#include "Hdb.h"
#include "Hashtable.h"
#include "DList.h"

struct PageCache{
	Hashtable* pageFinder;
	DList* pageStore;
	PDB pdb;
};

PageCache* newPageCache(Hdb*);
int freePageCache(PageCache*);
PPage getPage(PageCache* pageCache,PageNo pageNo);
int addPage(PageCache* pageCache,PPage page);
//int removePage(PageCache* pageCache,PageNo);
PPage newPage(PDB,PageType);
int writePage(PDB,PPage);
#endif
