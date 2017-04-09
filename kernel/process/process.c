#include "process.h"
#include "irq.h"

PCB idle;
PCB* current = &idle;
PCB pcbPool[PCB_NUM];


void schedule()
{
	uint32_t esp0 = (uint32_t)current->tf;		//free space of TrapFrame
    esp0 += sizeof(struct TrapFrame);
    tss.esp0 = esp0;
}
