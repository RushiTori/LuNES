#ifndef LU_NES_CARTRIDGE_H
#define LU_NES_CARTRIDGE_H

#include "Byte.h"

typedef enum MapperID {
	MAPPER_NROM = 0,
	MAPPER_MMC1,
	MAPPER_UXROM,
	MAPPER_CNROM,
	MAPPER_MMC3,
	MAPPER_MMC5,

	MAPPER_COUNT
} MapperID;

typedef enum ConsoleID {
	CONSOLE_NES_FAMICOM = 0,
	CONSOLE_VS_SYSTEM,
	CONSOLE_PLAYCHOICE10,
	CONSOLE_EXTENDED,
} ConsoleID;

typedef enum TimingMode {
	TIMING_MODE_NTSC = 0,
	TIMING_MODE_PAL,
	TIMING_MODE_MULTIPLE,
	TIMING_MODE_DENDY,
} TimingMode;

typedef struct Header {
	u16 prgRomPages;
	u16 chrRomPages;
	bool hasBattery;
	bool has512BytesPadding;
	bool usesAlternativeNametables;

	ConsoleID consoleID;  // TODO: account for expanded console types, and vs subsystems
	bool usesNES2Point0;

	MapperID mapperID;
	u8 submapperID;	 // TODO: change type

	u16 chrRamSize;

	TimingMode expectedTimingMode;
	u8 miscRomsCount;
	u8 expDevice;
} Header;

typedef struct Cartridge {
	Header header;

	u8* PRGRom;
	u8* CHRRom;

	u8* PRGRam;
	u8* CHRRam;
} Cartridge;

#endif	// LU_NES_CARTRIDGE_H