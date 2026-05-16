#ifndef LU_NES_H
#define LU_NES_H

#include "CPU.h"
#include "Cartridge.h"
#include "PPU.h"

typedef struct NES {
	CPU cpu;
	PPU ppu;
	PPUOnFrameCB onFrame;
	Cartridge* currentGame;
} NES;

void NESInit(NES* nes, PPUOnFrameCB onFrameCB);

bool NESLoadGame(NES* nes, const char* romFilePath);
void NESUnloadGame(NES* nes);

#endif	// LU_NES_H
