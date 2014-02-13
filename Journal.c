#include "Journal.h"
#include <assert.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>

#define JOURNAL_NAME "%s.journal"
#define JOURNAL_FIEL_SIZE (1024 * 1024 * 512)
static int newJournalFile(Journal* trace){
	/*
	char trace_path[PATH_MAX];
	char buf[64]={'\0'};
	time_t now=time(NULL);
	memset(trace,'\0',sizeof(Trace));
	strftime(buf,sizeof(buf), "%d-%b-%I:%M:%S",localtime(&now));
	snprintf(trace_path,PATH_MAX,TRACE_NAME,buf);
	trace->traceFile=fopen(trace_path,"a+");
	if(!trace->traceFile){
		fprintf(stderr,"trace file:%s,%s\n",trace_path,strerror(errno));
		return 0;
	}
	*/
	return 1;
}

Journal* newJournal(const char* journalPath){
	Journal* journal;
	assert(journalPath);
	journal=(Journal*)malloc(sizeof(Journal));
	if(journal){
		strcpy(journal->journalPath,journalPath);
		
	}
	return journal;
}

int initJournal(Journal* journal,const char* journalPath){
	assert(journalPath);
	return 1;
}

void writeJournal(Journal* journal,JournalRecord journalRecord){
	/*
	char smllBuffer[1024];
	assert(journal);
	{
		int filesize=ftell(journal->journalFile);
		if(filesize >= JOURNAL_FIEL_SIZE){
			newJournalFile(journal);
		}
	}
	fwrite(journal->journalFile,smallBuffer,1);
	*/
}

void freeJournal(Journal* journal,bool isnew){
	if(journal){
		fflush(journal->journalFile);
		fclose(journal->journalFile);
		if(isnew)
			free(journal);
	}
}
