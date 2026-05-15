#include "Cartridge.h"

#define INES_HEADER_SIZE 16
#define TRAINER_PADDING_SIZE 512

#define HEADER_PRG_ROM_PAGES 4
#define HEADER_CHR_ROM_PAGES 5
#define HEADER_FLAG6 6
#define HEADER_FLAG7 7
#define HEADER_FLAG8 8
#define NES20_HEADER_HIGH_PRG_ROM 9
#define NES20_HEADER_PRG_RAM_SIZE 10
#define NES20_HEADER_VRAM_SIZE 11
#define NES20_HEADER_TV_SYSTEM 12
#define NES20_HEADER_VS_PPU_VARIANT 13
#define NES20_HEADER_MISC_ROMS 14
#define NES20_HEADER_DEFAULT_EXP_DEVICE 15

#define FLAG6_MIRRORING 0
#define FLAG6_PRG_RAM 1
#define FLAG6_TRAINER 2
#define FLAG6_ALT_NMT_LAYOUT 3

#define FLAG7_VS_UNISYSTEM 0
#define FLAG7_PLAYCHOICE_10 1

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
	cart->header.prgRomPages = MakeWord(bytes[NES20_HEADER_HIGH_PRG_ROM], bytes[HEADER_PRG_ROM_PAGES]);
	cart->header.chrRomPages = bytes[HEADER_CHR_ROM_PAGES];

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

	cart->header.miscRomPages = bytes[NES20_HEADER_MISC_ROMS];

	cart->header.expDevice = bytes[NES20_HEADER_DEFAULT_EXP_DEVICE];
}

Cartridge* CartridgeCreate(u8* bytes, size_t bytesSize) {
	const char* INES_HEADER = "NES\x1A";

	// header is invalid, we can't process the rom
	if (bytesSize < INES_HEADER_SIZE) return NULL;
	if (strncmp(bytes, INES_HEADER, strlen(INES_HEADER))) return NULL;

	uint32_t estimateRomSize = 16 + (bytes[NES20_HEADER_HIGH_PRG_ROM] << 8 | bytes[HEADER_PRG_ROM_PAGES]) * PRG_ROM_PAGE_SIZE +
									   bytes[HEADER_CHR_ROM_PAGES] * CHR_ROM_PAGE_SIZE + GetFlag(bytes[HEADER_FLAG6], FLAG6_TRAINER)
								   ? 512
								   : 0;

	HeaderFormat fmt = HEADER_UNKNOWN;
	Cartridge* cart = malloc(sizeof(Cartridge));
	if (bytes[HEADER_FLAG7] & bytes[0x0C] == 0x08 && (estimateRomSize == bytesSize || bytes[NES20_HEADER_MISC_ROMS] != 0)) {
		fmt = HEADER_NES20;
		CartridgeParseNES20Header(cart, bytes);

		size_t PRGRomSize = cart->header.prgRomPages * PRG_ROM_PAGE_SIZE;
		size_t CHRRomSize = cart->header.chrRomPages * CHR_ROM_PAGE_SIZE;
		size_t MiscRomSize =
			bytesSize - (INES_HEADER_SIZE + ((cart->header.has512BytesPadding) ? TRAINER_PADDING_SIZE : 0) + PRGRomSize + CHRRomSize);

		cart->PRGRom = (cart->header.prgRomPages == 0) ? NULL : malloc(PRGRomSize);
		cart->CHRRom = (cart->header.chrRomPages == 0) ? NULL : malloc(CHRRomSize);

		cart->PRGRam = (cart->header.prgRamShifts == 0) ? NULL : malloc(RAM_BASE_VAL << cart->header.prgRamShifts);
		cart->CHRRam = (cart->header.chrRamShifts == 0) ? NULL : malloc(RAM_BASE_VAL << cart->header.chrRamShifts);

		cart->PRGNVRam = (cart->header.prgNVRamShifts == 0) ? NULL : malloc(RAM_BASE_VAL << cart->header.prgNVRamShifts);
		cart->CHRNVRam = (cart->header.chrNVRamShifts == 0) ? NULL : malloc(RAM_BASE_VAL << cart->header.chrNVRamShifts);

		// WIP Mapper and console dependant, to implement later
		// cart->MiscRom = (cart->header.miscRomPages == 0) ? NULL : malloc()

		size_t PRGRomStart = INES_HEADER_SIZE + (cart->header.has512BytesPadding) ? TRAINER_PADDING_SIZE : 0;
		size_t CHRRomStart = PRGRomStart + PRGRomSize;
		size_t MiscRomStart = CHRRomStart + CHRRomSize;
		if (PRGRomSize > 0) memcpy(cart->PRGRom, &(bytes[PRGRomStart]), PRGRomSize);
		if (CHRRomSize > 0) memcpy(cart->CHRRom, &(bytes[CHRRomStart]), CHRRomSize);
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

	cart->PRGBank = 0;
	cart->CHRBank = 0;

	return cart;
}

void CartridgeDestroy(Cartridge* cart) {}