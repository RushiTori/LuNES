#include "Instructions.h"

#include "CPU.h"

bool InstCallShouldGetArgValue(InstCall call) {
	if (call == ADC) return true;
	if (call == SBC) return true;
	if (call == LDA) return true;
	if (call == LDX) return true;
	if (call == LDY) return true;
	if (call == BIT) return true;
	if (call == CMP) return true;
	if (call == CPX) return true;
	if (call == CPY) return true;
	if (call == AND) return true;
	if (call == EOR) return true;
	if (call == ORA) return true;
	return false;
}

static void CPUUpdateNZ(CPU* cpu, u8 value) {
	cpu->flags = AssignBit(cpu->flags, CPU_FLAG_N, value & 0x80);
	cpu->flags = AssignBit(cpu->flags, CPU_FLAG_Z, value == 0);
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
	u8 newCarry = GetBit(value, 0);
	cpu->flags = AssignBit(cpu->flags, CPU_FLAG_C, newCarry);

	return value >> 1;
}

static u8 ASLbase(CPU* cpu, u8 value) {
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
void CLC(CPU* cpu, [[maybe_unused]] u16 input) { cpu->flags = ClearBit(cpu->flags, CPU_FLAG_C); }
void CLI(CPU* cpu, [[maybe_unused]] u16 input) { cpu->flags = ClearBit(cpu->flags, CPU_FLAG_I); }
void CLV(CPU* cpu, [[maybe_unused]] u16 input) { cpu->flags = ClearBit(cpu->flags, CPU_FLAG_V); }
void CLD(CPU* cpu, [[maybe_unused]] u16 input) { cpu->flags = ClearBit(cpu->flags, CPU_FLAG_D); }

// set flags
void SEC(CPU* cpu, [[maybe_unused]] u16 input) { cpu->flags = SetBit(cpu->flags, CPU_FLAG_C); }
void SEI(CPU* cpu, [[maybe_unused]] u16 input) { cpu->flags = SetBit(cpu->flags, CPU_FLAG_I); }
void SED(CPU* cpu, [[maybe_unused]] u16 input) { cpu->flags = SetBit(cpu->flags, CPU_FLAG_D); }

// transfers
void TAX(CPU* cpu, [[maybe_unused]] u16 input) { AssignAndUpdateNZ(cpu, &(cpu->x), cpu->a); }
void TAY(CPU* cpu, [[maybe_unused]] u16 input) { AssignAndUpdateNZ(cpu, &(cpu->y), cpu->a); }
void TXA(CPU* cpu, [[maybe_unused]] u16 input) { AssignAndUpdateNZ(cpu, &(cpu->a), cpu->x); }
void TXS(CPU* cpu, [[maybe_unused]] u16 input) { cpu->sp = cpu->x; }
void TYA(CPU* cpu, [[maybe_unused]] u16 input) { AssignAndUpdateNZ(cpu, &(cpu->a), cpu->y); }
void TSX(CPU* cpu, [[maybe_unused]] u16 input) { AssignAndUpdateNZ(cpu, &(cpu->x), cpu->sp); }

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
void INX(CPU* cpu, [[maybe_unused]] u16 input) { AssignAndUpdateNZ(cpu, &(cpu->y), cpu->y + 1); }
void INY(CPU* cpu, [[maybe_unused]] u16 input) { AssignAndUpdateNZ(cpu, &(cpu->x), cpu->x + 1); }

// dec
void DEC(CPU* cpu, u16 input) { MemWriteAndUpdateNZ(cpu, input, CPUMemRead(cpu->memory, input) - 1); }
void DEX(CPU* cpu, [[maybe_unused]] u16 input) { AssignAndUpdateNZ(cpu, &(cpu->y), cpu->y - 1); }
void DEY(CPU* cpu, [[maybe_unused]] u16 input) { AssignAndUpdateNZ(cpu, &(cpu->x), cpu->x - 1); }

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
void LSRa(CPU* cpu, [[maybe_unused]] u16 input) { AssignAndUpdateNZ(cpu, &(cpu->a), LSRbase(cpu, cpu->a)); }

void ASL(CPU* cpu, u16 input) { MemWriteAndUpdateNZ(cpu, input, ASLbase(cpu, CPUMemRead(cpu->memory, input))); }
void ASLa(CPU* cpu, [[maybe_unused]] u16 input) { AssignAndUpdateNZ(cpu, &(cpu->a), ASLbase(cpu, cpu->a)); }

void ROL(CPU* cpu, u16 input) { MemWriteAndUpdateNZ(cpu, input, ROLbase(cpu, CPUMemRead(cpu->memory, input))); }
void ROLa(CPU* cpu, [[maybe_unused]] u16 input) { AssignAndUpdateNZ(cpu, &(cpu->a), ROLbase(cpu, cpu->a)); }

void ROR(CPU* cpu, u16 input) { MemWriteAndUpdateNZ(cpu, input, RORbase(cpu, CPUMemRead(cpu->memory, input))); }
void RORa(CPU* cpu, [[maybe_unused]] u16 input) { AssignAndUpdateNZ(cpu, &(cpu->a), RORbase(cpu, cpu->a)); }

// stack
void PHP(CPU* cpu, [[maybe_unused]] u16 input) { CPUStackPush(cpu, SetBit(cpu->flags, CPU_FLAG_B)); }

void PLP(CPU* cpu, [[maybe_unused]] u16 input) {
	cpu->flags = CPUStackPop(cpu);
	cpu->flags = ClearBit(cpu->flags, CPU_FLAG_B);
}

void PHA(CPU* cpu, [[maybe_unused]] u16 input) { CPUStackPush(cpu, cpu->a); }
void PLA(CPU* cpu, [[maybe_unused]] u16 input) { AssignAndUpdateNZ(cpu, &(cpu->a), CPUStackPop(cpu)); }

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

void RTS(CPU* cpu, [[maybe_unused]] u16 input) { cpu->pc = CPUStackPop16(cpu) + 1; }

void RTI(CPU* cpu, [[maybe_unused]] u16 input) {
	cpu->flags = CPUStackPop(cpu);
	cpu->pc = CPUStackPop16(cpu);
}

// weird land
void BRK(CPU* cpu, [[maybe_unused]] u16 input) {
	cpu->pc++;
	CPUInterrupt(cpu, CPU_INT_BRK, true);
}

void NOP([[maybe_unused]] CPU* cpu, [[maybe_unused]] u16 input) {}

// Illegal Land
void JAM([[maybe_unused]] CPU* cpu, [[maybe_unused]] u16 input) {
	// WIP
}

void SAX([[maybe_unused]] CPU* cpu, [[maybe_unused]] u16 input) {
	// WIP
}

void SLO([[maybe_unused]] CPU* cpu, [[maybe_unused]] u16 input) {
	// WIP
}

void ANC([[maybe_unused]] CPU* cpu, [[maybe_unused]] u16 input) {
	// WIP
}

void RLA([[maybe_unused]] CPU* cpu, [[maybe_unused]] u16 input) {
	// WIP
}

void SRE([[maybe_unused]] CPU* cpu, [[maybe_unused]] u16 input) {
	// WIP
}

void ALR([[maybe_unused]] CPU* cpu, [[maybe_unused]] u16 input) {
	// WIP
}

void RRA([[maybe_unused]] CPU* cpu, [[maybe_unused]] u16 input) {
	// WIP
}

void ARR([[maybe_unused]] CPU* cpu, [[maybe_unused]] u16 input) {
	// WIP
}

void ANE([[maybe_unused]] CPU* cpu, [[maybe_unused]] u16 input) {
	// WIP
}

void SHA([[maybe_unused]] CPU* cpu, [[maybe_unused]] u16 input) {
	// WIP
}

void SHX([[maybe_unused]] CPU* cpu, [[maybe_unused]] u16 input) {
	// WIP
}

void SHY([[maybe_unused]] CPU* cpu, [[maybe_unused]] u16 input) {
	// WIP
}

void LAX([[maybe_unused]] CPU* cpu, [[maybe_unused]] u16 input) {
	// WIP
}

void LXA([[maybe_unused]] CPU* cpu, [[maybe_unused]] u16 input) {
	// WIP
}

void LAS([[maybe_unused]] CPU* cpu, [[maybe_unused]] u16 input) {
	// WIP
}

void DCP([[maybe_unused]] CPU* cpu, [[maybe_unused]] u16 input) {
	// WIP
}

void SBX([[maybe_unused]] CPU* cpu, [[maybe_unused]] u16 input) {
	// WIP
}

void ISC([[maybe_unused]] CPU* cpu, [[maybe_unused]] u16 input) {
	// WIP
}

void USBC([[maybe_unused]] CPU* cpu, [[maybe_unused]] u16 input) {
	// WIP
}

void TAS([[maybe_unused]] CPU* cpu, [[maybe_unused]] u16 input) {
	// WIP
}