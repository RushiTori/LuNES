#include "Instructions.h"

#include "CPU.h"

static void CPUUpdateNZ(CPU* cpu, u8 value) {
	ClearBit(cpu->flags, CPU_FLAG_N);
	ClearBit(cpu->flags, CPU_FLAG_Z);

	if (value & 0x80) SetBit(cpu->flags, CPU_FLAG_N);
	if (value == 0) SetBit(cpu->flags, CPU_FLAG_Z);
}

static void AssignAndUpdateNZ(CPU* cpu, u8* dest, u8 src) {
	*dest = src;
	CPUUpdateNZ(cpu, src);
}

static void MemWriteAndUpdateNZ(CPU* cpu, u16 addr, u8 value) {
	CPUMemWrite(cpu->memory, addr, value);
	CPUUpdateNZ(cpu, value);
}

static u8 LSRbase(CPU* cpu, u8 value) {
	u8 oldCarry = GetBit(cpu->flags, CPU_FLAG_C);
	u8 newCarry = GetBit(value, 0);
	cpu->flags = AssignBit(cpu->flags, CPU_FLAG_C, newCarry);

	return value >> 1;
}

static u8 ASLbase(CPU* cpu, u8 value) {
	u8 oldCarry = GetBit(cpu->flags, CPU_FLAG_C);
	u8 newCarry = GetBit(value, 7);
	cpu->flags = AssignBit(cpu->flags, CPU_FLAG_C, newCarry);

	return value << 1;
}

static u8 ROLbase(CPU* cpu, u8 value) {
	u8 oldCarry = GetBit(cpu->flags, CPU_FLAG_C);
	u8 newCarry = GetBit(value, 7);
	cpu->flags = AssignBit(cpu->flags, CPU_FLAG_C, newCarry);

	value <<= 1;
	return AssignBit(value, 0, oldCarry);
}

static u8 RORbase(CPU* cpu, u8 value) {
	u8 oldCarry = GetBit(cpu->flags, CPU_FLAG_C);
	u8 newCarry = GetBit(value, 0);
	cpu->flags = AssignBit(cpu->flags, CPU_FLAG_C, newCarry);

	value >>= 1;
	return AssignBit(value, 7, oldCarry);
}

static void CMPbase(CPU* cpu, u8 reg, u8 value) {
	cpu->flags = AssignBit(cpu->flags, CPU_FLAG_Z, reg == value);
	cpu->flags = AssignBit(cpu->flags, CPU_FLAG_N, GetBit(reg - value, 7));
	cpu->flags = AssignBit(cpu->flags, CPU_FLAG_C, reg >= value);
}

static void JMPccbase(CPU* cpu, u16 addr, bool doJump) {
	if (doJump) {
		cpu->pc = addr;
		cpu->extraCycles++;
	} else {
		cpu->extraCycles = 0;
	}
}

// clear flags
void CLC(CPU* cpu, u16 input) { cpu->flags = ClearBit(cpu->flags, CPU_FLAG_C); }
void CLI(CPU* cpu, u16 input) { cpu->flags = ClearBit(cpu->flags, CPU_FLAG_I); }
void CLV(CPU* cpu, u16 input) { cpu->flags = ClearBit(cpu->flags, CPU_FLAG_V); }
void CLD(CPU* cpu, u16 input) { cpu->flags = ClearBit(cpu->flags, CPU_FLAG_D); }

// set flags
void SEC(CPU* cpu, u16 input) { cpu->flags = SetBit(cpu->flags, CPU_FLAG_C); }
void SEI(CPU* cpu, u16 input) { cpu->flags = SetBit(cpu->flags, CPU_FLAG_I); }
void SED(CPU* cpu, u16 input) { cpu->flags = SetBit(cpu->flags, CPU_FLAG_D); }

// transfers
void TAX(CPU* cpu, u16 input) { AssignAndUpdateNZ(cpu, &(cpu->x), cpu->a); }
void TAY(CPU* cpu, u16 input) { AssignAndUpdateNZ(cpu, &(cpu->y), cpu->a); }
void TAS(CPU* cpu, u16 input) { AssignAndUpdateNZ(cpu, &(cpu->sp), cpu->a); }
void TXA(CPU* cpu, u16 input) { AssignAndUpdateNZ(cpu, &(cpu->a), cpu->x); }
void TXS(CPU* cpu, u16 input) { cpu->sp = cpu->x; }
void TYA(CPU* cpu, u16 input) { AssignAndUpdateNZ(cpu, &(cpu->a), cpu->y); }
void TSX(CPU* cpu, u16 input) { AssignAndUpdateNZ(cpu, &(cpu->x), cpu->sp); }

