#ifndef LU_NES_CPU_MEMORY_H
#define LU_NES_CPU_MEMORY_H

#include "Byte.h"

#define CPU_RAM_SIZE 0x0800	 // 2048 bytes or 2KiB

typedef u8 CPUram[CPU_RAM_SIZE];

u8 CPUMemRead(const CPUram ram, u16 addr);
void CPUMemWrite(CPUram ram, u16 addr, u8 value);

u16 CPUMemRead16(const CPUram ram, u16 addr);
void CPUMemWrite16(CPUram ram, u16 addr, u16 value);

u16 CPUMemReadPage16(const CPUram ram, u8 page, u8 addr);
void CPUMemWritePage16(CPUram ram, u8 page, u8 addr, u16 value);

#endif	// LU_NES_CPU_MEMORY_H
