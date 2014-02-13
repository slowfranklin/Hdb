#ifndef vcode_h
#define vcode_h
#include <stdint.h>

int setVCodei8(uint8_t data,char* dataBuffer,int offset);
int setVCodei16(uint16_t data,char* dataBuffer,int offset);
int setVCodei32(uint32_t data,char* dataBuffer,int offset);
int setVCodei64(uint64_t data,char* dataBuffer,int offset);

int getVCode(const char* dataBuffer,int offset,uint64_t* i);
int getVSize(uint64_t i);
int calculateVSize(uint64_t i);


#endif
