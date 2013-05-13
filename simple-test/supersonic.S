.section .start
.extern cmd
start:
	// Clean the dcache first; mcr 15, 0, %0, c7, c10, 1
	
	// Dump the icache out.
	dsb sy
	mov r2, #0
	mcr p15, 0, r2, c7, c5, 0
	
	b cmd

#define NATIVE(name, ofs) .offset ofs ; name: ; .globl name

NATIVE(outputf, 0x8E004AB8)
NATIVE(draw_text, 0x8E00C39C)
NATIVE(set_header, 0x8E00CB18)
NATIVE(update_screen, 0x8E00D2EC)
NATIVE(atoi, 0x8E01F1D8)
NATIVE(text_guts, 0x8E00C360)
NATIVE(sd_card_init, 0x8E009A64)
NATIVE(fat_open_file, 0x8E012A54)
NATIVE(fat_read_file, 0x8E012438)
NATIVE(sleep_ms, 0x8E004E8C)
NATIVE(buzz, 0x8E033AA0)
NATIVE(partition_table, 0x8E07BAA0)