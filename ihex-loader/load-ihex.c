#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include "rawusb.h"
#include "usb.h"

unsigned char hex2int(char c)
{
	if (c >= '0' && c <= '9')
		return (c - '0');
	if (c >= 'A' && c <= 'F')
		return (c - 'A') + 10;
	if (c >= 'a' && c <= 'f')
		return (c - 'a') + 10;
	printf("Invalid hex: %c\n", c);
	abort();
}

unsigned char getbyte(char* c)
{
	return (hex2int(c[0]) << 4) | hex2int(c[1]);
}

void mw(usb_handle *usb, uint32_t addr, uint32_t data)
{
	char buf[64];
	int done;
	
	snprintf(buf, 64, "oem mw %08x 1 %08x", addr, data);
	if (usb_write(usb, buf, strlen(buf)) < 0)
	{
		printf("E: usb_write failed\n");
		exit(3);
	}

	done = 0;
	while (!done)
	{
		int len = usb_read(usb, buf, 64);
		
		if (!strncmp(buf, "INFO", 4))
			printf("DEVICE: %s\n", buf+4);
		else if (!strncmp(buf, "OKAY", 4))
			done = 1;
		else if (!strncmp(buf, "FAIL", 4)) {
			printf("E: mw %08x 1 %08x failed; device reported %s\n", addr, data, buf+4);
			exit(3);
		} else {
			printf("E: unexpected response from device: %s\n", buf);
			exit(3);
		}
	}
}

int main(int argc, char** argv) {
	FILE* f;
	uint32_t seg_addr = 0x0;
	char linebuf[1024];
	int nwords = 0;
	
	usb_handle *usb;
	
	if (argc != 2)
	{
	  printf("Usage: %s hexfile\n", argv[0]);
	  return 1;
	}
	if (!(f = fopen(argv[1], "r")))
	{
	  perror("fopen");
	  return 1;
	}
	
	printf("Connecting to phone...\n");
	usb = rawusb_open(PROTO_FASTBOOT);
	
	printf("Uploading %s to phone...\n", argv[1]);
	while(fgets(linebuf, 1024, f))
	{
		int datalen;
		int rectype;
		uint32_t address;
		char *buf = linebuf;
		
		if (strlen(buf) < 11)
		{
			printf("E: Insufficiently long line: %d\n", (int) strlen(linebuf));
			exit(2);
		}
		
		if (*buf != ':')
		{
			printf("W: Invalid start of line: '%c' (skipping line)\n", *buf);
			continue;
		}
		buf++;
		
		datalen = getbyte(buf);
		buf += 2;
		
		address = getbyte(buf);
		address <<= 8;
		buf += 2;
		
		address |= getbyte(buf);
		buf += 2;
		
		rectype = getbyte(buf);
		buf += 2;

		switch(rectype)
		{
		case 0x00:	/* Data record */
			address += seg_addr;
			
			if (datalen % 4)
			{
				printf("E: line length %d not multiple of 32 bits\n", datalen);
				printf("E: by the spec, I should support this, but seriously dude, what the fuck\n");
				exit(2);
			}
			
			if (address % 4)
			{
				printf("E: line start address %08x not word-aligned\n", address);
				printf("E: by the spec, I should support this, but seriously dude, what the fuck\n");
				exit(2);
			}
			
			while (datalen)
			{
				uint32_t d;
				
				/* little endian, t(*A*t) */
				d = getbyte(buf);
				buf += 2;
				
				d |= getbyte(buf) << 8;
				buf += 2;
				
				d |= getbyte(buf) << 16;
				buf += 2;
				
				d |= getbyte(buf) << 24;
				buf += 2;
#ifdef DEBUG				
				printf("data: %08x -> %08x\n", address, d);
#endif
				mw(usb, address, d);
				
				nwords++;
				address += 4;
				datalen -= 4;
			}
			break;
		case 0x01:
			printf("EOF\n");
			break;
		case 0x02:
			seg_addr = address << 4;
			break;
		case 0x03:
			printf("W: unsupported: start segment address record\n");
			break;
		case 0x04:
			if (datalen != 2)
			{
				printf("E: extended linear address record with byte count %d (should be 2) unsupported\n", datalen);
				exit(2);
			}
			
			seg_addr = getbyte(buf) << 24 | getbyte(buf+2) << 16;
			break;
		case 0x05:
			printf("W: unsupported: start linear address record\n");
			break;
		}
	}
	fclose(f);
	
	printf("Loaded %d words\n", nwords);
	
	return 0;
}
