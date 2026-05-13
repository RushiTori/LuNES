#include "CPU.h"

u16 GetInterruptVector(InterruptID interrupt) {
	switch (interrupt) {
		default:
		case CPU_INT_RESET: return 0XFFFC;
		case CPU_INT_NMI: return 0XFFFA;
		case CPU_INT_IRQ: return 0XFFFE;
		case CPU_INT_BRK: return 0XFFFE;
	}
}

void CPUInit(CPU* cpu) { *cpu = (CPU){0}; }

uint64_t CPUStep(CPU* cpu) {
	// WIP: implement the step functions
}

static void CPUStackPush(CPU* cpu, u8 value) {
	u16 realAddr = 0x0100 + cpu->sp;
	CPUMemWrite(cpu->memory, realAddr, value);
	cpu->sp--;
}

static void CPUStackPush16(CPU* cpu, u16 value) {
	CPUStackPush(cpu, GetHighByte(value));
	CPUStackPush(cpu, GetLowByte(value));
}

static u8 CPUStackPop(CPU* cpu) {
	cpu->sp++;
	u16 realAddr = 0x0100 + cpu->sp;
	return CPUMemRead(cpu->memory, realAddr);
}

static u16 CPUStackPop16(CPU* cpu) {
	u8 low = CPUStackPop(cpu);
	u8 high = CPUStackPop(cpu);
	return MakeWord(high, low);
}

uint64_t CPUInterrupt(CPU* cpu, InterruptID interrupt, bool pushB) {
	if (interrupt == CPU_INT_IRQ && GetFlag(cpu->flags, CPU_FLAG_I)) return 0;

	CPUStackPush16(cpu, cpu->pc);
	CPUStackPush(cpu, AssignBit(cpu->flags, 4, pushB));

	cpu->cycles += 7;
	cpu->flags = SetBit(cpu->flags, CPU_FLAG_I);
	cpu->pc = CPUMemRead16(cpu->memory, GetInterruptVector(interrupt));

	return 7;
}
