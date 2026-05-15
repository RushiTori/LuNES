#ifndef LU_NES_ADDRESSING_MODES_H
#define LU_NES_ADDRESSING_MODES_H

#include "Byte.h"

typedef struct CPU CPU;

typedef enum AddressingMode {
	ADDRESSING_IMPLIED,		 // impl
	ADDRESSING_IMMEDIATE,	 // #
	ADDRESSING_RELATIVE,	 // rel
	ADDRESSING_ZERO_PAGE,	 // zpg
	ADDRESSING_ZERO_PAGE_X,	 // zpg,X
	ADDRESSING_ZERO_PAGE_Y,	 // zpg,Y
	ADDRESSING_ABSOLUTE,	 // abs
	ADDRESSING_ABSOLUTE_X,	 // abs,X
	ADDRESSING_ABSOLUTE_Y,	 // abs,Y
	ADDRESSING_INDIRECT,	 // ind
	ADDRESSING_INDIRECT_X,	 // X,ind
	ADDRESSING_INDIRECT_Y,	 // ind,Y

	ADDRESSING_MODE_COUNT,

	ADDRESSING_A = ADDRESSING_IMPLIED,	// A
} AddressingMode;

u8 AddressingModeGetInputSize(AddressingMode mode);
u16 AddressingModeGetAddress(AddressingMode mode, CPU* cpu, u16 addr, bool hasPenalty);
u8 AddressingModeGetValue(AddressingMode mode, CPU* cpu, u16 value, bool hasPenalty);

#endif	// LU_NES_ADDRESSING_MODES_H
