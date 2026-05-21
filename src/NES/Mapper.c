#include "Mapper.h"

#include <math.h>

#include "Cartridge.h"

#define IsAddressInRange(addr, first, last) (((addr) >= (first)) && ((addr) <= (last)))
#define GetMirroredAddress(addr, first, range, start) ((start) + (((addr) - (first)) % (range)))

// BIGGGG TODO: actually have some sort of way of handling PRG-NVRAM, not sure of how to do that, gotta read more docs

//-------------------------------------- Mapper 0: MAPPER_NROM  ---------------------------------------------

#define MAPPER_NROM_PRG_RAM_START 0x6000
#define MAPPER_NROM_PRG_RAM_END 0x7FFF
#define MAPPER_NROM_PRG_ROM_START 0x8000
#define MAPPER_NROM_PRG_ROM_END 0xFFFF

#define IsAddrInPrgROMMirror(addr, PRGSize) ((addr) >= (MAPPER_NROM_PRG_ROM_START + (PRGSize)))
#define GetPrgROMMirrorAddr(addr, PRGSize) \
	GetMirroredAddress((addr), MAPPER_NROM_PRG_ROM_START, (MAPPER_NROM_PRG_ROM_START + (PRGSize) - 1), (MAPPER_NROM_PRG_ROM_START + (PRGSize)))

// NROM doesn't need init

// TODO: test this actually, i wanna check if mirroring is handled correctly
//  when the emulator'll be actually rolled out, i'll probably try galaxian to check if the 8KB of rom breaks anything
u8 NROM_ReadMemCPU(Cartridge* cart, u16 address) {
	// doesn't use mapper since mapper is literally empty
	if (address >= MAPPER_NROM_PRG_RAM_START && address <= MAPPER_NROM_PRG_RAM_END) {
		u16 prgRamSize = cart->header.prgRamSize + cart->header.prgNVRamSize;
		// ram access
		if (address - MAPPER_NROM_PRG_RAM_START < prgRamSize) {
			return (cart->header.hasNV) ? cart->PRGRam[address] : cart->PRGNVRam[address];
		} else {
			// mirroring
			return NROM_ReadMemCPU(cart, MAPPER_NROM_PRG_RAM_END + (address - MAPPER_NROM_PRG_RAM_END) % prgRamSize);
		}
	} else if (address < MAPPER_NROM_PRG_ROM_START + cart->header.prgRomSize) {
		return cart->PRGRom[address - MAPPER_NROM_PRG_ROM_START];
		// it's always gonna fit in a u16, since it's never more than 0x8000 in that context (NROM)
	} else if (IsAddrInPrgROMMirror(((uint32_t)(address)), cart->header.prgRomSize)) {
		return NROM_ReadMemCPU(cart, GetPrgROMMirrorAddr(address, cart->header.prgRomSize));
	}
	// should never happen
	return 0;
}

void NROM_WriteMemCPU(Cartridge* cart, u16 address, u8 value) {
	if (address >= MAPPER_NROM_PRG_RAM_START && address < MAPPER_NROM_PRG_ROM_START) {
		u16 prgRamSize = cart->header.prgRamSize + cart->header.prgNVRamSize;
		if (address - MAPPER_NROM_PRG_RAM_START < prgRamSize) {
			if (cart->header.hasNV)
				cart->PRGRam[address - MAPPER_NROM_PRG_RAM_START] = value;
			else
				cart->PRGNVRam[address - MAPPER_NROM_PRG_RAM_START] = value;
		} else {
			// mirroring
			NROM_WriteMemCPU(cart, MAPPER_NROM_PRG_RAM_END + (address - MAPPER_NROM_PRG_RAM_END) % prgRamSize, value);
		}
	}
}

#define MAPPER_NROM_CHR_ROM_START 0x0000
#define MAPPER_NROM_CHR_ROM_END 0x1FFF

u8 NROM_ReadMemPPU(Cartridge* cart, u16 address) {
	if (address <= MAPPER_NROM_CHR_ROM_END) {
		if (cart->header.chrRamSize > 0) return cart->CHRRam[address];
		if (cart->header.chrNVRamSize > 0) return cart->CHRNVRam[address];
		return cart->CHRRom[address];
	}
	// should never happen
	// there might be open bus behaviour somewhere in there, unsure
	return 0;
}

