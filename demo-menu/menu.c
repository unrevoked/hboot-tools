#include "api.h"

enum buttons {
	BTN_NONE = -1,
	BTN_POWER = 0,
	BTN_VOLUP = 1,
	BTN_VOLDN = 2,
};

int board_button_to_const(int raw)
{
	switch(raw)
	{
	case 1: return BTN_POWER;
	case 17: return BTN_VOLUP;
	case 18: return BTN_VOLDN;
	default: return BTN_NONE;
	}
}

void *memcpy(char *dst, char *src, int n)
{
	char *dst0 = dst;
	while (n--)
		*(dst++) = *(src++);
	return dst0;
}

void *memset(char *b, int c, int len)
{
	char *b0 = b;
	while (len--)
		*(b++) = c;
	return b0;
}

void draw(char *s, int row, int col, int fg, int bg)
{
	text_guts(col, row, s, 0, fg);
	text_guts(col, row, s, 1, bg);
}

void tohex(char *s, unsigned long i)
{
	int p = 0;
	char *hex = "0123456789ABCDEF";
	
	for (p = 7; p >= 0; p--)
	{
		s[p] = hex[i & 0xF];
		i >>= 4;
	}
	s[8] = 0;
}

int test_buttons()
{
	int i;
	int (*buttoncheck)(int) = *(int (*)(int))0x8D03D904;
	
	for (i = 1; i < 32; i++)
		if (buttoncheck(i))
			return i;
	return 0;
}

/* XXX mecha hack */
unsigned int *get_fb()
{
	int *curfb = 0x8D098F50;
	
	if (*curfb)
		return ((void* (*)())0x8D00FA68)();
	else
		return ((void* (*)())0x8D00FA58)();
}

void clear_fb()
{
	unsigned int *fb = get_fb();
	int c;
	
	for (c = 0; c < board_screen_x()*board_screen_y()*2/4; c++)
		fb[c] = 0;
}

struct line {
	char *s;
	unsigned short fgcolor, bgcolor;
};

extern int image_width, image_height;
extern unsigned short image_data[];

int cons_r;
void draw_message(struct line *lines)
{
	clear_fb();
	int i;
	unsigned short *fb = get_fb();
	
	for (i = 0; i < image_width * image_height; i++)
		fb[i] = image_data[i];
	
	/* XXX 24 is hardcoded for Mecha */
	cons_r = image_height / 24;
	while (lines->s)
	{
		draw(lines->s, cons_r++, 1, lines->fgcolor, lines->bgcolor);
		lines++;
	}
}

struct menuitem {
	char *item;
	int value;
	unsigned short color;
};

int do_menu(struct menuitem *items)
{
	int menu_start_r = cons_r;
	int nitems;
	int curitem = 0;
	int button;
	
	for (nitems = 0; items[nitems].item; nitems++)
		;
	
	do {
		cons_r = menu_start_r;
		
		draw("  <VOLUP> Select previous item.", cons_r++, 1, COL_BGRN, COL_BLACK);
		draw("  <VOLDN> Select next item.", cons_r++, 1, COL_BGRN, COL_BLACK);
		draw("  <POWER> Choose item.", cons_r++, 1, COL_BGRN, COL_BLACK);
		cons_r++;
		
		for (nitems = 0; items[nitems].item; nitems++)
		{
			draw(items[nitems].item, cons_r++, 1,
			     (nitems == curitem) ? COL_BLACK : items[nitems].color,
			     (nitems == curitem) ? items[nitems].color : COL_BLACK);
		}
		update_screen();
		
		do {
			sleep_ms(100);
			button = board_button_to_const(test_buttons());
		} while (button == BTN_NONE);
		
		if (button == BTN_VOLDN)
		{
			/* can't use mod, since it needs libgcc */
			curitem++;
			if (curitem == nitems)
				curitem = 0;
		}
		
		if (button == BTN_VOLUP)
		{
			curitem--;
			if (curitem == -1)
				curitem = nitems - 1;
		}
		
	} while(button != BTN_POWER);
	
	char s[64];
	sprintf(s, "INFOValue is %d", items[curitem].value);
	fastboot_respond(s);
	return items[curitem].value;
}

