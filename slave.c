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
#include <poll.h>
#include <string.h>
#include "os8104.h"
#include "gpio.h"

int main(int argc, char *argv[])
{
	int ret = 0;

	os8104_init(0);

	gpio_setedge(12, "falling");
	//os8104_writebyte(bMSGC, 0x07);
	os8104_writebyte(bMSGC, bMSGC_RBE);

	while (1) {
		unsigned char val;
		val = os8104_readbyte(bMSGS);
		if (!val)
			continue;
		//printf("!! %s INT:%d\n", os8104_bMSGS(), 0/*gpio_read(12)*/);
		if (val & bMSGS_MRX) {
			char buf[32];
			int i;
			os8104_readbytes(0xA0, buf, 19);
			printf("RECEIVED: ");
			for (i = 0; i < 8; i++)
				printf("%02x ", buf[i]);
			printf("  \n");
			//os8104_writebyte(0x85, 0x01);
			//os8104_writebyte(0x85, 0x40);
			//os8104_writebyte(bMSGC, bMSGC_RMRX);
			//os8104_writebyte(bMSGC, bMSGC_RBE);
			os8104_writebyte(0x85, 0x41);
			os8104_writebyte(0x85, 0x41);
			//os8104_writebyte(0x85, 0x41);
			//os8104_writebyte(0x85, 0x41);
			//os8104_writebyte(0x85, 0x41);
	//	os8104_writebyte(bIE, bIE_IMRX | bIE_IALC);
			printf("%s INT:%d\n", os8104_bMSGS(), 0/*gpio_read(12)*/);

		}
		if (val & bMSGS_MTX)
			os8104_writebyte(bMSGC, bMSGC_RMTX);
		if (val & bMSGS_ERR)
			os8104_writebyte(bMSGC, bMSGC_RERR);
		if (val & bMSGS_ALC)
			os8104_writebyte(bMSGC, bMSGC_RALC);
		//sleep(1);
	}

	while (1) {
		struct pollfd fdset[1];
		unsigned char val;
		int rc;

		/* wait for INT */
		fdset[0].fd = gpio_getfd(12);
		fdset[0].events = POLLPRI | POLLERR;
		fdset[0].revents = 0;
		rc = poll(fdset, 1, 5000);
		if (rc < 0) {
			printf("\npoll() failed!\n");
			return -1;
		}
#if 0
		printf("AF: 8e=%02x 87=%02x %s bIE=%02x %d\n",
				os8104_readbyte(0x8e),
				os8104_readbyte(0x87),
				os8104_bMSGS(),
			       	os8104_readbyte(bIE), gpio_read(12));
#endif

		if (rc == 0)
			continue;
		if (fdset[0].revents & POLLPRI == 0)
			continue;
		gpio_read(12);

check:
		val = os8104_readbyte(bMSGS);
		if (val & bMSGS_MRX) {
			char buf[32];
			int i;
			os8104_readbytes(0xA0, buf, 19);
			printf("RECEIVED: ");
			for (i = 0; i < 8; i++)
				printf("%02x ", buf[i]);
			printf("\n");
			//os8104_writebyte(bMSGC, bMSGC_RMRX);
			//os8104_writebyte(bMSGC, bMSGC_RBE);
			//os8104_writebyte(0x85, 0x01);
			//os8104_writebyte(0x85, 0x40);
			os8104_writebyte(0x85, 0x41);
			os8104_writebyte(0x85, 0x41);
	//	os8104_writebyte(bIE, bIE_IMRX | bIE_IALC);
			printf("%s INT:%d\n", os8104_bMSGS(), 0/*gpio_read(12)*/);

		}
		if (val & bMSGS_MTX)
			os8104_writebyte(bMSGC, bMSGC_RMTX);
		if (val & bMSGS_ERR)
			os8104_writebyte(bMSGC, bMSGC_RERR);
		if (val & bMSGS_ALC)
			os8104_writebyte(bMSGC, bMSGC_RALC);
		//sleep(1);
	}

	os8104_fini();

	return ret;
}