// literally does nothing, no RAM to write to
void NROM_WriteMemPPU(Cartridge* cart, u16 address, u8 value) {
	if (address <= MAPPER_NROM_CHR_ROM_END) {
		if (cart->header.chrRamSize > 0) cart->CHRRam[address] = value;
		if (cart->header.chrNVRamSize > 0) cart->CHRNVRam[address] = value;
	}
}

//-------------------------------------- Mapper 1: MAPPER_MMC1  --------------------------------------------

#define MAPPER_MMC1_PRG_RAM_START 0x6000
#define MAPPER_MMC1_PRG_RAM_END 0x7FFF
#define MAPPER_MMC1_PRG_ROM_START 0x8000
#define MAPPER_MMC1_PRG_ROM_MID 0xC000
#define MAPPER_MMC1_PRG_ROM_END 0xFFFF

#define MAPPER_MMC1_CHR_BANK0_START 0x0000
#define MAPPER_MMC1_CHR_BANK0_END 0x0FFF
#define MAPPER_MMC1_CHR_BANK1_START 0x1000
#define MAPPER_MMC1_CHR_BANK1_END 0x1FFF

#define MAPPER_MMC1_SERIAL_REGISTER_START 0x8000
#define MAPPER_MMC1_SERIAL_REGISTER_END 0xFFFF

#define MAPPER_MMC1_WRITETO_CONTROL_START 0x8000
#define MAPPER_MMC1_WRITETO_CONTROL_END 0x9FFF
#define MAPPER_MMC1_WRITETO_CHRBANK0_START 0xA000
#define MAPPER_MMC1_WRITETO_CHRBANK0_END 0xBFFF
#define MAPPER_MMC1_WRITETO_CHRBANK1_START 0xC000
#define MAPPER_MMC1_WRITETO_CHRBANK1_END 0xDFFF
#define MAPPER_MMC1_WRITETO_PRGBANK_START 0xE000
#define MAPPER_MMC1_WRITETO_PRGBANK_END 0xFFFF

#define MAPPER_MMC1_SERIALREGISTER_MASK 0b11111

#define MAPPER_MMC1_SERIAL_DATA_FLAG 0
#define MAPPER_MMC1_SERIAL_RESET_FLAG 7

// on the 5th write to the shift register, it will dump its contents to one of the registers of the MMC1

// Control ($8000-$9FFF)
// CPPMM
// C = CHR-ROM bank mode (0: switch 8KB at a time; 1: switch 2 separate 4KB banks)
// P = PRG-ROM bank mode (0, 1: switch 32KB at $8000, ignoring low bit of bank number; 2: fix first bank at $8000 and switch 16KB bank at $C000;
//                        3: fix last bank at $C000 and switch 16KB bank at $8000)
// M = Nametable arrangement (0: one screen, lower bank; 1: one screen, upper bank; 2: horizontal arrangement ("vertical mirroring"); 3: vertical
// arrangement ("horizontal mirroring"))

// CHR Bank 0 ($A000-$BFFF)
// CCCCC
// C = Select 4KB or 8KB CHR bank at PPU $0000 (low bit ignored in 8KB mode)

// CHR Bank 1 ($A000-$BFFF)
// CCCCC
// C = Select 4KB or 8KB CHR bank at PPU $1000 (ignored in 8KB mode)

// PRG Bank ($E000-$FFFF)
// RPPPP
// R =
// MMC1A -> 0: fixed bank affects A17..A14, 1; fixed bank only affects A16..A14, bit 3 directly controls A17 across the entire $8000-$FFFF
// address range MMC1B -> 0: PRG-RAM enabled, 1: PRG-RAM disabled

void MMC1_Init(Cartridge* cart) {
	cart->mapper.mmc1.shiftCnt = 0;
	cart->mapper.mmc1.shiftRegister = 0;
	// mapper mmc1 never inits to PRG ROM bank mode 2, let's force it to 3
	cart->mapper.mmc1.controlReg = (cart->mapper.mmc1.controlReg & 0b100111) | 0b11 << MMC1_CONTROL_PRGBANK_ORIG;

	// i'm really forced to do this because of Dr. Mario? -_-
	cart->mapper.mmc1.prgBankReg = 0;
}

