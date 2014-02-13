#ifndef byte_order_h
#define byte_order_h

typedef enum EndianType EndianType;

enum EndianType{
	ENDIAN_NULL,
	BIG_ENDIAN,
	LITTLE_ENDIAN
};

EndianType checkEndianType(void);
#endif
