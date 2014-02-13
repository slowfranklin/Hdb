#ifndef common_type_h
#define common_type_h
#include <stdint.h>

typedef unsigned char bool;
#define true 0x01
#define false 0x00

typedef void (*InitDelegate)(void* data);
typedef void (*FreeDelegate)(void *data);
typedef bool (*FindDelegate)(const void* data,const void* data2);
typedef void (*ForEachDelegate)(const void* data);
/*
data1==data2 0
data1>data2  1
data1<data2  -1
*/
typedef int (*CompareDelegate)(const void* data1,const void* data2);
typedef int (*CompareValueDelegate)(const char* v1,int v1Size,const char* v2,int v2Size);

#define COMPARE_EQUAL	(0)
#define COMPARE_BIGGER	(1)
#define COMPARE_LITTER	(-1)

#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
#endif