// TODO: need to test this, Zelda 1 would prolly be fine
u8 MMC1_ReadMemCPU(Cartridge* cart, u16 address) {
	if (address >= MAPPER_MMC1_PRG_RAM_START && address <= MAPPER_MMC1_PRG_RAM_END) {
		u16 prgRamSize = cart->header.prgRamSize + cart->header.prgNVRamSize;
		// ram access
		if (address - MAPPER_NROM_PRG_RAM_START < prgRamSize) {
			return (cart->header.hasNV) ? cart->PRGRam[address] : cart->PRGNVRam[address];
		} else {
			// mirroring
			return MMC1_ReadMemCPU(cart, MAPPER_NROM_PRG_RAM_END + (address - MAPPER_NROM_PRG_RAM_END) % prgRamSize);
		}
	} else if (address >= MAPPER_MMC1_SERIAL_REGISTER_START) {
		switch (GetBits(cart->mapper.mmc1.controlReg, MMC1_CONTROL_PRGBANK_ORIG, MMC1_CONTROL_PRGBANK_SIZE)) {
			case MMC1_CONTROL_PRGBANK_ONEBANK:
			case MMC1_CONTROL_PRGBANK_ONEBANK_AGAIN: {
				// get nth doublebank
				return cart->PRGRom[address - MAPPER_MMC1_PRG_ROM_START + (cart->mapper.mmc1.prgBankReg >> 1) * MMC1_ONEBANK_OFFSET];
			}

			case MMC1_CONTROL_PRGBANK_FIX_FIRST: {
				if (address - MAPPER_MMC1_PRG_ROM_START < MMC1_FIXED_OFFSET) {
					// get first bank
					return cart->PRGRom[address - MAPPER_MMC1_PRG_ROM_START];
				}
				// get nth bank
				return cart->PRGRom[address - MAPPER_MMC1_PRG_ROM_MID + cart->mapper.mmc1.prgBankReg * MMC1_FIXED_OFFSET];
			}
			case MMC1_CONTROL_PRGBANK_FIX_LAST: {
				if (address - MAPPER_MMC1_PRG_ROM_START < MMC1_FIXED_OFFSET) {
					// get nth bank
					return cart->PRGRom[address - MAPPER_MMC1_PRG_ROM_START + cart->mapper.mmc1.prgBankReg * MMC1_FIXED_OFFSET];
				}
				// get last bank
				return cart->PRGRom[address - MAPPER_MMC1_PRG_ROM_MID + ((cart->header.prgRomSize / PRG_ROM_PAGE_SIZE) - 1) * MMC1_FIXED_OFFSET];
			}
		}
	}
	// should never happen
	return 0;
}

// TODO: fix to add compatibility with these stupid variants
// TODO: implement PRG-RAM locking
void MMC1_WriteMemCPU(Cartridge* cart, u16 address, u8 value) {
	MMC1Mapper* mapperData = &cart->mapper.mmc1;
	if (address >= MAPPER_MMC1_PRG_RAM_START && address <= MAPPER_MMC1_PRG_RAM_END) {
		u16 prgRamSize = cart->header.prgRamSize + cart->header.prgNVRamSize;
		if (address - MAPPER_NROM_PRG_RAM_START < prgRamSize) {
			cart->PRGRam[address - MAPPER_NROM_PRG_RAM_START] = value;
		} else {
			// mirroring
			NROM_WriteMemCPU(cart, MAPPER_NROM_PRG_RAM_END + (address - MAPPER_NROM_PRG_RAM_END) % prgRamSize, value);
		}
	} else if (address >= MAPPER_MMC1_SERIAL_REGISTER_START) {
		if (GetFlag(value, 7)) {
			mapperData->shiftCnt = 0;
			mapperData->shiftRegister = 0;
			mapperData->controlReg |= 0xC;
		} else {
			mapperData->shiftRegister = (cart->mapper.mmc1.shiftRegister >> 1) | ((value & 0b1) << 4);
			mapperData->shiftCnt++;

			if (mapperData->shiftCnt >= 5) {
				u8 valToWrite = mapperData->shiftRegister & MAPPER_MMC1_SERIALREGISTER_MASK;
				mapperData->shiftCnt = 0;
				mapperData->shiftRegister = 0;
				if (address >= MAPPER_MMC1_WRITETO_CONTROL_START && address <= MAPPER_MMC1_WRITETO_CONTROL_END) {
					mapperData->controlReg = valToWrite;
				} else if (address >= MAPPER_MMC1_WRITETO_CHRBANK0_START && address <= MAPPER_MMC1_WRITETO_CHRBANK0_END) {
					mapperData->chrBank0Reg = valToWrite;
				} else if (address >= MAPPER_MMC1_WRITETO_CHRBANK1_START && address <= MAPPER_MMC1_WRITETO_CHRBANK1_START) {
					mapperData->chrBank1Reg = valToWrite;
				} else if (address >= MAPPER_MMC1_WRITETO_PRGBANK_START && address) {
					mapperData->prgBankReg = valToWrite;
				}
				// if none of those if statements are triggered, something wrong happened
			}
		}
	}
}

