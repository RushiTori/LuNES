#include "Mapper.h"

#define IsAddressInRange(addr, first, last) (((addr) >= (first)) && ((addr) <= (last)))
#define GetMirroredAddress(addr, first, range, start) ((start) + (((addr) - (first)) % (range)))

//-------------------------------------- Mapper 0: MAPPER_NROM  ---------------------------------------------

#define MAPPER_NROM_PRG_RAM_START 0x6000
#define MAPPER_NROM_PRG_RAM_END 0x7FFF
#define MAPPER_NROM_PRG_ROM_START 0x8000
#define MAPPER_NROM_PRG_ROM_END 0xFFFF

#define IsAddrInPrgROMMirror(addr, PRGPages) \
	isAddressInRange((addr), MAPPER_NROM_PRG_ROM_START + PRGPages * PRG_ROM_PAGE_SIZE, MAPPER_NROM_PRG_ROM_END)
#define GetPrgROMMirrorAddr(addr, PRGPages)                                                                         \
	GetMirroredAddress((addr), MAPPER_NROM_PRG_ROM_START, MAPPER_NROM_PRG_ROM_START + PRGPages * PRG_ROM_PAGE_SIZE, \
					   MAPPER_NROM_PRG_ROM_START + PRGPages * PRG_ROM_PAGE_SIZE)

u8 NROMReadMem(Cartridge* cart, u16 address) {
	if (address >= MAPPER_NROM_PRG_RAM_START && address <= MAPPER_NROM_PRG_RAM_END)
		return (cart->header.hasBattery)
				   ? cart->PRGRam[address - MAPPER_NROM_PRG_RAM_START]
				   : 0;	 // used by family basic v3.0
						 // family basic and playbox basic not fully supported (mirroring on high part of PRG RAM not emulated)
	else if (address < MAPPER_NROM_PRG_ROM_START + cart->header.prgRomPages * PRG_ROM_PAGE_SIZE)
		return cart->PRGRom[address - MAPPER_NROM_PRG_ROM_START];
	else if (IsAddrInPrgROMMirror(address, cart->header.prgRomPages))
		return NROMReadMem(cart, GetPrgROMMirrorAddr(address, cart->header.prgRomPages));
	else
		return 0;  // should never happen
}

void NROMWriteMem(Cartridge* cart, u16 address, u8 value) {
	if (address >= MAPPER_NROM_PRG_RAM_START && address < MAPPER_NROM_PRG_ROM_START)
		cart->PRGRam[address - MAPPER_NROM_PRG_RAM_START] =
			value;	// TODO: inaccuracy with NROMs with 2 KB of PRG RAM, mirroring to implement later
}

//-------------------------------------- Mapper 1: MAPPER_MMC1  --------------------------------------------

//-------------------------------------- Mapper 2: MAPPER_UXROM  -------------------------------------------
u8 UXROMReadMem(Cartridge* cart, u16 address) {}

void UXROMWriteMem(Cartridge* cart, u16 address, u8 value) {}