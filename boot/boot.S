# 系统启动时，BIOS会完成初始化，然后将磁盘第一个512字节保存到物理内存0x00007C00
# 位置，然后设置%CS=0000h, %IP=7C00h

#include "asm.h"
#include "mmu.h"

# 从此开始是16位代码
.code16
.globl start
start:
	cli                       # 关闭外部中断

	# Open graph mode:
	movw	$0x4f02, %ax
	movw	$0x0115, %bx
	movw	$0x7e00, %di
	int		$0x10

	movw	$0x4f01, %ax
	movw	$0x0115, %cx
	movw	$0x7e00, %di
	int		$0x10


	# 设置正确的段寄存器
	xorw    %ax, %ax          # %AX = 0
	movw    %ax, %ds          # %DS = AX (data segment)
	movw    %ax, %es          # %ES = AX (extra segment)
	movw    %ax, %ss          # %SS = AX (stack segment)

	# Enable A20:
	seta20.1:
		inb		$0x64, %al
		testb	$0x2, %al
		jnz		seta20.1

		movb	$0xd1, %al
		outb	%al, $0x64

	seta20.2:
		inb		$0x64, %al
		testb	$0x2, %al
		jnz		seta20.2

		movb	$0xdf, %al
		outb	%al, $0x60


	lgdt    gdtdesc           # 设置GDT(段描述符表地址为$gdt)
	movl    %cr0, %eax        # %CR0 |= PROTECT_ENABLE_BIT
	orl     $0x1, %eax
	movl    %eax, %cr0        # 设置PE bit

	# 长跳转进入保护模式，设置%CS为GDT中的第一项，%EIP为start32所对应地址
	ljmp    $GDT_ENTRY(1), $start32

.code32
start32:
	# 设置数据访问所用的段寄存器(%DS, %ES, %SS)
	movw    $GDT_ENTRY(2), %ax
	movw    %ax, %ds          # %DS = %AX	data segment
	movw    %ax, %es          # %ES = %AX	data segment
	movw    %ax, %ss          # %SS = %AX	data segment
	movw	%ax, %fs
	movw	%ax, %gs

	# 设置栈位置。栈从此没有切换过，请注意栈的大小！
	movl	$0, %ebp
	movl    $(128 << 20), %esp     # %ESP = $0x8000000
	call    bootmain          # jump to C code

# GDT
.p2align 2                         # 对齐
gdt:
	SEG_NULLASM                    			# GDT: the first term must be NULL
	SEG_ASM(STA_X|STA_R, 0x0, 0xffffffff)	# executable and readable	code segment
	SEG_ASM(STA_W, 0x0, 0xffffffff)			# writeable	data segment

gdtdesc:                           # GDT descriptor
	.word   (gdtdesc - gdt - 1)    # sizeof(gdt) - 1
	.long   gdt                    # address gdt