u8 MMC1_ReadMemPPU(Cartridge* cart, u16 address) {
	MMC1Mapper* mapperData = &cart->mapper.mmc1;
	u8 bank0;
	u8 bank1;

	if (!GetFlag(mapperData->controlReg, MMC1_CONTROL_CHRBANK_BIT)) {
		bank0 = (mapperData->chrBank0Reg & 0b11110);
		bank1 = (mapperData->chrBank0Reg & 0b11110) | 1;
	} else {
		bank0 = mapperData->chrBank0Reg;
		bank1 = mapperData->chrBank1Reg;
	}

	if (address <= MAPPER_MMC1_CHR_BANK0_END) {
		if (cart->CHRRom) {
			return cart->CHRRom[bank0 << 12 | address];
		} else if (cart->CHRRam) {
			return cart->CHRRam[bank0 << 12 | address];
		} else if (cart->CHRNVRam) {
			return cart->CHRNVRam[bank0 << 12 | address];
		}
	} else if (address <= MAPPER_MMC1_CHR_BANK1_END) {
		if (cart->CHRRom) {
			return cart->CHRRom[bank1 << 12 | (address - 0x1000)];
		} else if (cart->CHRRam) {
			return cart->CHRRam[bank1 << 12 | (address - 0x1000)];
		} else if (cart->CHRNVRam) {
			return cart->CHRNVRam[bank1 << 12 | (address - 0x1000)];
		}
	}
	// should never happen
	return 0;
}

