#ifndef LU_NES_INTERRUPTS_H
#define LU_NES_INTERRUPTS_H

#include "Byte.h"

typedef enum InterruptID {
	CPU_INT_RESET,
	CPU_INT_NMI,
	CPU_INT_IRQ,
	CPU_INT_BRK,
} InterruptID;

u16 GetInterruptVector(InterruptID interrupt);

#endif	// LU_NES_INTERRUPTS_H
