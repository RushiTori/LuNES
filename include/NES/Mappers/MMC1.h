#ifndef LU_NES_MMC1_H
#define LU_NES_MMC1_H

#include "Byte.h"

// on the 5th write to the shift register, it will dump its contents to one of the registers of the MMC1

// Shift Register ($8000-$FFFF)
// Rxxx xxxD
// R = Reset shift register and Control = Control OR $0C
// D = Data bit to be shifted in, LSB first

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

// registers are only 5 bits
#define MMC1_CONTROL_NAMETABLE_ORIG 0
#define MMC1_CONTROL_PRGBANK_ORIG 2
#define MMC1_CONTROL_NAMETABLE_SIZE 2
#define MMC1_CONTROL_PRGBANK_SIZE 2
#define MMC1_CONTROL_CHRBANK_BIT 4

#define MMC1_CONTROL_PRGBANK_ONEBANK 0b00
#define MMC1_CONTROL_PRGBANK_ONEBANK_AGAIN 0b01
#define MMC1_CONTROL_PRGBANK_FIX_FIRST 0b10
#define MMC1_CONTROL_PRGBANK_FIX_LAST 0b11

#define MMC1_ONEBANK_OFFSET 0x8000
#define MMC1_FIXED_OFFSET 0x4000

typedef struct MMC1Mapper {
	u8 shiftRegister;
	u8 shiftCnt;

	u8 controlReg;

	u8 chrBank0Reg;
	u8 chrBank1Reg;

	u8 prgBankReg;
} MMC1Mapper;

#endif	// LU_NES_UXROM_H
