#include "Trace.h"
#include "const.h"
#include <assert.h>
#include <time.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>
#include <malloc.h>
#include <string.h>

#define TRACE_NAME "%s.log"
#define TRACE_FILE_LIMIT (1024 * 1024 * 100)

static int newTraceFile(Trace* trace,const char* tracePath){
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
	return 1;
}

Trace* newTrace(const char* tracePath){
	char _tracePath[PATH_MAX];
	Trace* trace;
	if(!tracePath){
		getcwd(_tracePath,PATH_MAX);
	}
	trace = (Trace*)malloc(sizeof(Trace));
	if(trace){
		if(!newTraceFile(trace,_tracePath))
			goto ERROR;
	}
	return trace;
ERROR:
	if(trace)
		free(trace);
	return NULL;
}

int initTrace(Trace* trace,const char* tracePath){
	assert(trace);
	if(trace){
		return newTraceFile(trace,tracePath);
	}
	return 0;
}

void freeTrace(Trace* trace,bool isNew){
	assert(trace);
	if(trace){
		if(trace->traceFile){
			fflush(trace->traceFile);
			fclose(trace->traceFile);
		}
		if(isNew)
			free(trace);
	}
	
}

static void traceAppend(Trace* trace,TraceType traceType,char* msg,char* file,int line){
	time_t now=time(NULL);
	char buf[64];
	strftime(buf,sizeof(buf), "%d %b %I:%M:%S",localtime(&now));
	
	if(traceType == TRACE_ERROR){
		/*
		fprintf(stderr, "[%d] %s %c %s, os-error:%s %s:%d\n",
											(int)getpid(),
											buf,
											traceType,
											msg,
											strerror(errno),
											file,
											line);
		*/
		fprintf(trace->traceFile, "[%d] %s %c %s, os-error:%s %s:%d\n",
						(int)getpid(),
						buf,
						traceType,
						msg,
						strerror(errno),
						file,
						line);
						
		fflush(trace->traceFile);
	}
	else{
		/*
		fprintf(stderr, "[%d] %s %c %s\n",
                                (int)getpid(),
                                buf,
                                traceType,
                                msg);
		*/
		fprintf(trace->traceFile, "[%d] %s %c %s\n",
                                (int)getpid(),
                                buf,
                                traceType,
                                msg);
	}
}

void traceWrite(Trace* trace,TraceType traceType,
	char* file,int line,const char* format,...){
	{
		//fseek(trace->traceFile,0L,SEEK_END);
		int filesize=ftell(trace->traceFile);
		if(filesize > TRACE_FILE_LIMIT){
			FILE* oldFile;
			oldFile=trace->traceFile;
			newTraceFile(trace,trace->tracePath);
			fflush(oldFile);
			fclose(oldFile);
		}
	}
	char logText[1024 * 2];
	va_list ap;
	va_start(ap,format);
	vsnprintf(logText,sizeof(logText),format,ap);
	va_end(ap);
	traceAppend(trace,traceType,logText,file,line);
}
#ifdef DEBUG
#include <unistd.h>

void testTrace(){
	int i=0;
	char filepath[PATH_MAX];
	Trace trace;
	getcwd(filepath,PATH_MAX);
	initTrace(&trace,filepath);
	while(i<1000){
		traceWrite(&trace,TRACE_INFO,"Trace.c",113,"%d-%s",i,"weijunbao");
		i++;
	}
}

#endif
