#ifndef journal_h
#define journal_h
#include "CommonType.h"
#include "const.h"
#include <stdio.h>
#include <stdint.h>

typedef struct Journal Journal;
typedef enum JournalType JournalType;
typedef struct JournalRecord JournalRecord;
typedef uint32_t PageNo;
enum JournalType{
	JT_INSERT = 0X00,
	JT_UPDATE,
	JT_DELETE,
	JT_SPLIT_PAGE,
	JT_MERGE_PAGE,
	JT_RECYCLE_PAGE,
	JT_BEGIN_TRANSACTION,
	JT_COMMIT_TRANSACTION,
	JT_ROLLBACK_TRANSACTION
};

#define JOURNAL_STRUCT\
	JournalType journalType;
	
struct JournalRecord{
	JournalType journalType;
	
	void* key;
	int keySize;
	void* value;
	int valueSize;
};

struct JournalSplitPage{
	JOURNAL_STRUCT
	PageNo pageNo;
};
//JournalBuffer
struct Journal{
	char journalPath[PATH_MAX];
	FILE* journalFile;
	uint64_t maxLSN;
};

Journal* newJournal(const char* journalPath);
int initJournal(Journal*,const char*);
void writeJournal(Journal*,JournalRecord journalRecord);
void freeJournal(Journal*,bool);
void checkPoint();
#endif
