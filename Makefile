CFLAGS = -Wall -O2
CC = gcc

OBJS = sam7_pgm.o ihex.o serial.o settings.o cmd.o xmodem.o chip_id.o

sam7_pgm: $(OBJS)
	$(CC) $(CFLAGS) -o sam7_pgm $(OBJS) -lm

clean:
	rm -f sam7_pgm cksum_test *.o core core.*