// store
void STA(CPU* cpu, u16 input) { CPUMemWrite(cpu->memory, input, cpu->a); }
void STY(CPU* cpu, u16 input) { CPUMemWrite(cpu->memory, input, cpu->y); }
void STX(CPU* cpu, u16 input) { CPUMemWrite(cpu->memory, input, cpu->x); }

// load
void LDA(CPU* cpu, u16 input) { AssignAndUpdateNZ(cpu, &(cpu->a), CPUMemRead(cpu->memory, input)); }
void LDX(CPU* cpu, u16 input) { AssignAndUpdateNZ(cpu, &(cpu->y), CPUMemRead(cpu->memory, input)); }
void LDY(CPU* cpu, u16 input) { AssignAndUpdateNZ(cpu, &(cpu->x), CPUMemRead(cpu->memory, input)); }

// inc
void INC(CPU* cpu, u16 input) { MemWriteAndUpdateNZ(cpu, input, CPUMemRead(cpu->memory, input) + 1); }
void INX(CPU* cpu, u16 input) { AssignAndUpdateNZ(cpu, &(cpu->y), cpu->y + 1); }
void INY(CPU* cpu, u16 input) { AssignAndUpdateNZ(cpu, &(cpu->x), cpu->x + 1); }

// dec
void DEC(CPU* cpu, u16 input) { MemWriteAndUpdateNZ(cpu, input, CPUMemRead(cpu->memory, input) - 1); }
void DEX(CPU* cpu, u16 input) { AssignAndUpdateNZ(cpu, &(cpu->y), cpu->y - 1); }
void DEY(CPU* cpu, u16 input) { AssignAndUpdateNZ(cpu, &(cpu->x), cpu->x - 1); }

// arithmetics land
void ADC(CPU* cpu, u16 input) {
	u16 result = GetLowByte(input) + cpu->a + GetBit(cpu->flags, CPU_FLAG_C);

	bool newCarry = GetBit(result, 8);
	cpu->flags = AssignBit(cpu->flags, CPU_FLAG_C, newCarry);

	u8 signA = GetBit(cpu->a, 7);
	u8 signInput = GetBit(input, 7);
	u8 signResult = GetBit(result, 7);

	bool newV = false;
	if (((signA == signInput) && (signA != signResult))) newV = true;
	cpu->flags = AssignBit(cpu->flags, CPU_FLAG_V, newV);

	AssignAndUpdateNZ(cpu, &(cpu->a), result);
}

void SBC(CPU* cpu, u16 input) { ADC(cpu, 256 - GetLowByte(input) - 1); }

void AND(CPU* cpu, u16 input) { AssignAndUpdateNZ(cpu, &(cpu->a), cpu->a & GetLowByte(input)); }
void EOR(CPU* cpu, u16 input) { AssignAndUpdateNZ(cpu, &(cpu->a), cpu->a ^ GetLowByte(input)); }
void ORA(CPU* cpu, u16 input) { AssignAndUpdateNZ(cpu, &(cpu->a), cpu->a | GetLowByte(input)); }

void LSR(CPU* cpu, u16 input) { MemWriteAndUpdateNZ(cpu, input, LSRbase(cpu, CPUMemRead(cpu->memory, input))); }
void LSRa(CPU* cpu, u16 input) { AssignAndUpdateNZ(cpu, &(cpu->a), LSRbase(cpu, cpu->a)); }

void ASL(CPU* cpu, u16 input) { MemWriteAndUpdateNZ(cpu, input, ASLbase(cpu, CPUMemRead(cpu->memory, input))); }
void ASLa(CPU* cpu, u16 input) { AssignAndUpdateNZ(cpu, &(cpu->a), ASLbase(cpu, cpu->a)); }

void ROL(CPU* cpu, u16 input) { MemWriteAndUpdateNZ(cpu, input, ROLbase(cpu, CPUMemRead(cpu->memory, input))); }
void ROLa(CPU* cpu, u16 input) { AssignAndUpdateNZ(cpu, &(cpu->a), ROLbase(cpu, cpu->a)); }

void ROR(CPU* cpu, u16 input) { MemWriteAndUpdateNZ(cpu, input, RORbase(cpu, CPUMemRead(cpu->memory, input))); }
void RORa(CPU* cpu, u16 input) { AssignAndUpdateNZ(cpu, &(cpu->a), RORbase(cpu, cpu->a)); }

// stack
void PHP(CPU* cpu, u16 input) { CPUStackPush(cpu, SetBit(cpu->flags, CPU_FLAG_B)); }

