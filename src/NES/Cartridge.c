#include "Cartridge.h"

#define INES_HEADER_SIZE 16

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

typedef enum HeaderFormat {
	HEADER_NES20 = 0,
	HEADER_INES,
	HEADER_ARCHAIC_INES,
	HEADER_INES07,
	HEADER_UNKNOWN,
} HeaderFormat;

// TODO: rename to CartridgeParseNES20Header
static void CartridgeParseNES20Header(Cartridge* cart, u8* bytes) {
	cart->header.prgRomPages = MakeWord(bytes[NES20_HEADER_HIGH_PRG_ROM], bytes[HEADER_PRG_ROM_PAGES]);
	cart->header.chrRomPages = bytes[HEADER_CHR_ROM_PAGES];

	cart->header.has512BytesPadding = GetFlag(bytes[HEADER_FLAG6], FLAG6_TRAINER);

	cart->header.hasPRGRam = GetFlag(bytes[HEADER_FLAG6], FLAG6_PRG_RAM);

	cart->header.prgRamShifts = GetLowNybble(bytes[NES20_HEADER_PRG_RAM_SIZE]);
	cart->header.prgNVRamShifts = GetHighNybble(bytes[NES20_HEADER_PRG_RAM_SIZE]);

	cart->header.chrRamShifts = GetLowNybble(bytes[NES20_HEADER_VRAM_SIZE]);
	cart->header.chrNVRamShifts = GetHighNybble(bytes[NES20_HEADER_VRAM_SIZE]);

	cart->header.expectedTimingMode = bytes[NES20_HEADER_TV_SYSTEM] & 0b11;

	cart->header.mapperID =
		MakeWord(GetLowNybble(bytes[HEADER_FLAG8]), MakeByte(GetHighNybble(bytes[HEADER_FLAG7]), GetHighNybble(bytes[HEADER_FLAG6])));
	cart->header.submapperID = GetHighNybble(bytes[HEADER_FLAG8]);
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
	} else if (bytes[HEADER_FLAG7] & bytes[0x0C] == 0x04) {
		fmt = HEADER_ARCHAIC_INES;
	} else if (bytes[HEADER_FLAG7] & bytes[0x0C] == 0x00 && bytes[12] == bytes[13] == bytes[14] == bytes[15]) {
		fmt = HEADER_INES;
	} else {
		fmt = HEADER_UNKNOWN;
	}
}

void CartridgeDestroy(Cartridge* cart) {}