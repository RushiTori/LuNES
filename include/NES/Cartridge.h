#ifndef LU_NES_CARTRIDGE_H
#define LU_NES_CARTRIDGE_H

#include <stdlib.h>
#include <string.h>

#include "Byte.h"
#include "Mapper.h"

#define PRG_ROM_PAGE_SIZE 16384
#define CHR_ROM_PAGE_SIZE 8192

typedef enum ScrollingMode {
	SCROLLING_VERTICAL = 0,
	SCROLLING_HORIZONTAL,
	SCROLLING_FOUR_SCREENS,
	// TODO: add more scrolling modes
} ScrollingMode;

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

// TODO: add fields for actual ROM size and RAM size for both PRG and CHR, would help with actually handling weird sizes like 8KB PRG ROM
// (fucking galaxian... seriously did it save up that much money to use only 8KB of PRG ROM???)
typedef struct Header {
	size_t prgRomSize;
	size_t chrRomSize;
	u8 miscRomPages;

	bool hasPRGRam;
	u8 prgRamShifts;
	u8 prgNVRamShifts;

	u8 chrRamShifts;
	u8 chrNVRamShifts;

	ScrollingMode scrollMode;

	bool has512BytesPadding;
	bool usesAlternativeNametables;

	ConsoleID consoleID;  // TODO: account for expanded console types, and vs subsystems
	bool usesNES2Point0;

	MapperID mapperID;
	u8 submapperID;	 // TODO: change type

	u16 chrRamSize;

	TimingMode expectedTimingMode;
	u8 expDevice;  // expected controller
} Header;

typedef struct Cartridge {
	Header header;
	Mapper mapper;

	u8* PRGRom;
	u8* CHRRom;

	u8* PRGRam;
	u8* CHRRam;

	u8* PRGNVRam;
	u8* CHRNVRam;

	u8* MiscRom;
} Cartridge;

Cartridge* CartridgeCreate(u8* bytes, size_t bytesSize);
void CartridgeDestroy(Cartridge* cart);

#endif	// LU_NES_CARTRIDGE_H