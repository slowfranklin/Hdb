#include "ByteOrder.h"
#include <assert.h>

static EndianType endianType = ENDIAN_NULL;

EndianType checkEndianType(void){
	if(endianType == ENDIAN_NULL){
		short i=0x1234;
		unsigned char c=*(unsigned char*)&i;
		if(c == 0x12) endianType = BIG_ENDIAN;
		else if(c == 0x34) endianType = LITTLE_ENDIAN;
		else assert(0);
	}
	return endianType;
}
