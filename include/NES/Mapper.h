#ifndef LU_NES_MAPPER_H
#define LU_NES_MAPPER_H

typedef struct Cartridge Cartridge;

// mapper support includes

// MAPPER_NROM
#include "NROM.h"

// MAPPER_MMC1
#include "MMC1.h"

// MAPPER_UXROM
#include "UXROM.h"

// MAPPER_CNROM
#include "CNROM.h"

//-----------------------

#define CARTRIDGE_BASE_ADDRESS 0x4020

// TODO: add mappers up to mapper 7 because solstice
typedef enum MapperID {
	MAPPER_NROM = 0,
	MAPPER_MMC1,
	MAPPER_UXROM,
	MAPPER_CNROM,
	MAPPER_MMC3,
	MAPPER_MMC5,
	MAPPER_6,
	MAPPER_AXROM,
	MAPPER_6_SUB2,
	MAPPER_MMC2,
	MAPPER_MMC4,

	MAPPER_COUNT
} MapperID;

// do i implement like, an object that holds a union between every "Mapper" state? or do i rather have a struct for each mapper state, and then
// pass it as void*? both options sound bad, i don't wanna write 2000+ lines of code just to describe every mapper lmao and i don't want a struct
// with 132982309238 members

typedef struct Mapper {
	MapperID id;
	u8 submapperID;
	union {
		NROMMapper nrom;
		MMC1Mapper mmc1;
		UXROMMapper uxrom;
		CNROMMapper cnrom;
	};
} Mapper;

// TODO: implement a mapper object that'd hold states for every possible mapper (esoecially MMC ones)

// initializes the mapper data (registers for instance)
void MapperInit(Cartridge* cart);

// these handle CPU bus R/W operations
u8 MapperReadMemCPU(Cartridge* cart, u16 address);
void MapperWriteMemCPU(Cartridge* cart, u16 address, u8 value);

// these handle the PPU bus R/W operations
u8 MapperReadMemPPU(Cartridge* cart, u16 address);
void MapperWriteMemPPU(Cartridge* cart, u16 address, u8 value);

#endif	// LU_NES_MAPPER_H