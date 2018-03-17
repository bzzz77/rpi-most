static const char *device = "/dev/spidev0.1";
static unsigned short mode;
static unsigned char bits = 8;
static unsigned int speed = 200000;
static unsigned short delay;
static int fd;

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "os8104.h"
#include "gpio.h"

#ifndef __linux__
typedef unsigned long long __u64;
typedef unsigned long __u32;
typedef unsigned short __u16;
typedef unsigned char __u8;

#define SPI_IOC_WR_MODE	0
#define SPI_IOC_RD_MODE 0
#define SPI_IOC_WR_BITS_PER_WORD 0
#define SPI_IOC_RD_BITS_PER_WORD 0
#define SPI_IOC_RD_MAX_SPEED_HZ 0
#define SPI_IOC_WR_MAX_SPEED_HZ 0
#define SPI_IOC_MESSAGE(a)	0

struct spi_ioc_transfer {
	__u64		tx_buf;
	__u64		rx_buf;

	__u32		len;
	__u32		speed_hz;

	__u16		delay_usecs;
	__u8		bits_per_word;
	__u8		cs_change;
	__u32		pad;
};
#endif

static void pabort(const char *s)
{
	perror(s);
	abort();
}

char *os8104_bMSGS()
{
	static char msg[32];
	unsigned char val;

	val = os8104_readbyte(bMSGS);
	snprintf(msg, sizeof(msg), "bMSGS{%s%s%s%s%s%s}",
			val & 0x80 ? "RBS " : "",
			val & 0x40 ? "TXR " : "",
			val & 0x08 ? "ACL " : "",
			val & 0x04 ? "ERR " : "",
			val & 0x02 ? "MTX " : "",
			val & 0x01 ? "MRX " : "");
	return msg;
}

char *os8104_bXSR()
{
        static char msg[32];
        unsigned char val;

        val = os8104_readbyte(bXSR1);
        snprintf(msg, sizeof(msg), "bXSR1{%s%s%s%s%s%s}",
                        val & 0x40 ? "MSL " : "",
                        val & 0x20 ? "MXL " : "",
                        val & 0x10 ? "ME " : "",
                        val & 0x08 ? "ERR " : "",
                        val & 0x02 ? "ESL " : "",
                        val & 0x01 ? "EXL " : "");
        return msg;
}

char *os8104_bCM2()
{
	static char msg[32];
	unsigned char val;

	val = os8104_readbyte(bCM2);
	snprintf(msg, sizeof(msg), "bCM2{%s%s}",
		val & 0x80 ? "!LOC " : "LOC ",
		val & 0x40 ? "NAC " : "");
	return msg;
}

void gpio_init(void)
{
	// PIN22 (GPIO25) - RESET
	gpio_open(25, 1);

	// PIN32 (GPIO12) - INT
	gpio_open(12, 0);

	// PIN08 (GPIO24) - SPI SCK (to choose SPI at reset)
	gpio_open(8, 1);

	// reset OS8104 with SCL=0 to reset and enable SPI
	gpio_set(8, 0);
	gpio_set(25, 0);
	usleep(1000);
	gpio_set(25, 1);
	/* poll for 0 at PIN12 (reset completion) */
	while (gpio_read(12) == 1)
		usleep(10);

	// will be used by SPI
	gpio_close(8);
}

void gpio_fini(void)
{
	gpio_close(8);
	gpio_close(12);
	gpio_close(25);
}

void spi_init()
{
	int ret;

	fd = open(device, O_RDWR);
	if (fd < 0)
		pabort("can't open device");

	/*
	 * spi mode
	 */
	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
		pabort("can't set spi mode");

	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
		pabort("can't get spi mode");

	/*
	 * bits per word
	 */
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't set bits per word");

	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't get bits per word");

	/*
	 * max speed hz
	 */
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't set max speed hz");

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't get max speed hz");

}

void spi_fini()
{
	close(fd);
}

void os8104_init(int master)
{
	gpio_init();
	spi_init();

	printf("C4=%02x C5=%02x C6=%02x\n",
		os8104_readbyte(0xC4),
		os8104_readbyte(0xC5),
		os8104_readbyte(0xC6));

	/* SCK as output */
	os8104_writebyte(0x82, 0x10);
	/* clear power-on interrupt */
	os8104_writebyte(0x85, 0x04);
	/* bNAH (Node High Address) */
	os8104_writebyte(0x8A, 0x00);
	/* bNAL (Node Low Address) */
	os8104_writebyte(0x8B, 0x01);
	/* bGA (Group Address) */
	//os8104_writebyte(0x89, 0x01);

	if (master) {
		sleep(1);
		printf("Master .");fflush(stdout);
		/* master mode | transmitter output enabled */
		os8104_writebyte(bXCR, 0xE2);
		printf(".");fflush(stdout);
		/* PLL enabled | RMCK@512 | crystal@384Fs | PLL by XTI/O */
		os8104_writebyte(bCM1, 0x76);
		printf(".");fflush(stdout);
		/* FSY/SCK as output, SOURCE not muted */
		os8104_writebyte(bSDC1, 0x12);
		printf(".");fflush(stdout);
		/* Mask all errors except XML */
		os8104_writebyte(bXSR1, 0x50);
		printf(".");fflush(stdout);
		/* resrve 8 quadlets for sync.data */
		os8104_writebyte(bSBC, 0x08);
		printf(".");fflush(stdout);
		/* XXX: enable desired interrupts */
		/* reset all interrupts */
		os8104_writebyte(bMSGC, 0x0F);
		/* wait for PLL to lock */
		while (os8104_readbyte(bCM2) & bCM2_LOK) {
			printf("%02x\n", os8104_readbyte(bCM2));
			sleep(1);
		}
		printf("OK\n");fflush(stdout);
	} else {
		printf("Slave..");
		/* PLL enabled | RMCK@512 | crystal@384Fs | PLL by network */
		os8104_writebyte(bCM1, 0x74);
		/* slave mode | transmitter output enabled */
		os8104_writebyte(bXCR, 0x62);
		/* FSY/SCK as output, SOURCE not muted */
		os8104_writebyte(bSDC1, 0x12);
		/* Mask all errors except XML */
		os8104_writebyte(bXSR1, 0x50);
		/* XXX: enable desired interrupts */
		os8104_writebyte(bIE, 0 /* bIE_IMRX | bIE_IALC*/);
		/* reset all interrupts */
		os8104_writebyte(bMSGC, 0x0F);
		printf("OK\n");
	}
}

