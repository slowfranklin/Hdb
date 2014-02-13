#include "util.h"

void setU64(char* buffer,int offset,uint64_t value){

}

uint64_t getU64(const char* buffer,int offset){
	return  (buffer[offset + 7]  & 0xFF) |	(buffer[offset + 6] <<8)|
			(buffer[offset + 5] <<16) | (buffer[offset + 4] <<24) |
			((uint64_t)buffer[offset + 3]) <<32 | ((uint64_t)buffer[offset + 2] <<40) |
			((uint64_t)buffer[offset + 1]) <<48 | ((uint64_t)buffer[offset + 0] <<56);
}

void setU32(char* buffer,int offset,uint32_t value){
	buffer[offset + 3] = (unsigned char)(value & 0xff);
	buffer[offset + 2] = (unsigned char)((value >> 8) & 0xff);
	buffer[offset + 1] = (unsigned char)((value >> 16) & 0xff);
	buffer[offset + 0] = (unsigned char)((value >> 24) & 0xff);
}

uint32_t getU32(const char* buffer,int offset){
	return 	((unsigned char)buffer[offset+0] << 24) | 
			((unsigned char)buffer[offset+1] << 16) | 
			((unsigned char)buffer[offset+2] << 8)  | 
			((unsigned char)buffer[offset+3] & 0xff);
}

void setU16(char* buffer,int offset,uint16_t value){
	buffer[offset+1]=(unsigned char)(value & 0xFF);
	buffer[offset+0]=(unsigned char)((value>>8)& 0xFF);
}

uint16_t getU16(const char* buffer,int offset){
	return	(buffer[offset + 1] & 0xFF) | (buffer[offset + 0] << 8);
}

void setU8(char* buffer,int offset,uint8_t value){
	buffer[offset]=value;
}

uint8_t getU8(const char* buffer,int offset){
	return buffer[offset];
}

void seti8(char* buffer,int offset,int8_t value){
	buffer[offset]=value;
}

int8_t geti8(const char* buffer,int offset){
	return buffer[offset];
}

void seti16(char* buffer,int offset,int16_t value){
	buffer[offset+1]=(char)(value & 0xFF);
	buffer[offset+0]=(char)((value>>8)& 0xFF);
}

int16_t geti16(const char* buffer,int offset){
	return	buffer[offset + 1] | buffer[offset + 0] << 8;
}

void seti32(char* buffer,int offset,int32_t value){
	buffer[offset + 3] = (char)(value & 0xff);
	buffer[offset + 2] = (char)((value >> 8) & 0xff);
	buffer[offset + 1] = (char)((value >> 16) & 0xff);
	buffer[offset + 0] = (char)((value >> 24) & 0xff);
}

int32_t geti32(const char* buffer,int offset){
	return (buffer[offset+0]<<24) | (buffer[offset+1]<<16) | (buffer[offset+2]<<8) | buffer[offset+3];
}

void seti64(char* buffer,int offset,int64_t value){

}

int64_t geti64(const char* buffer,int offset){
	return 1;
}
