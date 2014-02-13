/*
 * Copyright (C) Austin Appleby
 */

#ifndef _MURMUR_HASH_H_
#define _MURMUR_HASH_H_
#include <stdint.h>
#include <stdlib.h>
static inline uint32_t 
murmur_hash2(char *data, size_t len)
{
	uint32_t  h, k;

	h = 0 ^ len;

	while (len >= 4) {
		k  = data[0];
		k |= data[1] << 8;
		k |= data[2] << 16;
		k |= data[3] << 24;

		k *= 0x5bd1e995;
		k ^= k >> 24;
		k *= 0x5bd1e995;

		h *= 0x5bd1e995;
		h ^= k;

		data += 4;
		len -= 4;
	}

	switch (len) {
	case 3:
		h ^= data[2] << 16;
	case 2:
		h ^= data[1] << 8;
	case 1:
		h ^= data[0];
		h *= 0x5bd1e995;
	}

	h ^= h >> 13;
	h *= 0x5bd1e995;
	h ^= h >> 15;

	return h;
}


static inline int
_hash (char* key,int keySize)
{
  unsigned int value;	/* Used to compute the hash value.  */
  int   index;		/* Used to cycle through random values. */


  /* Set the initial value from key. */
  value = 0x238F13AF * keySize;
  for (index = 0; index < keySize; index++)
    value = (value + (key[index] << (index*5 % 24))) & 0x7FFFFFFF;

  value = (1103515243 * value + 12345) & 0x7FFFFFFF;  

  /* Return the value. */
  return((int) value);
}

#endif
