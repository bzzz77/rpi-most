CC=gcc
CCFLAGS=-g

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

bins := master slave 

all:		${bins}

master:		master.o gpio.o os8104.o
slave:		slave.o gpio.o os8104.o

clean:
		rm -rf ${bins} *.o *.dSYM


