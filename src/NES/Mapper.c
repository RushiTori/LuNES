#include "Mapper.h"

#include <math.h>

#include "Cartridge.h"

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

// NROM doesn't need init

// TODO: fix reading so that it doesn't rely on a cartridge's PRG-ROM that's at least 16KB in size
u8 NROM_ReadMemCPU(Cartridge* cart, u16 address) {
	// doesn't use mapper since mapper is literally empty
	if (address >= MAPPER_NROM_PRG_RAM_START && address <= MAPPER_NROM_PRG_RAM_END) {
		u16 prgRamSize = 64 << cart->header.prgRamShifts + 64 << cart->header.prgNVRamShifts;
		// ram access
		if (address - MAPPER_NROM_PRG_RAM_START < prgRamSize) {
			return (cart->header.hasPRGRam) ? cart->PRGRam[address] : 0;
		} else {
			// mirroring
			return NROM_ReadMem(cart, MAPPER_NROM_PRG_RAM_END + (address - MAPPER_NROM_PRG_RAM_END) % prgRamSize);
		}
	} else if (address < MAPPER_NROM_PRG_ROM_START + cart->header.prgRomPages * PRG_ROM_PAGE_SIZE) {
		return cart->PRGRom[address - MAPPER_NROM_PRG_ROM_START];
	} else if (IsAddrInPrgROMMirror(address, cart->header.prgRomPages)) {
		return NROMReadMem(cart, GetPrgROMMirrorAddr(address, cart->header.prgRomPages));
	}
	// should never happen
	return 0;
}

void NROM_WriteMemCPU(Cartridge* cart, u16 address, u8 value) {
	if (address >= MAPPER_NROM_PRG_RAM_START && address < MAPPER_NROM_PRG_ROM_START) {
		u16 prgRamSize = 64 << cart->header.prgRamShifts + 64 << cart->header.prgNVRamShifts;
		if (address - MAPPER_NROM_PRG_RAM_START < prgRamSize) {
			cart->PRGRam[address - MAPPER_NROM_PRG_RAM_START] = value;
		} else {
			// mirroring
			NROM_WriteMem(cart, MAPPER_NROM_PRG_RAM_END + (address - MAPPER_NROM_PRG_RAM_END) % prgRamSize, value);
		}
	}
}

#define MAPPER_NROM_CHR_ROM_START 0x0000
#define MAPPER_NROM_CHR_ROM_END 0x1FFF

u8 NROM_ReadMemPPU(Cartridge* cart, u16 address) {
	if (address >= MAPPER_NROM_CHR_ROM_START && address <= MAPPER_NROM_CHR_ROM_END) {
		if (cart->header.chrRamSize > 0) return cart->CHRRam[address];
		if (cart->header.chrNVRamShifts > 0) return cart->CHRNVRam[address];
		return cart->CHRRom[address];
	}
	// should never happen
	// there might be open bus behaviour somewhere in there, unsure
	return 0;
}

// literally does nothing, no RAM to write to
void NROM_WriteMemPPU(Cartridge* cart, u16 address, u8 value) {
	if (address >= MAPPER_NROM_CHR_ROM_START && address <= MAPPER_NROM_CHR_ROM_END) {
		if (cart->header.chrRamSize > 0) cart->CHRRam[address] = value;
		if (cart->header.chrNVRamShifts > 0) cart->CHRNVRam[address] = value;
	}
}

//-------------------------------------- Mapper 1: MAPPER_MMC1  --------------------------------------------

#define MAPPER_MMC1_PRG_ROM_START 0x8000
#define MAPPER_MMC1_PRG_ROM_MID 0xC000
#define MAPPER_MMC1_PRG_ROM_END 0xFFFF

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

