#ifndef LU_NES_MAPPER_H
#define LU_NES_MAPPER_H

#include "Byte.h"
#include "Cartridge.h"

#define CARTRIDGE_BASE_ADDRESS 0x4020

#define PRG_ROM_PAGE_SIZE 16384
#define CHR_ROM_PAGE_SIZE 8192

typedef u8 (*MapperReadCb)(Mapper* mapper, u16 address);
typedef void (*MapperWriteCb)(Mapper* mapper, u16 address, u8 value);

typedef struct Mapper {
	MapperReadCb readMem;
	MapperWriteCb writeMem;
} Mapper;

// Mapper 0: MAPPER_NROM
u8 NROMReadMem(Cartridge* cart, u16 address);
void NROMWriteMem(Cartridge* cart, u16 address, u8 value);

// Mapper 1: MAPPER_MMC1
u8 MMC1ReadMem(Cartridge* cart, u16 address);
void MMC1WriteMem(Cartridge* cart, u16 address, u8 value);

// Mapper 2: MAPPER_UXROM
u8 UXROMReadMem(Cartridge* cart, u16 address);
void UXROMWriteMem(Cartridge* cart, u16 address, u8 value);

#endif	// LU_NES_MAPPER_H