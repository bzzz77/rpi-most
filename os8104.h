#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>


/* registers */
#define	bXCR	0x80
#define bCM1	0x83
#define bCM2	0x8E
#define bSDC1	0x82
#define bXSR1	0x81
#define bSBC	0x96
#define bMSGC	0x85
#define bMSGS	0x86
#define bIE	0x88

/* bIE */
#define bIE_IMRX	0x01
#define bIE_IMTX	0x02
#define bIE_IERR	0x04
#define bIE_IALC	0x08

/* bMSGS */
#define bMSGS_MRX	0x01
#define bMSGS_MTX	0x02
#define bMSGS_ERR	0x04
#define bMSGS_ALC	0x08
#define bMSGS_TXR	0x40
#define bMSGS_RBS	0x80

/* bMSGC */
#define bMSGC_RMRX	0x01
#define bMSGC_RMTX	0x02
#define bMSGC_RERR	0x04
#define bMSGC_RALC	0x08
#define bMSGC_SAI	0x10
#define bMSGC_RBE	0x40
#define bMSGC_STX	0x80

/* bCM2 */
#define	bCM2_LP		0x10
#define	bCM2_ZP		0x20
#define	bCM2_NAC	0x40
#define	bCM2_LOK	0x80


void os8104_init(int master);
void os8104_fini(void);
unsigned char os8104_readbyte(unsigned reg);
int os8104_writebyte(unsigned reg, unsigned char value);
void os8104_readbytes(unsigned reg, unsigned char *buf, int len);
int os8104_writebytes(unsigned reg, unsigned char *val, int len);
char *os8104_bMSGS();
char *os8104_bXSR();

