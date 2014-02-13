#include "VCode.h"
#include <assert.h>


#define VCODE1 (127)
#define VCODE2 (16383)
#define VCODE3 (2097151)
#define VCODE4 (268435455)
#define VCODE5 (34359738367)
#define VCODE6 (4398046511103)
#define VCODE7 (562949953421311)
#define VCODE8 (72057594037927935)
#define VCODE9 (9223372036854775807)

int getVSize(uint64_t i){
	int byte=1;
	if(i<=VCODE1) byte=1;
	else if(i<=VCODE2) byte=2;
	else if(i<=VCODE3) byte=3;
	else if(i<=VCODE4) byte=4;
	else if(i<=VCODE5) byte=5;
	else if(i<=VCODE6) byte=6;
	else if(i<=VCODE7) byte=7;
	else if(i<=VCODE8) byte=8;
	else if(i<=VCODE9) byte=9;
	else byte=10;
	return byte;
}

int calculateVSize(uint64_t i){
	int vsize = 0;
	do{
		vsize++;
	}while(i >>=7);
	return vsize;
}

int setVCodei8(uint8_t data,char* dataBuffer,int offset){
	int bytes=0,i;
	while(data > 0){
		unsigned char c = (unsigned char)(data & 0x7F);
		dataBuffer[offset++] = c;
		bytes+=1;
		data >>= 7;
	}
	for(i=0;i<bytes-1; i++){
		dataBuffer[offset - bytes + i] |=(1<<7);
	}
	assert(bytes > 0);
	return bytes;
}
//from low to high
int setVCodei16(uint16_t data,char* dataBuffer,int offset){
	int bytes=0,i;
	while(data > 0){
		unsigned char c = (unsigned char)(data & 0x7F);
		dataBuffer[offset++] = c;
		bytes+=1;
		data >>= 7;
	}
	for(i=0;i<bytes-1; i++){
		dataBuffer[offset - bytes + i] |=(1<<7);
	}
	assert(bytes > 0);
	return bytes;
}

int setVCodei32(uint32_t data,char* dataBuffer,int offset){
	int bytes=0,i;
	while(data > 0){
		unsigned char c = (unsigned char)(data & 0x7F);
		dataBuffer[offset++] = c;
		bytes+=1;
		data >>= 7;
	}
	for(i=0;i<bytes-1; i++){
		dataBuffer[offset - bytes + i] |=(1<<7);
	}
	assert(bytes > 0);
	return bytes;
}

int setVCodei64(uint64_t data,char* dataBuffer,int offset){
	int bytes=0,i;
	while(data > 0){
		unsigned char c = (unsigned char)(data & 0x7F);
		dataBuffer[offset++] = c;
		bytes+=1;
		data >>= 7;
	}
	for(i=0;i<bytes-1; i++){
		dataBuffer[offset - bytes + i] |=(1<<7);
	}
	assert(bytes > 0);
	return bytes;
}

int getVCode(const char* dataBuffer,int offset,uint64_t* i){
	int vsize;
	uint64_t vcode=0;
	vsize = 0;
	assert(dataBuffer);
	do{
		unsigned char temp = (dataBuffer[offset] & 0x7f);
		vcode = vcode | (temp << (vsize * 7));
		vsize += 1;
	}
	while(dataBuffer[offset++] & 0x80);
	*i = vcode;
	return vsize;
}
