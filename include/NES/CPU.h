#ifndef LU_NES_CPU_H
#define LU_NES_CPU_H

#include "CPUMemory.h"

typedef struct CPU {
	CPUram memory;

	u16 pc;

	u8 a;
	u8 x;
	u8 y;

	u8 flags;
	u8 sp;

	u8 extraCycles;
	uint64_t cycles;
} CPU;

#endif	// LU_NES_CPU_H
