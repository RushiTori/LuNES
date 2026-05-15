#include "PPU.h"

#define PPU_CYCLES_PER_SCANLINE 340
#define PPU_SCNALINES_PER_FRAME 260

void PPUInit(PPU* ppu) {
	*ppu = (PPU){0};
	ppu->scanline = -1;
}

void PPUPlot(PPU* ppu, u8 x, u8 y, Pixel col) { ppu->buffer[x + y * PPU_FRAME_WIDTH] = col; }

void PPUStep(PPU* ppu, PPUOnFrameCB onFrame) {
	ppu->cycles++;
	if (ppu->cycles <= PPU_CYCLES_PER_SCANLINE) return;

	ppu->cycles = 0;
	ppu->scanline++;
	if (ppu->scanline <= PPU_SCNALINES_PER_FRAME) return;

	ppu->scanline = -1;
	ppu->frames++;

	if (onFrame) onFrame(ppu->buffer);
}
