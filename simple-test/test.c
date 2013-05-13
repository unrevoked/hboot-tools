#include "api.h"

void draw(char *s, int row, int col, int fg, int bg)
{
	text_guts(col, row, s, 0, fg);
	text_guts(col, row, s, 1, bg);
}

int cmd(int argc, char **argv)
{
	int i;
	int r = 25;
	fat_handle p;
	struct partition *par = partition_table;
	
	draw("Joshua's magic command!", r++, 1, COL_BGRN, COL_BLUE);
	
	buzz();
	
	for (par = partition_table; par->name; par++)
		outputf("Partition %s: %x -> %x (%x), flags %x %x\n", par->name, par->start, par->start + par->size, par->size, par->flags1, par->flags2);
	
	
	outputf("Initializing SD card...");
	sd_card_init();
	
	outputf("argc: %d", argc);
	for (i = 0; i < argc; i++)
	{
		int flags;
		
		flags = 0;
		
		outputf("Trying to open: %s", argv[i]);
		p = fat_open_file(argv[i], &flags);
		
		outputf("result: p %x, flags %x", p, flags);
	}
	
	update_screen();
}

