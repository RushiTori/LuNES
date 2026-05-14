#ifndef LU_NES_BYTE_H
#define LU_NES_BYTE_H

#include <stdbool.h>
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;

typedef int8_t s8;
typedef int16_t s16;

#define GetBit(field, index) (((field) >> (index)) & 1)
#define GetFlag(field, index) (GetBit((field), (index)) ? true : false)
#define SetBit(field, index) ((field) | (1 << (index)))
#define ClearBit(field, index) ((field) & ~(1 << (index)))
#define FlipBit(field, index) ((field) ^ (1 << (index)))

#define GetBits(field, index, len) (((field) & (((1 << (len)) - 1) << (index))) >> (index))

#define GetHighNybble(byte_) (((byte_) >> 4) & 0x0F)
#define GetLowNybble(byte_) ((byte_) & 0x0F)

#define MakeByte(high, low) ((((high) & 0x0F) << 4) | ((low) & 0x0F))
#define MakeField(b0, b1, b2, b3, b4, b5, b6, b7)                                                                            \
	((((b0) & 1) << 0) | (((b1) & 1) << 1) | (((b2) & 1) << 2) | (((b3) & 1) << 3) | (((b4) & 1) << 4) | (((b5) & 1) << 5) | \
	 (((b6) & 1) << 6) | (((b7) & 1) << 7))

#define GetHighByte(word) (((word) >> 8) & 0xFF)
#define GetLowByte(word) ((word) & 0xFF)
#define MakeWord(high, low) ((((high) & 0xFF) << 8) | ((low) & 0xFF))

u8 AssignBit(u8 field, u8 index, bool isSet);

#endif	// LU_NES_BYTE_H