void battery_too_low()
{
	char s[64];
	
	struct line msg[] = {
		{ " Battery too low                      ", COL_BLACK, COL_BRED },
		{ "--------------------------------------", COL_BRED, COL_BLACK },
		{ "The battery in your phone is too", COL_BRED, COL_BLACK },
		{ "low to safely reflash your boot-", COL_BRED, COL_BLACK },
		{ "loader.", COL_BRED, COL_BLACK },
		{ "", COL_BRED, COL_BLACK },
		{ "Turn off your phone, and let it", COL_BRED, COL_BLACK },
		{ "charge for at least half an hour", COL_BRED, COL_BLACK },
		{ "before trying again.", COL_BRED, COL_BLACK },
		{ "", COL_BRED, COL_BLACK },
		{ "  <POWER> Turn off the phone.", COL_BGRN, COL_BLACK },
		{ 0, 0, 0 },
	};
	
	draw_message(msg);
	update_screen();
	
	sprintf(s, "INFOBattery too low: %dmV (thresh %d)", get_battery_voltage(), board_battery_threshold());
	fastboot_respond(s);
	
	while (board_button_to_const(test_buttons()) != BTN_POWER)
		sleep_ms(100);
	
	/* Next time we boot, just boot straight into fastboot. */
	set_boot_mode(2);
	powerdown(); /* XXX on Mecha, this actually reboots!  Same if you do a 'fastboot oem powerdown'. */
}

/* At this point, we are running in SVC mode (M[4:0] = 10011), and asynch
 * aborts, IRQs, and FIQs are disabled.  */
int cmd()
{
	int r = 2;
	int c;
	char s[64] = "INFO";
	
	
	struct line confirmation[] = {
		{ " Confirmation                         ", COL_BLACK, COL_BRED },
		{ "--------------------------------------", COL_BRED, COL_BLACK },
		{ "unrEVOked AlphaRev is about to re-", COL_BRED, COL_BLACK },
		{ "flash your bootloader.  This will", COL_BRED, COL_BLACK },
		{ "almost certainly void your warranty.", COL_BRED, COL_BLACK },
		{ "Do not take your phone back to a", COL_BRED, COL_BLACK },
		{ "store for repair before reflashing", COL_BRED, COL_BLACK },
		{ "the original bootloader!", COL_BRED, COL_BLACK },
		{ "", COL_BRED, COL_BLACK },
		{ 0, 0, 0 },
	};
	
	draw_message(confirmation);
	
	struct menuitem items[] = {
		{ "Reboot                                 ", 1, COL_BCYAN },
		{ "Reboot                                 ", 1, COL_BCYAN },
		{ "I understand the risks. Flash my phone.", 2, COL_BRED },
		{ "Reboot                                 ", 1, COL_BCYAN },
		{ "Show battery screen                    ", 3, COL_BMAG },
		{ "Patch commands and return              ", 4, COL_BGRN },
		{ 0, 0, 0 },
	};
	
	switch (do_menu(items))
	{
	case 1:
		reboot();
		/* no return */
	case 2:
		/* fu */
		break;
	case 3:
		
		battery_too_low();
		/* no return */
	case 4:
		*(unsigned int *)0x8D099388 = 0x8D09395C; /* patch in command table on mecha... */
		((void (*)())0x8D0014A8)(); /* and jump to a safe point in board_init (the probe code kills the display) */
		/* no return */
	}
	
	fastboot_respond("INFOWould flash phone.");
	
	sprintf(s, "INFOBattery voltage: %dmV (thresh %d)", get_battery_voltage(), board_battery_threshold());
	fastboot_respond(s);
	
	if (get_battery_voltage() < board_battery_threshold())
		battery_too_low();

	sleep_ms(50);
	set_boot_mode(2);
	reboot();
	/* Here we go... */
	
	/* XXX Needs a safe place for it -- check the mmu mapping for
	 * something in physical memory?  Having a MB binary at the end of
	 * our code will trash the stack (and other stuff maybe??) if we
	 * load it at bss_end...
	 */
	 
//	extern unsigned char _binary_hboot_nb0_start[];
//	extern unsigned char start[];
//	memcpy(0x8D000000, _binary_hboot_nb0_start, start - 0x8D000000);
	
//	((void (*)())0x8D001000)();
	
	//reboot();
}

