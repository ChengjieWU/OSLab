#include "x86/memory.h"

#----|-----entry------|-errorcode-|-----id-----|---handler---|
.globl vec0;    vec0:   pushl $0;  pushl    $0; jmp asm_do_irq
.globl vec1;    vec1:   pushl $0;  pushl    $1; jmp asm_do_irq
.globl vec2;    vec2:   pushl $0;  pushl    $2; jmp asm_do_irq
.globl vec3;    vec3:   pushl $0;  pushl    $3; jmp asm_do_irq
.globl vec4;    vec4:   pushl $0;  pushl    $4; jmp asm_do_irq
.globl vec5;    vec5:   pushl $0;  pushl    $5; jmp asm_do_irq
.globl vec6;    vec6:   pushl $0;  pushl    $6; jmp asm_do_irq
.globl vec7;    vec7:   pushl $0;  pushl    $7; jmp asm_do_irq
.globl vec8;    vec8:              pushl    $8; jmp asm_do_irq
.globl vec9;    vec9:   pushl $0;  pushl    $9; jmp asm_do_irq
.globl vec10;   vec10:             pushl   $10; jmp asm_do_irq
.globl vec11;   vec11:             pushl   $11; jmp asm_do_irq
.globl vec12;   vec12:             pushl   $12; jmp asm_do_irq
.globl vec13;   vec13:             pushl   $13; jmp asm_do_irq
.globl vec14;   vec14:             pushl   $14; jmp asm_do_irq

.globl vecsys; vecsys:  pushl $0;  pushl $0x80; jmp asm_do_irq

.globl irq0;     irq0:  pushl $0;  pushl $1000; jmp asm_do_irq
.globl irq1;     irq1:  pushl $0;  pushl $1001; jmp asm_do_irq
.globl irq14;   irq14:  pushl $0;  pushl $1014; jmp asm_do_irq
.globl irq_empty; irq_empty:pushl $0;  pushl   $-1; jmp asm_do_irq

# the label "current" here is the memory 'address'!! of variable current
.extern current

.globl asm_do_irq
.extern irq_handle

# The follow code is *VERY* important for interrupts and exceptions handling.
# You should make yourself understand it clearly.

#asm_do_irq:	will never be used
#	pushal
#
#	pushl %esp		# ???
#	call irq_handle
#	
#	addl $4, %esp
#	popal
#	addl $8, %esp
#	iret


asm_do_irq:
	cli		#avoid nested interruptions, eflags has been stored and iret will restore it,
			#at the same time, allow interruptions again
			
	pushl %ds
	pushl %es
	pushl %fs
	pushl %gs
	pushal

	movw $SELECTOR_KERNEL(SEG_KERNEL_DATA), %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %gs
	movw %ax, %fs

	pushl %esp
	call irq_handle
	
	movl current, %eax
    movl (%eax),%esp

	popal
	popl %gs
	popl %fs
	popl %es
	popl %ds
	addl $8, %esp
	
	iret
