OUTPUT_FORMAT ("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
/* Internal Memory Map*/
MEMORY
{
	rom  (rx)  : ORIGIN = 0x08010000, LENGTH = 1024k - 64k
	ram  (rwx) : ORIGIN = 0x20000000, LENGTH = 128k
	ram1 (rwx) : ORIGIN = 0x10000000, LENGTH = 64k
}


INCLUDE arm-gcc-link_f4_main.ld