// todo: add PRG RAM support
u8 MMC1_ReadMemCPU(Cartridge* cart, u16 address) {
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
			return cart->PRGRom[address - MAPPER_MMC1_PRG_ROM_START + cart->mapper.mmc1.prgBankReg * MMC1_FIXED_OFFSET];
		}
		case MMC1_CONTROL_PRGBANK_FIX_LAST: {
			if (address - MAPPER_MMC1_PRG_ROM_START < MMC1_FIXED_OFFSET) {
				// get nth bank
				return cart->PRGRom[address - MAPPER_MMC1_PRG_ROM_START + cart->mapper.mmc1.prgBankReg * MMC1_FIXED_OFFSET];
			}
			// get last bank
			return cart->PRGRom[address - MAPPER_MMC1_PRG_ROM_START + (cart->header.prgRomPages - 1) * MMC1_FIXED_OFFSET];
		}
	}
}

// todo: add PRG RAM support
// todo: fix to add compatibility with these stupid variants
void MMC1_WriteMemCPU(Cartridge* cart, u16 address, u8 value) {
	MMC1Mapper* mapperData = &cart->mapper.mmc1;
	if (address >= MAPPER_MMC1_SERIAL_REGISTER_START) {
		if (GetFlag(value, 7)) {
			mapperData->shiftCnt = 0;
			mapperData->shiftRegister = 0;
			mapperData->controlReg |= 0xC;
		} else {
			mapperData->shiftRegister = (cart->mapper.mmc1.shiftRegister << 1) | (value & 0b1);
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
				} else if (address >= MAPPER_MMC1_WRITETO_PRGBANK_START && address <= MAPPER_MMC1_WRITETO_PRGBANK_END) {
					mapperData->prgBankReg = valToWrite;
				}
				// if none of those if statements are triggered, something wrong happened
			}
		}
	}
}

// TODO: implement PPU Bus functions for MMC1

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
	u8 bankCount = cart->header.prgRomPages;
	u8 bankMask = bankCount - 1;
	if (value >= bankCount) return value & bankMask;
	return value;
}

u8 UXROM_ReadMemCPU(Cartridge* cart, u16 address) {
	UXROMMapper* mapperData = &cart->mapper.uxrom;
	if (address >= MAPPER_UXROM_PRG_ROM_BANKED_START && address <= MAPPER_UXROM_PRG_ROM_BANKED_END) {
		return cart->PRGRom[address + 0x4000 * mapperData->PRGBank];
	} else if (address >= MAPPER_UXROM_PRG_ROM_FIXED_START) {
		// reads last bank of PRG ROM
		return cart->PRGRom[address + 0x4000 * (cart->header.prgRomPages - 1)];
	}
}

void UXROM_WriteMemCPU(Cartridge* cart, u16 address, u8 value) {
	UXROMMapper* mapperData = &cart->mapper.uxrom;
	if (address >= MAPPER_UXROM_PRG_BANK_REG_START) mapperData->PRGBank = UXROMTruncateReg(cart, value);
}

// TODO: implement PPU Bus functions for UxROM

//-------------------------------------- Mapper 3: MAPPER_CNROM  -------------------------------------------
u8 CNROM_ReadMemCPU(Cartridge* cart, u16 address) {
	// WIP
}
void CNROM_WriteMemCPU(Cartridge* cart, u16 address, u8 value) {
	// WIP
}

// TODO: implement PPU Bus functions for CNROM

//----------------------------------------------------------------------------------------------------------

typedef void (*MapperInitCb)(Cartridge*);
void MapperInit(Cartridge* cart) {
	MapperInitCb func = NULL;
	switch (cart->mapper.id) {
		case MAPPER_MMC1: func = MMC1_Init; break;
		case MAPPER_UXROM: func = UXROM_ReadMemCPU; break;
		case MAPPER_CNROM: func = CNROM_ReadMemCPU; break;

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
		case MAPPER_NROM: func = NROM_ReadMemPPU;

		default:
	}
	if (func) func(cart, address);
}

typedef u8 (*MapperWriteMemPPUCb)(Cartridge*, u16, u8);

void MapperWriteMemPPU(Cartridge* cart, u16 address, u8 value) {
	MapperWriteMemPPUCb func = NULL;
	switch (cart->mapper.id) {
		case MAPPER_NROM: func = NROM_WriteMemPPU;

		default:
	}
	if (func) func(cart, address, value);
}