void PLP(CPU* cpu, u16 input) {
	cpu->flags = CPUStackPop(cpu);
	cpu->flags = ClearBit(cpu->flags, CPU_FLAG_B);
}

void PHA(CPU* cpu, u16 input) { CPUStackPush(cpu, cpu->a); }
void PLA(CPU* cpu, u16 input) { AssignAndUpdateNZ(cpu, &(cpu->a), CPUStackPop(cpu)); }

// compare
void BIT(CPU* cpu, u16 input) {
	cpu->flags = AssignBit(cpu->flags, CPU_FLAG_Z, (input & cpu->a) == 0);
	cpu->flags = AssignBit(cpu->flags, CPU_FLAG_V, GetFlag(input, 6));
	cpu->flags = AssignBit(cpu->flags, CPU_FLAG_N, GetFlag(input, 7));
}

void CMP(CPU* cpu, u16 input) { CMPbase(cpu, cpu->a, input); }
void CPX(CPU* cpu, u16 input) { CMPbase(cpu, cpu->x, input); }
void CPY(CPU* cpu, u16 input) { CMPbase(cpu, cpu->y, input); }

// conditional branches
void BPL(CPU* cpu, u16 input) { JMPccbase(cpu, input, GetFlag(cpu->flags, !CPU_FLAG_N)); }
void BMI(CPU* cpu, u16 input) { JMPccbase(cpu, input, GetFlag(cpu->flags, CPU_FLAG_N)); }
void BVC(CPU* cpu, u16 input) { JMPccbase(cpu, input, GetFlag(cpu->flags, !CPU_FLAG_V)); }
void BVS(CPU* cpu, u16 input) { JMPccbase(cpu, input, GetFlag(cpu->flags, CPU_FLAG_V)); }
void BCC(CPU* cpu, u16 input) { JMPccbase(cpu, input, GetFlag(cpu->flags, !CPU_FLAG_C)); }
void BCS(CPU* cpu, u16 input) { JMPccbase(cpu, input, GetFlag(cpu->flags, CPU_FLAG_C)); }
void BNE(CPU* cpu, u16 input) { JMPccbase(cpu, input, GetFlag(cpu->flags, !CPU_FLAG_Z)); }
void BEQ(CPU* cpu, u16 input) { JMPccbase(cpu, input, GetFlag(cpu->flags, CPU_FLAG_Z)); }

// jumps
void JSR(CPU* cpu, u16 input) {
	CPUStackPush16(cpu, cpu->pc - 1);
	cpu->pc = input;
}

void JMP(CPU* cpu, u16 input) { cpu->pc = input; }

void RTS(CPU* cpu, u16 input) { cpu->pc = CPUStackPop16(cpu) + 1; }

void RTI(CPU* cpu, u16 input) {
	cpu->flags = CPUStackPop(cpu);
	cpu->pc = CPUStackPop16(cpu);
}

// weird land
void BRK(CPU* cpu, u16 input) { CPUInterrupt(cpu, CPU_INT_BRK, true); }
void NOP(CPU* cpu, u16 input) {}

// Illegal Land
void JAM(CPU* cpu, u16 input) {
	// WIP
}

void SAX(CPU* cpu, u16 input) {
	// WIP
}

void SLO(CPU* cpu, u16 input) {
	// WIP
}

void ANC(CPU* cpu, u16 input) {
	// WIP
}

void RLA(CPU* cpu, u16 input) {
	// WIP
}

void SRE(CPU* cpu, u16 input) {
	// WIP
}

void ALR(CPU* cpu, u16 input) {
	// WIP
}

void RRA(CPU* cpu, u16 input) {
	// WIP
}

void ARR(CPU* cpu, u16 input) {
	// WIP
}

void ANE(CPU* cpu, u16 input) {
	// WIP
}

void SHA(CPU* cpu, u16 input) {
	// WIP
}

void SHX(CPU* cpu, u16 input) {
	// WIP
}

void SHY(CPU* cpu, u16 input) {
	// WIP
}

void LAX(CPU* cpu, u16 input) {
	// WIP
}

void LXA(CPU* cpu, u16 input) {
	// WIP
}

void LAS(CPU* cpu, u16 input) {
	// WIP
}

void DCP(CPU* cpu, u16 input) {
	// WIP
}

void SBX(CPU* cpu, u16 input) {
	// WIP
}

void ISC(CPU* cpu, u16 input) {
	// WIP
}

void USBC(CPU* cpu, u16 input) {
	// WIP
}
