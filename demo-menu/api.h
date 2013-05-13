#ifndef _API_H
#define _API_H

extern void (*outputf)(char *fmt, ...);
extern void (*draw_text)(char *s, int color);
extern void (*set_header)(char *s, int color);
extern void (*update_screen)();
extern void (*atoi)(char *s, int *v);
extern void (*text_guts)(int col, int row, char *s, int inverted, int color);
extern int  (*sd_card_init)();
typedef void *fat_handle;
extern fat_handle (*fat_open_file)(char *f, int *length);
extern int (*fat_read_file)(fat_handle h, void *buf, int length);
extern void (*sleep_ms)(int time);
extern void (*buzz)();
extern void (*reboot)();
extern void (*set_boot_mode)(int i);
extern void (*fastboot_respond)(char *s);
extern int (*board_battery_threshold)();
extern int (*board_screen_x)();
extern int (*board_screen_y)();
extern int (*get_battery_voltage)();
extern int (*partition_update)(char *pname, char *buf, int size);
extern void (*sprintf)(char *buf, char *fmt, ...);
extern void (*powerdown)();

#define BOOT_MODE_FASTBOOT 2

struct partition {
	char *name;
	unsigned int start;
	unsigned int size;
	unsigned int flags1;
	unsigned int flags2;
};

extern struct partition *partition_table;

extern int _end;

#define COL_BLACK  0x0000
#define COL_BLUE   0x000F
#define COL_GREEN  0x03E0
#define COL_CYAN   0x03EF
#define COL_RED    0x7800
#define COL_MAG    0x780F
#define COL_BRWN   0x7BE0
#define COL_GRAY   0x7BEF
#define COL_BBLUE  0xF100
#define COL_BGRN   0x07E0
#define COL_BCYAN  0x07FF
#define COL_BRED   0xF100
#define COL_BMAG   0xF11F
#define COL_YLLW   0xFFE0
#define COL_WHITE  0xFFFF

#endif
