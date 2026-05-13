#ifndef LU_NES_OPERATIONS_H
#define LU_NES_OPERATIONS_H

#include "AddressingMode.h"
#include "Instructions.h"

InstCall OperationsGetCall(u8 opcode);
u8 OperationsGetClocks(u8 opcode);
u8 OperationsGetAddressingMode(u8 opcode);
bool OperationsHasPageCrossPenalty(u8 opcode);

#endif	// LU_NES_OPERATIONS_H
