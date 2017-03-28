#ifndef __IRQ_H__
#define __IRQ_H__

#include "types.h"
/* will never be used
typedef struct TrapFrame {
	uint32_t edi, esi, ebp, old_esp, ebx, edx, ecx, eax;
	int32_t irq;
	uint32_t error_code, eip, cs, eflags;
} TrapFrame;
*/

typedef struct TrapFrame {
	uint32_t edi, esi, ebp, old_esp, ebx, edx, ecx, eax;	// GPRs
	uint32_t gs, fs, es, ds;					// data segment registers
	int32_t irq;								// #irq
	uint32_t error_code;						// error code
	uint32_t eip, cs, eflags;					// execution state saved by hardware
	uint32_t esp, ss;							// only uses when DPL = 3
} TrapFrame;


#endif
