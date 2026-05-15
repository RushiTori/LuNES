#ifndef LU_NES_PIXEL_H
#define LU_NES_PIXEL_H

#include "Byte.h"

typedef u8 Pixel;

#define PPU_PIXEL_MIN 0x00
#define PPU_PIXEL_MAX 0x3F

// Returns the color in 0xRRGGBBAA format.
// Note: the alpha part will always equal 0xFF.
uint32_t PixelToRGBA(Pixel pixel);

// Returns the color in 0xAABBGGRR format.
// Note: the alpha part will always equal 0xFF.
uint32_t PixelToABGR(Pixel pixel);

#endif	// LU_NES_PIXEL_H
