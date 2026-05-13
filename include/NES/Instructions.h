#ifndef LU_NES_INSTRUCTIONS_H
#define LU_NES_INSTRUCTIONS_H

#include "Byte.h"

typedef struct CPU CPU;

typedef void (*InstCall)(CPU*, u16);

bool InstCallShouldGetArgValue(InstCall call);

// clear flags
void CLC(CPU* cpu, u16 input);
void CLI(CPU* cpu, u16 input);
void CLV(CPU* cpu, u16 input);
void CLD(CPU* cpu, u16 input);

// set flags
void SEC(CPU* cpu, u16 input);
void SEI(CPU* cpu, u16 input);
void SED(CPU* cpu, u16 input);

// transfers
void TAX(CPU* cpu, u16 input);
void TAY(CPU* cpu, u16 input);
void TAS(CPU* cpu, u16 input);
void TXA(CPU* cpu, u16 input);
void TXS(CPU* cpu, u16 input);
void TYA(CPU* cpu, u16 input);
void TSX(CPU* cpu, u16 input);

// store
void STA(CPU* cpu, u16 input);
void STY(CPU* cpu, u16 input);
void STX(CPU* cpu, u16 input);

// load
void LDA(CPU* cpu, u16 input);
void LDX(CPU* cpu, u16 input);
void LDY(CPU* cpu, u16 input);

// inc
void INC(CPU* cpu, u16 input);
void INX(CPU* cpu, u16 input);
void INY(CPU* cpu, u16 input);

// dec
void DEC(CPU* cpu, u16 input);
void DEX(CPU* cpu, u16 input);
void DEY(CPU* cpu, u16 input);

// arithmetics land
void ADC(CPU* cpu, u16 input);
void SBC(CPU* cpu, u16 input);
void AND(CPU* cpu, u16 input);
void EOR(CPU* cpu, u16 input);
void ORA(CPU* cpu, u16 input);
void LSR(CPU* cpu, u16 input);
void LSRa(CPU* cpu, u16 input);
void ASL(CPU* cpu, u16 input);
void ASLa(CPU* cpu, u16 input);
void ROL(CPU* cpu, u16 input);
void ROLa(CPU* cpu, u16 input);
void ROR(CPU* cpu, u16 input);
void RORa(CPU* cpu, u16 input);

// stack
void PHP(CPU* cpu, u16 input);
void PLP(CPU* cpu, u16 input);
void PHA(CPU* cpu, u16 input);
void PLA(CPU* cpu, u16 input);

// compare
void BIT(CPU* cpu, u16 input);
void CMP(CPU* cpu, u16 input);
void CPX(CPU* cpu, u16 input);
void CPY(CPU* cpu, u16 input);

// conditional branches
void BPL(CPU* cpu, u16 input);
void BMI(CPU* cpu, u16 input);
void BVC(CPU* cpu, u16 input);
void BVS(CPU* cpu, u16 input);
void BCC(CPU* cpu, u16 input);
void BCS(CPU* cpu, u16 input);
void BNE(CPU* cpu, u16 input);
void BEQ(CPU* cpu, u16 input);

// jumps
void JSR(CPU* cpu, u16 input);
void JMP(CPU* cpu, u16 input);
void RTS(CPU* cpu, u16 input);
void RTI(CPU* cpu, u16 input);

// weird land
void BRK(CPU* cpu, u16 input);
void NOP(CPU* cpu, u16 input);

// Illegal Land
void JAM(CPU* cpu, u16 input);
void SAX(CPU* cpu, u16 input);
void SLO(CPU* cpu, u16 input);
void ANC(CPU* cpu, u16 input);
void RLA(CPU* cpu, u16 input);
void SRE(CPU* cpu, u16 input);
void ALR(CPU* cpu, u16 input);
void RRA(CPU* cpu, u16 input);
void ARR(CPU* cpu, u16 input);
void ANE(CPU* cpu, u16 input);
void SHA(CPU* cpu, u16 input);
void SHX(CPU* cpu, u16 input);
void SHY(CPU* cpu, u16 input);
void LAX(CPU* cpu, u16 input);
void LXA(CPU* cpu, u16 input);
void LAS(CPU* cpu, u16 input);
void DCP(CPU* cpu, u16 input);
void SBX(CPU* cpu, u16 input);
void ISC(CPU* cpu, u16 input);
void USBC(CPU* cpu, u16 input);

#endif	// LU_NES_INSTRUCTIONS_H
