#include "AddressingMode.h"

u8 AddressingModeGetInputSize(AddressingMode mode) {
	switch (mode) {
		case ADDRESSING_IMPLIED: return 0;
		case ADDRESSING_IMMEDIATE: return 1;
		case ADDRESSING_ABSOLUTE: return 2;
		case ADDRESSING_ZERO_PAGE: return 1;
		case ADDRESSING_RELATIVE: return 1;
		case ADDRESSING_INDIRECT: return 2;
		case ADDRESSING_ZERO_PAGE_X: return 1;
		case ADDRESSING_ZERO_PAGE_Y: return 1;
		case ADDRESSING_ABSOLUTE_X: return 2;
		case ADDRESSING_ABSOLUTE_Y: return 2;
		case ADDRESSING_INDIRECT_X: return 1;
		case ADDRESSING_INDIRECT_Y: return 1;
		default: break;
	}

	return 0;
}

static u16 BuildAddress(CPU* cpu, u16 base, u8 offset, bool hasPenalty) {
	u16 addr = base + offset;

	u8 baseHigh = GetHighByte(base);
	u8 addrHigh = GetHighByte(addr);
	if (hasPenalty && (baseHigh != addrHigh)) cpu->extraCycles++;

	return addr;
}

u16 AddressingModeGetAddress(AddressingMode mode, CPU* cpu, u16 addr, bool hasPenalty) {
	if (AddressingModeGetInputSize(mode) == 1) addr = GetLowByte(addr);
	u16 result;

	switch (mode) {
		case ADDRESSING_IMPLIED: result = 0x0000; break;

		default:
		case ADDRESSING_IMMEDIATE: break;  // Unsupported

		case ADDRESSING_ABSOLUTE:
		case ADDRESSING_ZERO_PAGE: result = addr; break;

		case ADDRESSING_INDIRECT: result = CPUMemRead16(cpu->memory, addr); break;

		case ADDRESSING_ZERO_PAGE_X: result = GetLowByte(addr + cpu->x); break;

		case ADDRESSING_ZERO_PAGE_Y: result = GetLowByte(addr + cpu->y); break;

		case ADDRESSING_INDIRECT_X: result = CPUMemReadPage16(cpu->memory, 0, GetLowByte(addr + cpu->x)); break;

		case ADDRESSING_RELATIVE: {
			u16 base = cpu->pc;
			s8 offset = GetLowByte(addr);
			result = BuildAddress(cpu, base, offset, hasPenalty);
		} break;

		case ADDRESSING_ABSOLUTE_X: {
			u16 base = addr;
			u8 offset = cpu->x;
			result = BuildAddress(cpu, base, offset, hasPenalty);
		} break;

		case ADDRESSING_ABSOLUTE_Y: {
			u16 base = addr;
			u8 offset = cpu->y;
			result = BuildAddress(cpu, base, offset, hasPenalty);
		} break;

		case ADDRESSING_INDIRECT_Y: {
			u16 base = CPUMemReadPage16(cpu->memory, 0, GetLowByte(addr));
			u8 offset = cpu->y;
			result = BuildAddress(cpu, base, offset, hasPenalty);
		} break;
	}

	return result;
}

u8 AddressingModeGetValue(AddressingMode mode, CPU* cpu, u16 value, bool hasPenalty) {
	if (AddressingModeGetInputSize(mode) == 1) value = GetLowByte(value);
	u8 result;

	switch (mode) {
		default:
		case ADDRESSING_IMPLIED:
		case ADDRESSING_RELATIVE:
		case ADDRESSING_INDIRECT: break;  // Unsupported

		case ADDRESSING_IMMEDIATE: result = value; break;

		case ADDRESSING_ABSOLUTE:
		case ADDRESSING_ABSOLUTE_X:
		case ADDRESSING_ABSOLUTE_Y:
		case ADDRESSING_ZERO_PAGE:
		case ADDRESSING_ZERO_PAGE_X:
		case ADDRESSING_ZERO_PAGE_Y:
		case ADDRESSING_INDIRECT_X:
		case ADDRESSING_INDIRECT_Y: {
			u16 addr = AddressingModeGetAddress(mode, cpu, value, hasPenalty);
			result = CPUMemRead(cpu->memory, addr);
		} break;
	}

	return result;
}
