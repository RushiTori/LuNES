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
#define FLAG6_PRG_RAM 1
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

// TODO: fix parsing, it doesn't actually follow NES2.0 format properly
static void CartridgeParseNES20Header(Cartridge* cart, u8* bytes) {
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

	cart->header.scrollMode = GetFlag(bytes[6], 0) ? SCROLLING_VERTICAL : SCROLLING_HORIZONTAL;

	cart->header.has512BytesPadding = GetFlag(bytes[HEADER_FLAG6], FLAG6_TRAINER);

	cart->header.hasPRGRam = GetFlag(bytes[HEADER_FLAG6], FLAG6_PRG_RAM);

	cart->header.consoleID = bytes[HEADER_FLAG7] & 0b11;
	cart->header.mapperID =
		MakeWord(GetLowNybble(bytes[HEADER_FLAG8]), MakeByte(GetHighNybble(bytes[HEADER_FLAG7]), GetHighNybble(bytes[HEADER_FLAG6])));
	cart->header.submapperID = GetHighNybble(bytes[HEADER_FLAG8]);

	// if 0, no ram, if non-zero, is (64 << prg(NV)RamShifts) bytes
	cart->header.prgRamShifts = GetLowNybble(bytes[NES20_HEADER_PRG_RAM_SIZE]);
	cart->header.prgNVRamShifts = GetHighNybble(bytes[NES20_HEADER_PRG_RAM_SIZE]);

	// if 0, no ram, if non-zero, is (64 << chr(NV)RamShifts) bytes
	cart->header.chrRamShifts = GetLowNybble(bytes[NES20_HEADER_VRAM_SIZE]);
	cart->header.chrNVRamShifts = GetHighNybble(bytes[NES20_HEADER_VRAM_SIZE]);

	// defines frame rate
	cart->header.expectedTimingMode = bytes[NES20_HEADER_TV_SYSTEM] & 0b11;

	// byte 13 ignored for now

	// TODO: might have to change the misc rom implem too, like i did for PRGRom and CHRRom
	cart->header.miscRomPages = bytes[NES20_HEADER_MISC_ROMS];

	cart->header.expDevice = bytes[NES20_HEADER_DEFAULT_EXP_DEVICE];
}

Cartridge* CartridgeCreate(u8* bytes, size_t bytesSize) {
	const char* INES_HEADER = "NES\x1A";

	// header is invalid, we can't process the rom
	if (bytesSize < INES_HEADER_SIZE) return NULL;
	if (strncmp(bytes, INES_HEADER, strlen(INES_HEADER))) return NULL;

	uint32_t estimateRomSize = 16 + MakeWord(GetBits(bytes[NES20_HEADER_HIGH_ROM], 0, 4), bytes[HEADER_PRG_ROM_PAGES]) * PRG_ROM_PAGE_SIZE +
									   bytes[HEADER_CHR_ROM_PAGES] * CHR_ROM_PAGE_SIZE + GetFlag(bytes[HEADER_FLAG6], FLAG6_TRAINER)
								   ? 512
								   : 0;

	HeaderFormat fmt = HEADER_UNKNOWN;
	Cartridge* cart = malloc(sizeof(Cartridge));
	if (bytes[HEADER_FLAG7] & bytes[0x0C] == 0x08 && (estimateRomSize == bytesSize || bytes[NES20_HEADER_MISC_ROMS] != 0)) {
		fmt = HEADER_NES20;
		CartridgeParseNES20Header(cart, bytes);

		size_t MiscRomSize = bytesSize - (INES_HEADER_SIZE + ((cart->header.has512BytesPadding) ? TRAINER_PADDING_SIZE : 0) +
										  cart->header.prgRomSize + cart->header.chrRomSize);

		cart->PRGRom = (cart->header.prgRomSize == 0) ? NULL : malloc(cart->header.prgRomSize);
		cart->CHRRom = (cart->header.chrRomSize == 0) ? NULL : malloc(cart->header.chrRomSize);

		cart->PRGRam = (cart->header.prgRamShifts == 0) ? NULL : malloc(RAM_BASE_VAL << cart->header.prgRamShifts);
		cart->CHRRam = (cart->header.chrRamShifts == 0) ? NULL : malloc(RAM_BASE_VAL << cart->header.chrRamShifts);

		cart->PRGNVRam = (cart->header.prgNVRamShifts == 0) ? NULL : malloc(RAM_BASE_VAL << cart->header.prgNVRamShifts);
		cart->CHRNVRam = (cart->header.chrNVRamShifts == 0) ? NULL : malloc(RAM_BASE_VAL << cart->header.chrNVRamShifts);

		// WIP Mapper and console dependant, to implement later
		// cart->MiscRom = (cart->header.miscRomPages == 0) ? NULL : malloc()

		size_t PRGRomStart = INES_HEADER_SIZE + (cart->header.has512BytesPadding) ? TRAINER_PADDING_SIZE : 0;
		size_t CHRRomStart = PRGRomStart + cart->header.prgRomSize;
		size_t MiscRomStart = CHRRomStart + cart->header.chrRomSize;
		if (cart->header.prgRomSize > 0) memcpy(cart->PRGRom, &(bytes[PRGRomStart]), cart->header.prgRomSize);
		if (cart->header.chrRomSize > 0) memcpy(cart->CHRRom, &(bytes[CHRRomStart]), cart->header.chrRomSize);
		// if trainer data exists and we have space to store it, copy it to PRG-RAM
		if (cart->header.has512BytesPadding && cart->header.prgRamShifts != 0) {
			memcpy(cart->PRGRam, &(bytes[TRAINER_START]), TRAINER_PADDING_SIZE);
		}
		// WIP Mapper and console dependant, to implement later
		// if (MiscRomSize > 0) memcpy(cart->MiscRom, &(bytes[MiscRomStart]), MiscRomSize);

	} else if (bytes[HEADER_FLAG7] & bytes[0x0C] == 0x04) {
		fmt = HEADER_ARCHAIC_INES;
		free(cart);
		return NULL;
		// NOT IMPLEMENTED

	} else if (bytes[HEADER_FLAG7] & bytes[0x0C] == 0x00 && bytes[12] == bytes[13] == bytes[14] == bytes[15]) {
		fmt = HEADER_INES;
		free(cart);
		return NULL;
		// NOT IMPLEMENTED

	} else {
		fmt = HEADER_UNKNOWN;
		free(cart);
		return NULL;
		// NOT IMPLEMENTED
	}

	return cart;
}

Cartridge* CartridgeCreateFromFile(const char* romFilePath) {
	FILE* file = fopen(romFilePath, "rb");
	if (!file) return NULL;

	fseek(file, 0, SEEK_END);
	size_t romSize = ftell(file);
	fseek(file, 0, SEEK_SET);
	if (!romSize) {
		fclose(file);
		return NULL;
	}

	u8* romData = malloc(sizeof(u8) * romSize);
	if (!romData) {
		fclose(file);
		return NULL;
	}

	size_t readLen = fread(romData, sizeof(u8), romSize, file);
	while (readLen != romSize) {
		// Treating a bad read as an interrupted one
		// WIP: read errno to return in case of actual error
		size_t currRead = fread(romData + readLen, sizeof(u8), romSize - readLen, file);
		readLen += currRead;
	}

	fclose(file);

	Cartridge* cart = CartridgeCreate(romData, romSize);
	free(romData);

	return cart;
}

void CartridgeDestroy(Cartridge* cart) {}
