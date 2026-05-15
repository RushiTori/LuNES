#ifndef LU_NES_PPU_H
#define LU_NES_PPU_H

#include "Pixel.h"

#define PPU_FRAME_WIDTH 256
#define PPU_FRAME_HEIGHT 240

typedef Pixel FrameBuffer[PPU_FRAME_WIDTH * PPU_FRAME_HEIGHT];

typedef void (*PPUOnFrameCB)(FrameBuffer);

typedef struct PPU {
	FrameBuffer buffer;

	uint32_t cycles;
	int32_t scanline;
	uint64_t frames;
} PPU;

void PPUInit(PPU* ppu);

void PPUPlot(PPU* ppu, u8 x, u8 y, Pixel col);
void PPUStep(PPU* ppu, PPUOnFrameCB onFrame);

#endif	// LU_NES_PPU_H