void MMC1_WriteMemPPU(Cartridge* cart, u16 address, u8 value) {
	if (cart->CHRRom) return;
	MMC1Mapper* mapperData = &cart->mapper.mmc1;
	u8 bank0;
	u8 bank1;

	if (GetFlag(mapperData->controlReg, MMC1_CONTROL_CHRBANK_BIT)) {
		bank0 = (mapperData->chrBank0Reg & 0b11110);
		bank1 = (mapperData->chrBank0Reg & 0b11110) | 1;
	} else {
		bank0 = mapperData->chrBank0Reg;
		bank1 = mapperData->chrBank1Reg;
	}

	if (address <= MAPPER_MMC1_CHR_BANK0_END) {
		if (cart->CHRRam) {
			cart->CHRRam[bank0 << 12 | address] = value;
		} else if (cart->CHRNVRam) {
			cart->CHRNVRam[bank0 << 12 | address] = value;
		}
	} else if (address <= MAPPER_MMC1_CHR_BANK1_END) {
		if (cart->CHRRam) {
			cart->CHRRam[bank1 << 12 | address] = value;
		} else if (cart->CHRNVRam) {
			cart->CHRNVRam[bank1 << 12 | address] = value;
		}
	}
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

#include "Cartridge.h"

// no bus conflict emulation due to lack of documentation for now

static u8 UXROM_TruncateReg(Cartridge* cart, u8 value) {
	u8 bankCount = (cart->header.prgRomSize / PRG_ROM_PAGE_SIZE);
	u8 bankMask = bankCount - 1;
	if (value >= bankCount) return value & bankMask;
	return value;
}

u8 UXROM_ReadMemCPU(Cartridge* cart, u16 address) {
	UXROMMapper* mapperData = &cart->mapper.uxrom;
	if (address >= MAPPER_UXROM_PRG_ROM_BANKED_START && address <= MAPPER_UXROM_PRG_ROM_BANKED_END) {
		return cart->PRGRom[address + PRG_ROM_PAGE_SIZE * mapperData->PRGBank];
	} else if (address >= MAPPER_UXROM_PRG_ROM_FIXED_START) {
		// reads last bank of PRG ROM
		return cart->PRGRom[address + cart->header.prgRomSize - PRG_ROM_PAGE_SIZE];
	}
	// should never happen
	return 0;
}

void UXROM_WriteMemCPU(Cartridge* cart, u16 address, u8 value) {
	UXROMMapper* mapperData = &cart->mapper.uxrom;
	if (address >= MAPPER_UXROM_PRG_BANK_REG_START) mapperData->PRGBank = UXROM_TruncateReg(cart, value);
}

// TODO: implement PPU Bus functions for UxROM

//-------------------------------------- Mapper 3: MAPPER_CNROM  -------------------------------------------
u8 CNROM_ReadMemCPU([[maybe_unused]] Cartridge* cart, [[maybe_unused]] u16 address) {
	// WIP
	return 0;
}
void CNROM_WriteMemCPU([[maybe_unused]] Cartridge* cart, [[maybe_unused]] u16 address, [[maybe_unused]] u8 value) {
	// WIP
}

// TODO: implement PPU Bus functions for CNROM

//----------------------------------------------------------------------------------------------------------

typedef void (*MapperInitCb)(Cartridge*);
void MapperInit(Cartridge* cart) {
	MapperInitCb func = NULL;
	switch (cart->mapper.id) {
		case MAPPER_MMC1:
			func = MMC1_Init;
			break;
			// case MAPPER_UXROM: func = UXROM_Init; break;
			// case MAPPER_CNROM: func = CNROM_Init; break;

		default:
	}
	if (func) func(cart);
}

typedef u8 (*MapperReadMemCPUCb)(Cartridge*, u16);

u8 MapperReadMemCPU(Cartridge* cart, u16 address) {
	MapperReadMemCPUCb func = NULL;
	switch (cart->mapper.id) {
		case MAPPER_NROM: func = NROM_ReadMemCPU; break;
		case MAPPER_MMC1: func = MMC1_ReadMemCPU; break;
		case MAPPER_UXROM: func = UXROM_ReadMemCPU; break;
		case MAPPER_CNROM: func = CNROM_ReadMemCPU; break;

		default:
	}
	if (func) return func(cart, address);
	return 0;
}

typedef void (*MapperWriteMemCPUCb)(Cartridge*, u16, u8);
void MapperWriteMemCPU(Cartridge* cart, u16 address, u8 value) {
	MapperWriteMemCPUCb func = NULL;
	switch (cart->mapper.id) {
		case MAPPER_NROM: func = NROM_WriteMemCPU; break;
		case MAPPER_MMC1: func = MMC1_WriteMemCPU; break;
		case MAPPER_UXROM: func = UXROM_WriteMemCPU; break;
		case MAPPER_CNROM: func = CNROM_WriteMemCPU; break;

		default:
	}
	if (func) func(cart, address, value);
}

typedef u8 (*MapperReadMemPPUCb)(Cartridge*, u16);

u8 MapperReadMemPPU(Cartridge* cart, u16 address) {
	MapperReadMemPPUCb func = NULL;
	switch (cart->mapper.id) {
		case MAPPER_NROM: func = NROM_ReadMemPPU; break;
		case MAPPER_MMC1: func = MMC1_ReadMemPPU; break;

		default:
	}
	if (func) return func(cart, address);
	return 0;
}

typedef void (*MapperWriteMemPPUCb)(Cartridge*, u16, u8);

void MapperWriteMemPPU(Cartridge* cart, u16 address, u8 value) {
	MapperWriteMemPPUCb func = NULL;
	switch (cart->mapper.id) {
		case MAPPER_NROM: func = NROM_WriteMemPPU; break;
		case MAPPER_MMC1: func = MMC1_WriteMemPPU; break;

		default:
	}
	if (func) func(cart, address, value);
}