#ifndef trace_h
#define trace_h
#include "const.h"
#include "CommonType.h"
#include <stdio.h>

typedef struct Trace Trace;
typedef enum TraceType TraceType;
enum TraceType{
	TRACE_INFO,
	TRACE_WARN,
	TRACE_ERROR
};

struct Trace{
	char tracePath[PATH_MAX];
	FILE* traceFile;
};

Trace* newTrace(const char* tracePath);
int initTrace(Trace*,const char* tracePath);
void traceWrite(Trace*,TraceType,char* file,int line,const char* farmat,...);
void freeTrace(Trace*,bool);

#ifdef DEBUG
void testTrace();
#endif
#endif