.section .start
.extern cmd
.extern size
.global start
start:
	// Relocate to a safe spot.
	sub r0, pc, #8
	ldr r1, dest
	ldr r2, real_size

.loop:
	ldr r3, [r0], #4
	str r3, [r1], #4
	subs r2, r2, #4
	bne .loop
.done:

	// Dump the icache out.
	dmb sy
	isb sy
	dsb sy
	
	ldr pc, relocated_entry

dest:
	.long start
real_size:
	.long size
relocated_entry:
	.long cmd

#define NATIVE(name, ofs) name: ; .long ofs ; .globl name

NATIVE(outputf, 0x8E004A24)
NATIVE(text_guts, 0x8D00E6C4)
NATIVE(set_header, 0x0)
NATIVE(update_screen, 0x8D00FC6C)
NATIVE(atoi, 0x0)
NATIVE(draw_text, 0x8D00E70C)
NATIVE(sd_card_init, 0x8D00BFBC)
NATIVE(fat_open_file, 0x8D0160F4)
NATIVE(fat_read_file, 0x8D015AD4)
NATIVE(sleep_ms, 0x8D005238)
NATIVE(buzz, 0x0)
NATIVE(partition_table, 0x0)
NATIVE(set_boot_mode, 0x8D00916C)
NATIVE(reboot, 0x8D00B5E4)
NATIVE(fastboot_respond, 0x8D001F20)
NATIVE(board_battery_threshold, 0x8D00B668)
NATIVE(board_screen_x, 0x8D00B608)
NATIVE(board_screen_y, 0x8D00B624)
NATIVE(get_battery_voltage, 0x8D0538C8)
NATIVE(partition_update, 0x8D007ED0)
NATIVE(sprintf, 0x8D004A90)
NATIVE(powerdown, 0x8D04DCF0)

.global read_cpsr
read_cpsr:
	mrs r0, CPSR
	mov pc, lr
