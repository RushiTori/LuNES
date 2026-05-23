#ifndef LU_NES_MMC3_H
#define LU_NES_MMC3_H

#include "Byte.h"

typedef struct MMC3Mapper {
	u8 regSelect;

	u8 r[8];

	u8 bankSelect;

	bool swapPRGRom;
	bool swapCHR;

	bool PRGRamEnable;
	bool PRGRamProtect;

	u8 irqCounter;

	u8 irqReloadValue;
	bool irqEnable;
} MMC3Mapper;

#endif	// LU_NES_MMC3_H