#ifndef LU_NES_CPU_H
#define LU_NES_CPU_H

#include "CPUMemory.h"
#include "Operations.h"

#define CPU_FLAG_C 0
#define CPU_FLAG_Z 1
#define CPU_FLAG_I 2
#define CPU_FLAG_D 3
#define CPU_FLAG_B 4
#define CPU_FLAG_5 5  // bit 5 is supposed to always be 1
#define CPU_FLAG_V 6
#define CPU_FLAG_N 7

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

void CPUInit(CPU* cpu);

uint64_t CPUStep(CPU* cpu);

void CPUStackPush(CPU* cpu, u8 value);
void CPUStackPush16(CPU* cpu, u16 value);
u8 CPUStackPop(CPU* cpu);
u16 CPUStackPop16(CPU* cpu);

uint64_t CPUInterrupt(CPU* cpu, InterruptID interrupt, bool pushB);

#endif	// LU_NES_CPU_H