unsigned char os8104_readbyte(unsigned reg)
{
	char tx1[2], tx2[2], rx[2];
	struct spi_ioc_transfer tr[2] = { 0 };
	int rc;

	tx1[0] = 0x40;
	tx1[1] = reg;
	tx2[0] = 0x41;

	tr[0].tx_buf = (unsigned long)tx1;
	tr[0].rx_buf = (unsigned long)NULL;
	tr[0].len = 2;
	tr[0].delay_usecs = delay;
	tr[0].speed_hz = speed;
	tr[0].bits_per_word = bits;
	tr[0].cs_change = 1;
	tr[1].tx_buf = (unsigned long)tx2;
	tr[1].rx_buf = (unsigned long)rx;
	tr[1].len = 2;
	tr[1].delay_usecs = delay;
	tr[1].speed_hz = speed;
	tr[1].bits_per_word = bits;

	rc = ioctl(fd, SPI_IOC_MESSAGE(2), tr);
	if (rc < 1)
		pabort("can't send spi message");

	return rx[1];
}

int os8104_writebyte(unsigned reg, unsigned char value)
{
	char tx1[4];
	struct spi_ioc_transfer tr[1] = { 0 };
	int rc;

	tx1[0] = 0x40;
	tx1[1] = reg;
	tx1[2] = value;

	tr[0].tx_buf = (unsigned long)tx1;
	tr[0].rx_buf = (unsigned long)NULL;
	tr[0].len = 3;
	tr[0].delay_usecs = delay;
	tr[0].speed_hz = speed;
	tr[0].bits_per_word = bits;

	rc = ioctl(fd, SPI_IOC_MESSAGE(1), tr);
	if (rc < 1)
		pabort("can't send spi message");
	return 0;
}

void os8104_readbytes(unsigned reg, unsigned char *buf, int len)
{
	char tx1[2], tx2[2], rx[2], tmp[20];
	struct spi_ioc_transfer tr[2] = { 0 };
	int rc, i;

	if (len + 1 > sizeof(tmp))
		pabort("too small buffer");

	for (i = 0; i < 2; i++) {
		tr[i].delay_usecs = delay;
		tr[i].speed_hz = speed;
		tr[i].bits_per_word = bits;
		tr[i].cs_change = 1;
	}

	tx1[0] = 0x40;
	tx1[1] = reg;
	tx2[0] = 0x41;

	tr[0].tx_buf = (unsigned long)tx1;
	tr[0].rx_buf = (unsigned long)NULL;
	tr[0].len = 2;

	tr[1].tx_buf = (unsigned long)tx2;
	tr[1].rx_buf = (unsigned long)tmp;
	tr[1].len = len + 1;

	rc = ioctl(fd, SPI_IOC_MESSAGE(2), tr);
	if (rc < 1)
		pabort("can't send spi message");
	memcpy(buf, tmp + 1, len);
}

int os8104_writebytes(unsigned reg, unsigned char *val, int len)
{
	char tx1[2], tmp[20];
	struct spi_ioc_transfer tr[2] = { 0 };
	int rc, i;

	memcpy(tmp + 1, val, len);
	tx1[0] = 0x40;
	tx1[1] = reg;

	for (i = 0; i < 2; i++) {
		tr[i].delay_usecs = delay;
		tr[i].speed_hz = speed;
		tr[i].bits_per_word = bits;
	}

	tr[0].tx_buf = (unsigned long)tx1;
	tr[0].rx_buf = (unsigned long)NULL;
	tr[0].len = 2;

	tr[1].tx_buf = (unsigned long)val;
	tr[1].rx_buf = (unsigned long)NULL;
	tr[1].len = len;

	rc = ioctl(fd, SPI_IOC_MESSAGE(2), tr);
	if (rc < 1)
		pabort("can't send spi message");
	return 0;
}

int os8104_sai(int nal, int nah)
{
	unsigned val;

	os8104_writebyte(0xC3, nal);
	os8104_writebyte(0xC2, nah);
	os8104_writebyte(bMSGC, bMSGC_SAI);
	do {
		usleep(1000);
		val = os8104_readbyte(bMSGS);
	} while (!(val & bMSGS_MTX));
	os8104_writebyte(bMSGC, bMSGC_RMTX);
	return !!(val & bMSGS_TXR);
}

void os8104_init_addr(void)
{
	int i;
	for (i = 1; i < 16; i++) {
		if (os8104_sai(i, 0) == 0)
			continue;
		printf("got addr %d\n", i);
		return;
	}
	printf("CAN'T GET ADDR\n");
}

