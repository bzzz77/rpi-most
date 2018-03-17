/*
 * SPI testing utility (using spidev driver)
 *
 * Copyright (c) 2007  MontaVista Software, Inc.
 * Copyright (c) 2007  Anton Vorontsov <avorontsov@ru.mvista.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * Cross-compile with cross-gcc -I/path/to/cross-kernel/include
 */

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "os8104.h"

int main(int argc, char *argv[])
{
	char tx[16], rx[16];
	int ret = 0;
	int i;

	os8104_init(1);

	//os8104_writebyte(fd, 0x83, 0x82);
	{
		unsigned char msg[32];
		msg[0] = 0x00;
		msg[1] = 0x00;
		msg[2] = 0x03;
		msg[3] = 0xc8;
		msg[4] = 0x12;
		msg[5] = 0x10;
		msg[6] = 0x11;
		msg[7] = 0x13;
		os8104_writebytes(0xc0, msg, 16);
		//os8104_writebyte(0x85, 0x80);
	}

	i = 1;
	while (1) {
		unsigned char val;

		os8104_writebyte(0xc0 + 5, i++);
		os8104_writebyte(0xc0 + 6, i);
		os8104_writebyte(bMSGC, bMSGC_STX);
		do {
			val = os8104_readbyte(bMSGS);
		} while ((val & bMSGS_MTX) == 0);
		if ((val & bMSGS_TXR) == 0) {
			printf("ERROR\n");
			continue;
		} else
			printf("SENT %02x\n", i);
		os8104_writebyte(bMSGC, bMSGC_RMTX);

		printf("83=%02x 8e=%02x 87=%02x %s %s\n",
				os8104_readbyte(0x83),
				os8104_readbyte(0x8e),
				os8104_readbyte(0x87),
				os8104_bMSGS(),
				os8104_bXSR());
		val = os8104_readbyte( 0x86);
		if (val & 0x01) {
			if ((val & 0x40) == 0)
				printf("TX ERR: %02x\n",
					os8104_readbyte(0xd5));
			os8104_writebyte(0x85, 0x01);
		}
		if (val & 0x02)
			os8104_writebyte(0x85, 0x02);
		if (val & 0x04)
			os8104_writebyte(0x85, 0x04);
		if (val & 0x08)
			os8104_writebyte(0x85, 0x08);
		if (i % 2 == 0)
			usleep(3 * 100000);
		else
			usleep(5 * 100000);
	}

	os8104_fini();

	return ret;
}
