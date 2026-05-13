#include "Mapper.h"

#include <math.h>

#define IsAddressInRange(addr, first, last) (((addr) >= (first)) && ((addr) <= (last)))
#define GetMirroredAddress(addr, first, range, start) ((start) + (((addr) - (first)) % (range)))

//-------------------------------------- Mapper 0: MAPPER_NROM  ---------------------------------------------

#define MAPPER_NROM_PRG_RAM_START 0x6000
#define MAPPER_NROM_PRG_RAM_END 0x7FFF
#define MAPPER_NROM_PRG_ROM_START 0x8000
#define MAPPER_NROM_PRG_ROM_END 0xFFFF

#define IsAddrInPrgROMMirror(addr, PRGPages) \
	IsAddressInRange((addr), MAPPER_NROM_PRG_ROM_START + PRGPages * PRG_ROM_PAGE_SIZE, MAPPER_NROM_PRG_ROM_END)
#define GetPrgROMMirrorAddr(addr, PRGPages)                                                                         \
	GetMirroredAddress((addr), MAPPER_NROM_PRG_ROM_START, MAPPER_NROM_PRG_ROM_START + PRGPages * PRG_ROM_PAGE_SIZE, \
					   MAPPER_NROM_PRG_ROM_START + PRGPages * PRG_ROM_PAGE_SIZE)

u8 NROM_ReadMem(Cartridge* cart, u16 address) {
	if (address >= MAPPER_NROM_PRG_RAM_START && address <= MAPPER_NROM_PRG_RAM_END) {
		// TODO: implement mirroring since now, we do know the size of PRG RAM through the header
		return (cart->header.hasPRGRam) ? cart->PRGRam[address - MAPPER_NROM_PRG_RAM_START] : 0;
	} else if (address < MAPPER_NROM_PRG_ROM_START + cart->header.prgRomPages * PRG_ROM_PAGE_SIZE) {
		return cart->PRGRom[address - MAPPER_NROM_PRG_ROM_START];
	} else if (IsAddrInPrgROMMirror(address, cart->header.prgRomPages)) {
		return NROMReadMem(cart, GetPrgROMMirrorAddr(address, cart->header.prgRomPages));
	}
	// should never happen
	return 0;
}

void NROM_WriteMem(Cartridge* cart, u16 address, u8 value) {
	if (address >= MAPPER_NROM_PRG_RAM_START && address < MAPPER_NROM_PRG_ROM_START) {
		// TODO: inaccuracy with NROMs with 2 KB of PRG RAM, mirroring to implement later
		cart->PRGRam[address - MAPPER_NROM_PRG_RAM_START] = value;
	}
}

//-------------------------------------- Mapper 1: MAPPER_MMC1  --------------------------------------------

u8 MMC1_ReadMem(Cartridge* cart, u16 address) {
	// WIP
}

void MMC1_WriteMem(Cartridge* cart, u16 address, u8 value) {
	// WIP
}

//-------------------------------------- Mapper 2: MAPPER_UXROM  -------------------------------------------

#define MAPPER_UXROM_SUBMAPPER_UNSPECIFIED 0
#define MAPPER_UXROM_NO_BUS_CONFLICT 1
#define MAPPER_UXROM_AND_BUS_CONFLICT 2

#define MAPPER_UXROM_PRG_ROM_BANKED_START 0x8000
#define MAPPER_UXROM_PRG_ROM_BANKED_END 0xBFFF
#define MAPPER_UXROM_PRG_ROM_FIXED_START 0xC000
#define MAPPER_UXROM_PRG_ROM_FIXED_END 0xFFFF

#define MAPPER_UXROM_PRG_BANK_REG_START 0x8000
#define MAPPER_UXROM_PRG_BANK_REG_END 0xFFFF

// no bus conflict emulation due to lack of documentation for now

static u8 UXROM_TruncateReg(Cartridge* cart, u8 value) {
	u8 bankCount = cart->header.prgRomPages;
	u8 bankMask = bankCount - 1;
	if (value >= bankCount) return value & bankMask;
	return value;
}

u8 UXROM_ReadMem(Cartridge* cart, u16 address) {
	if (address >= MAPPER_UXROM_PRG_ROM_BANKED_START && address <= MAPPER_UXROM_PRG_ROM_BANKED_END) {
		return cart->PRGRom[address + 0x4000 * cart->PRGBank];
	} else if (address >= MAPPER_UXROM_PRG_ROM_FIXED_START) {
		// reads last bank of PRG ROM
		return cart->PRGRom[address + 0x4000 * (cart->header.prgRomPages - 1)];
	}
}

void UXROM_WriteMem(Cartridge* cart, u16 address, u8 value) {
	if (address >= MAPPER_UXROM_PRG_BANK_REG_START) cart->PRGBank = UXROMTruncateReg(cart, value);
}

//-------------------------------------- Mapper 3: MAPPER_CNROM  -------------------------------------------

//----------------------------------------------------------------------------------------------------------
u8 MapperReadMem(Cartridge* cart, u16 address) {
	// WIP
}
void MapperWriteMem(Cartridge* cart, u16 address, u8 value) {
	// WIP
}