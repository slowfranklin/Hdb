#ifndef hdb_h
#define hdb_h
#define _GNU_SOURCE

typedef struct Hdb Hdb;
typedef struct Hdb* PDB;
typedef struct Config Config;
#include "CommonType.h"
#include "Page.h"
#include "const.h"
#include "PageCache.h"
#include "Strategy.h"
#include "List.h"
#include "LinkList.h"
#include "Trace.h"
#include "Journal.h"
#include <stdint.h>

struct Config{
	char* tracepath;
	char* journalpath;
};

struct Hdb{
	int dirDepth;
	PageNo pageCount;
	int depthCount[32];
	PageNo* bucket;
	char dbPath[PATH_MAX];
	int fd;
	int index;
	PageCache* pageCache;
	Strategy* strategy;
	Linklist* freePages;
	CompareValueDelegate compareKeyFunc;
	Trace trace;
	Journal journal;
	Config config;
	uint64_t lsn;
};
PDB newHdb(void);
PDB openHdb(const char* filepath);
int insertHdb(PDB pdb,const void* key,int keySize,const void* value,int valueSize);
void* searchHdb(PDB pdb,const void* pKey,int keySize);
int deleteHdb(PDB pdb,const void* key,int keySize);
int beginTransaction(PDB);
int commitTransaction(PDB);
int rollBackTransaction(PDB);
int commitHdb(PDB pdb);
int verifyHdb(PDB pdb);
int closeHdb(PDB pdb);


#endif
