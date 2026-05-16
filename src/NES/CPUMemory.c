
#include "CPUMemory.h"

/*
$0000 - $07FF: 2KB of internal Work RAM
$0800 - $1FFF: Mirrors of $0000 - $07FF (Work RAM)
$2000 - $2007: PPU Registers
$2008 - $3FFF: Mirrors of $2000 - $2007 (PPU Registers)
$4000 - $4017: APU and I/O Registers
$4018 - $401F: APU and I/O Functionality that is normally disabled, see the latest section for more information
$4020 - $FFFF: Unmapped by default, available for cartridge space
$6000 - $7FFF: tends to be cartridge RAM
$8000 - $FFFF: tends to be cartridge ROM on read, and mapper registers if present on write
*/

#define CPU_RAM_MIRROR_START 0x0000
#define CPU_RAM_MIRROR_FIRST 0x0800
#define CPU_RAM_MIRROR_LAST 0x1F00
#define CPU_RAM_MIRROR_RANGE 0x0800

#define PPU_REG_MIRROR_START 0x2000
#define PPU_REG_MIRROR_FIRST 0x2008
#define PPU_REG_MIRROR_LAST 0x3FFF
#define PPU_REG_MIRROR_RANGE 0x0008

#define IsAddressInRange(addr, first, last) (((addr) >= (first)) && ((addr) <= (last)))
#define GetMirroredAddress(addr, first, range, start) ((start) + (((addr) - (first)) % (range)))

#define IsAddrInRamMirror(addr) IsAddressInRange((addr), CPU_RAM_MIRROR_FIRST, CPU_RAM_MIRROR_LAST)
#define GetRamMirrorAddr(addr) GetMirroredAddress((addr), CPU_RAM_MIRROR_FIRST, CPU_RAM_MIRROR_RANGE, CPU_RAM_MIRROR_START)

#define IsAddrInPPURegMirror(addr) IsAddressInRange((addr), PPU_REG_MIRROR_FIRST, PPU_REG_MIRROR_LAST)
#define GetPPURegMirrorAddr(addr) GetMirroredAddress((addr), PPU_REG_MIRROR_FIRST, PPU_REG_MIRROR_RANGE, PPU_REG_MIRROR_START)

u8 CPUMemRead(const CPUram ram, u16 addr) {
	// Handling the mirrors first
	if (IsAddrInRamMirror(addr)) return CPUMemRead(ram, GetRamMirrorAddr(addr));
	if (IsAddrInPPURegMirror(addr)) return CPUMemRead(ram, GetPPURegMirrorAddr(addr));

	if (addr < CPU_RAM_SIZE) return ram[addr];

	// WIP: finish implementing the ranges
	return 0xFF;
}

void CPUMemWrite(CPUram ram, u16 addr, u8 value) {
	// Handling the mirrors first
	if (IsAddrInRamMirror(addr)) return CPUMemWrite(ram, GetRamMirrorAddr(addr), value);
	if (IsAddrInPPURegMirror(addr)) return CPUMemWrite(ram, GetPPURegMirrorAddr(addr), value);

	if (addr < CPU_RAM_SIZE) ram[addr] = value;

	// WIP: finish implementing the ranges
}

u16 CPUMemRead16(const CPUram ram, u16 addr) {
	u8 lowByte = CPUMemRead(ram, addr);
	u8 highByte = CPUMemRead(ram, addr + 1);

	return MakeWord(highByte, lowByte);
}

void CPUMemWrite16(CPUram ram, u16 addr, u16 value) {
	u8 lowByte = GetLowByte(value);
	u8 highByte = GetHighByte(value);

	CPUMemWrite(ram, addr, lowByte);
	CPUMemWrite(ram, addr + 1, highByte);
}

u16 CPUMemReadPage16(const CPUram ram, u8 page, u8 addr) {
	u16 lowAddr = MakeWord(page, addr);
	u16 highAddr = MakeWord(page, addr + 1);

	u8 lowByte = CPUMemRead(ram, lowAddr);
	u8 highByte = CPUMemRead(ram, highAddr);

	return MakeWord(highByte, lowByte);
}

void CPUMemWritePage16(CPUram ram, u8 page, u8 addr, u16 value) {
	u16 lowAddr = MakeWord(page, addr);
	u16 highAddr = MakeWord(page, addr + 1);

	u8 lowByte = GetLowByte(value);
	u8 highByte = GetHighByte(value);

	CPUMemWrite(ram, lowAddr, lowByte);
	CPUMemWrite(ram, highAddr, highByte);
}
