OUTPUT_FORMAT(
	"elf32-i386",  /* Default       */
	"elf32-i386",  /* Big endian    */
	"elf32-i386"   /* Little endian */
)
OUTPUT_ARCH(i386)
ENTRY(exbootmain)  /* 入口函数，你可以不定义成 main，只要是代码中的符号即可 */

SECTIONS
{
	/* 0x80000 is within base memory, and is reserved so that it can be used. */
	/* Temporarily, the extend boot is 0x4a94 big, safe to fit in. */
	/* On my mac, there's a BIOS-mem in OS course's directory. It will be helpful to refer to it. */
	. = 0x80000;

	PROVIDE(start = .);

	.text : AT(0x80000) {
		*(.text .text.*)
	}

	PROVIDE(etext = .);  /* 定义符号 etext, 其值为代码段之后的地址 */

	.rodata : {
		*(.rodata .rodata.*)
	}

	. = ALIGN(0x1000);  /* 将数据段按页对齐 */

	.data : {
		*(.data)
	}

	.bss : {
		*(.bss)
	}

	PROVIDE(end = .);  /* 定义符号 end, 其值为程序/数据段之后的地址 */
}
