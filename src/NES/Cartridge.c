#include "Cartridge.h"

#define INES_HEADER_SIZE 16
#define TRAINER_PADDING_SIZE 512

#define HEADER_PRG_ROM_PAGES 4
#define HEADER_CHR_ROM_PAGES 5
#define HEADER_FLAG6 6
#define HEADER_FLAG7 7
#define HEADER_FLAG8 8
#define NES20_HEADER_HIGH_ROM 9
#define NES20_HEADER_PRG_RAM_SIZE 10
#define NES20_HEADER_VRAM_SIZE 11
#define NES20_HEADER_TV_SYSTEM 12
#define NES20_HEADER_VS_PPU_VARIANT 13
#define NES20_HEADER_MISC_ROMS 14
#define NES20_HEADER_DEFAULT_EXP_DEVICE 15

#define TRAINER_START 16

#define FLAG6_MIRRORING 0
#define FLAG6_HAS_NV 1
#define FLAG6_TRAINER 2
#define FLAG6_ALT_NMT_LAYOUT 3

#define FLAG7_VS_UNISYSTEM 0
#define FLAG7_PLAYCHOICE_10 1

#define HIGH_ROM_EXP_MASK

#define HIGH_ROM_PRG_BITS_START 0
#define HIGH_ROM_CHR_BITS_START 4
#define HIGH_ROM_PRG_BITS_SIZE 4
#define HIGH_ROM_CHR_BITS_SIZE 4

#define ROM_EXP_MUL_START 0
#define ROM_EXP_EXPONENT_START 2
#define ROM_EXP_MUL_SIZE 2
#define ROM_EXP_EXPONENT_SIZE 6

#define RAM_BASE_VAL 64

typedef enum HeaderFormat {
	HEADER_NES20 = 0,
	HEADER_INES,
	HEADER_ARCHAIC_INES,
	HEADER_INES07,
	HEADER_UNKNOWN,
} HeaderFormat;

static void CartridgeParseINES10Header(Cartridge* cart, u8* bytes) {
	cart->header.prgRomSize = bytes[HEADER_PRG_ROM_PAGES] * PRG_ROM_PAGE_SIZE;
	cart->header.chrRomSize = bytes[HEADER_CHR_ROM_PAGES] * CHR_ROM_PAGE_SIZE;

	cart->mapper.id = MakeByte(GetHighNybble(bytes[HEADER_FLAG7]), GetHighNybble(bytes[HEADER_FLAG6]));

	cart->header.scrollMode = GetFlag(bytes[HEADER_FLAG6], 0) ? SCROLLING_VERTICAL : SCROLLING_HORIZONTAL;

	cart->header.has512BytesPadding = GetFlag(bytes[HEADER_FLAG6], FLAG6_TRAINER);

	cart->header.hasNV = GetFlag(bytes[HEADER_FLAG6], FLAG6_HAS_NV);

	cart->header.consoleID = bytes[HEADER_FLAG7] & 0b11;

	if (cart->header.hasNV) {
		cart->header.prgNVRamSize = bytes[HEADER_FLAG8];
		if (cart->header.prgNVRamSize == 0) cart->header.prgNVRamSize = 8192;
	} else {
		cart->header.prgRamSize = bytes[HEADER_FLAG8];
		if (cart->header.prgRamSize == 0) cart->header.prgRamSize = 8192;
	}
}

