OUTPUT_FORMAT(
	"elf32-i386",  /* Default       */
	"elf32-i386",  /* Big endian    */
	"elf32-i386"   /* Little endian */
)
OUTPUT_ARCH(i386)
ENTRY(game_main)  /* 入口函数，你可以不定义成 main，只要是代码中的符号即可 */

SECTIONS
{
	/* Paging has been implemented, and the virtual address can be set to 0x8048000. */
	/**
	 * 可执行程序的地址空间从 32MB 往上开始
	 */
	/*. = 0x2000000;*/
	. = 0x8048000;

	PROVIDE(start = .);

	/*.text : AT(0x2000000) {   AT 指定物理地址 */
	.text : AT(0x8048000) {
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
