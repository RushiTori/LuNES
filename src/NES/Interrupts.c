#include "Interrupts.h"

u16 GetInterruptVector(InterruptID interrupt) {
	switch (interrupt) {
		default:
		case CPU_INT_RESET: return 0XFFFC;
		case CPU_INT_NMI: return 0XFFFA;
		case CPU_INT_IRQ: return 0XFFFE;
		case CPU_INT_BRK: return 0XFFFE;
	}
}