static void CartridgeParseNES20Header(Cartridge* cart, u8* bytes) {
	CartridgeParseINES10Header(cart, bytes);

	// if 0, no ram, if non-zero, is (64 << prg(NV)RamShifts) bytes
	cart->header.prgRamSize = (64 << GetLowNybble(bytes[NES20_HEADER_PRG_RAM_SIZE]));
	if (cart->header.prgRamSize == 64) cart->header.prgRamSize = 0;
	cart->header.prgNVRamSize = (64 << GetHighNybble(bytes[NES20_HEADER_PRG_RAM_SIZE]));
	if (cart->header.prgNVRamSize == 64) cart->header.prgNVRamSize = 0;

	// if 0, no ram, if non-zero, is (64 << chr(NV)RamShifts) bytes
	cart->header.chrRamSize = (64 << GetLowNybble(bytes[NES20_HEADER_VRAM_SIZE]));
	if (cart->header.chrRamSize == 64) cart->header.chrRamSize = 0;
	cart->header.chrNVRamSize = (64 << GetHighNybble(bytes[NES20_HEADER_VRAM_SIZE]));
	if (cart->header.chrNVRamSize == 64) cart->header.chrNVRamSize = 0;

	// defines frame rate
	cart->header.expectedTimingMode = bytes[NES20_HEADER_TV_SYSTEM] & 0b11;

	// byte 13 ignored for now

	// TODO: might have to change the misc rom implem too, like i did for PRGRom and CHRRom
	cart->header.miscRomPages = bytes[NES20_HEADER_MISC_ROMS];

	cart->header.expDevice = bytes[NES20_HEADER_DEFAULT_EXP_DEVICE];

	cart->mapper.id =
		MakeWord(GetLowNybble(bytes[HEADER_FLAG8]), MakeByte(GetHighNybble(bytes[HEADER_FLAG7]), GetHighNybble(bytes[HEADER_FLAG6])));
	cart->mapper.submapperID = GetHighNybble(bytes[HEADER_FLAG8]);
}

Cartridge* CartridgeCreate(u8* bytes, size_t bytesSize) {
	const char* INES_HEADER = "NES\x1A";

	// header is invalid, we can't process the rom
	if (bytesSize < INES_HEADER_SIZE) return NULL;
	if (strncmp((char*)bytes, INES_HEADER, strlen(INES_HEADER))) return NULL;

	Cartridge* cart = malloc(sizeof(Cartridge));
	if (GetBits(bytes[NES20_HEADER_HIGH_ROM], HIGH_ROM_PRG_BITS_START, HIGH_ROM_PRG_BITS_SIZE) == 0b1111) {
		// romSize = 2^E * (MM*2+1)
		// for the bits of bytes[HEADER_PRG_ROM_PAGES] = EEEE EEMM
		cart->header.prgRomSize = (1 << GetBits(bytes[HEADER_PRG_ROM_PAGES], ROM_EXP_EXPONENT_START, ROM_EXP_EXPONENT_SIZE)) *
									  GetBits(bytes[HEADER_PRG_ROM_PAGES], ROM_EXP_MUL_START, ROM_EXP_EXPONENT_START) * 2 +
								  1;
	} else {
		cart->header.prgRomSize =
			MakeWord(GetBits(bytes[NES20_HEADER_HIGH_ROM], HIGH_ROM_PRG_BITS_START, HIGH_ROM_PRG_BITS_SIZE), bytes[HEADER_PRG_ROM_PAGES]) *
			PRG_ROM_PAGE_SIZE;
	}

	if (GetBits(bytes[NES20_HEADER_HIGH_ROM], HIGH_ROM_CHR_BITS_START, HIGH_ROM_CHR_BITS_SIZE) == 0b1111) {
		// romSize = 2^E * (MM*2+1)
		// for the bits of bytes[HEADER_PRG_ROM_PAGES] = EEEE EEMM
		cart->header.chrRomSize = (1 << GetBits(bytes[HEADER_CHR_ROM_PAGES], ROM_EXP_EXPONENT_START, ROM_EXP_EXPONENT_SIZE)) *
									  GetBits(bytes[HEADER_CHR_ROM_PAGES], ROM_EXP_MUL_START, ROM_EXP_MUL_SIZE) * 2 +
								  1;
	} else {
		cart->header.chrRomSize = MakeWord(GetBits(bytes[NES20_HEADER_HIGH_ROM], 0, 4), bytes[HEADER_CHR_ROM_PAGES]) * CHR_ROM_PAGE_SIZE;
	}
	cart->header.has512BytesPadding = GetFlag(bytes[HEADER_FLAG6], FLAG6_TRAINER);

	uint32_t estimateRomSize = 16 + cart->header.prgRomSize + cart->header.chrRomSize + (cart->header.has512BytesPadding ? 512 : 0);

	if (((bytes[HEADER_FLAG7] & 0x0C) == 0x08) && (estimateRomSize == bytesSize || bytes[NES20_HEADER_MISC_ROMS] != 0)) {
		// HEADER_NES20;
		CartridgeParseNES20Header(cart, bytes);

	} else if ((bytes[HEADER_FLAG7] & 0x0C) == 0x04) {
		// HEADER_ARCHAIC_INES;
		free(cart);
		return NULL;
		// NOT IMPLEMENTED

	} else if ((bytes[HEADER_FLAG7] & 0x0C) == 0x00 && bytes[12] == 0 && bytes[13] == 0 && bytes[14] == 0 && bytes[15] == 0) {
		// HEADER_INES;
		CartridgeParseINES10Header(cart, bytes);

	} else {
		// HEADER_UNKNOWN;
		free(cart);
		return NULL;
		// NOT IMPLEMENTED
	}

	/*size_t MiscRomSize = bytesSize - (INES_HEADER_SIZE + ((cart->header.has512BytesPadding) ? TRAINER_PADDING_SIZE : 0) +
											  cart->header.prgRomSize + cart->header.chrRomSize);*/

	cart->PRGRom = (cart->header.prgRomSize == 0) ? NULL : malloc(cart->header.prgRomSize);
	cart->CHRRom = (cart->header.chrRomSize == 0) ? NULL : malloc(cart->header.chrRomSize);

	cart->PRGRam = (cart->header.prgRamSize == 0) ? NULL : malloc(cart->header.prgRamSize);
	cart->CHRRam = (cart->header.chrRamSize == 0) ? NULL : malloc(cart->header.chrRamSize);

	cart->PRGNVRam = (cart->header.prgNVRamSize == 0) ? NULL : malloc(cart->header.prgNVRamSize);
	cart->CHRNVRam = (cart->header.chrNVRamSize == 0) ? NULL : malloc(cart->header.chrNVRamSize);

	// WIP Mapper and console dependant, to implement later
	// cart->MiscRom = (cart->header.miscRomPages == 0) ? NULL : malloc()

	size_t PRGRomStart = INES_HEADER_SIZE + (cart->header.has512BytesPadding) ? TRAINER_PADDING_SIZE : 0;
	size_t CHRRomStart = PRGRomStart + cart->header.prgRomSize;
	// size_t MiscRomStart = CHRRomStart + cart->header.chrRomSize;
	if (cart->header.prgRomSize > 0) memcpy(cart->PRGRom, &(bytes[PRGRomStart]), cart->header.prgRomSize);
	if (cart->header.chrRomSize > 0) memcpy(cart->CHRRom, &(bytes[CHRRomStart]), cart->header.chrRomSize);
	// if trainer data exists and we have space to store it, copy it to PRG-RAM
	if (cart->header.has512BytesPadding && cart->header.prgRamSize != 0) {
		memcpy(cart->PRGRam, &(bytes[TRAINER_START]), TRAINER_PADDING_SIZE);
	}
	// WIP Mapper and console dependant, to implement later
	// if (MiscRomSize > 0) memcpy(cart->MiscRom, &(bytes[MiscRomStart]), MiscRomSize);

	return cart;
}

