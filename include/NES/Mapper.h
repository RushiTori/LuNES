#ifndef LU_NES_MAPPER_H
#define LU_NES_MAPPER_H

#include "Cartridge.h"

#define CARTRIDGE_BASE_ADDRESS 0x4020

u8 MapperReadMem(Cartridge* cart, u16 address);
void MapperWriteMem(Cartridge* cart, u16 address, u8 value);

#endif	// LU_NES_MAPPER_H