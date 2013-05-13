all: menu.bin

CROSS ?= arm-elf-
PLATFORM ?= mecha

%.bin: %.elf
	$(CROSS)objcopy -O binary $< $@

%.elf: %.o $(PLATFORM).lds $(PLATFORM).o hboot.o unrevoked-alpharev.o
	$(CROSS)ld -static -o $@ -T $(PLATFORM).lds $(PLATFORM).o unrevoked-alpharev.o $<

%.o: %.nb0
	$(CROSS)objcopy -I binary -O elf32-littlearm -B arm $< $@

%.o: %.c
	$(CROSS)gcc -march=armv7-a -c -O -o $@ $<

%.o: %.S
	$(CROSS)gcc -march=armv7-a -c -O -o $@ $<

ppmify: ppmify.c
	gcc -o ppmify ppmify.c

%.c: %.ppm ppmify
	./ppmify < $< > $@

%.ppm: %.png
	convert $< $@

