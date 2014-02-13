#ifndef util_h
#define util_h

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void setU64(char* buffer,int offset,uint64_t value);
uint64_t getU64(const char* buffer,int offset);

void setU32(char* buffer,int offset,uint32_t value);
uint32_t getU32(const char* buffer,int offset);

void setU16(char* buffer,int offset,uint16_t value);
uint16_t getU16(const char* buffer,int offset);

void setU8(char* buffer,int offset,uint8_t value);
uint8_t getU8(const char* buffer,int offset);


void seti8(char* buffer,int offset,int8_t value);
int8_t geti8(const char* buffer,int offset);

void seti16(char* buffer,int offset,int16_t value);
int16_t geti16(const char* buffer,int offset);

void seti32(char* buffer,int offset,int32_t value);
int32_t geti32(const char* buffer,int offset);

void seti64(char* buffer,int offset,int64_t value);
int64_t geti64(const char* buffer,int offset);

#ifdef __cplusplus
 }
#endif
#endif