void CartridgeDestroy(Cartridge* cart) {
	// free everything potentially allocated for the cartridge
	if (cart->PRGRom) free(cart->PRGRom);
	if (cart->CHRRom) free(cart->CHRRom);
	if (cart->PRGRam) free(cart->PRGRam);
	if (cart->CHRRam) free(cart->CHRRam);
	if (cart->PRGNVRam) free(cart->PRGNVRam);
	if (cart->CHRNVRam) free(cart->CHRNVRam);
	if (cart->MiscRom) free(cart->MiscRom);

	// then destroy it like it's a cartridge of E.T on the Atari 2600
	free(cart);
}

void CartridgeNVSave(Cartridge* cart, const char* filename) {
	if (!cart->header.hasNV) return;

	FILE* f = fopen(filename, "w");
	if (f == NULL) {
		printf("couldn't write save data, please check file permissions\n");
		return;
	}
	if (cart->PRGNVRam) fwrite(cart->PRGNVRam, 1, cart->header.prgNVRamSize, f);
	if (cart->CHRNVRam) fwrite(cart->CHRNVRam, 1, cart->header.chrNVRamSize, f);
	fclose(f);
}

void CartridgeNVLoad(Cartridge* cart, const char* filename) {
	if (!cart->header.hasNV) return;
	FILE* f = fopen(filename, "r");
	if (f == NULL) {
		printf("couldn't read save data, please check file permissions\n");
		return;
	}
	if (cart->PRGNVRam) fread(cart->PRGNVRam, 1, cart->header.prgNVRamSize, f);
	if (cart->CHRNVRam) fread(cart->CHRNVRam, 1, cart->header.chrNVRamSize, f);
	fclose(f);
}