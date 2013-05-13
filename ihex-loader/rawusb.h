#ifndef _RAWUSB_H
#define _RAWUSB_H

#include "usb.h"

#define PROTO_ADB      0x1
#define PROTO_FASTBOOT 0x3

extern usb_handle *rawusb_open(int proto);

#endif
