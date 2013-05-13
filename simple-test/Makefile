PLATFORM?=supersonic

%.hex: %.elf
	arm-eabi-objcopy $< -O ihex $@

%.elf: %.o $(PLATFORM).lds $(PLATFORM).o
	arm-eabi-ld -o $@ -T $(PLATFORM).lds $(PLATFORM).o $<

%.o: %.c
	arm-eabi-gcc -march=armv7-a -c -o $@ $<

%.o: %.S
	arm-eabi-gcc -march=armv7-a -c -o $@ $<

clean:
	rm -f *.o *.hex *.elf

.PHONY: clean
