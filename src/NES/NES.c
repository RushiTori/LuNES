#include "NES.h"

void NESInit(NES* nes, PPUOnFrameCB onFrameCB) {
	*nes = (NES){0};

	CPUInit(&(nes->cpu));
	PPUInit(&(nes->ppu));
	nes->onFrame = onFrameCB;
}

bool NESLoadGame(NES* nes, const char* romFilePath) {
	// WIP
	NESUnloadGame(nes);
	nes->currentGame = CartridgeCreateFromFile(romFilePath);

	return (nes->currentGame != NULL);
}

void NESUnloadGame(NES* nes) {
	// WIP
	if (nes->currentGame) {
		// WIP: implement some sort of saving/backup
		CartridgeDestroy(nes->currentGame);
		nes->currentGame = NULL;
	}
}
