CFLAGS = -Wall -O2
CC = gcc

OBJS = sam7_pgm.o ihex.o serial.o settings.o cmd.o xmodem.o chip_id.o

sam7_pgm: $(OBJS)
	$(CC) $(CFLAGS) -o sam7_pgm $(OBJS) -lm

flasher.c flasher.h: flasher.armasm armlst2c
	arm-elf-as flasher.armasm -o flasher.armobj
	arm-elf-objdump -d flasher.armobj > flasher.armlst
	./armlst2c flasher

clean:
	rm -f sam7_pgm cksum_test *.o core core.*
	rm -f flasher.c flasher.h *.armobj *.armlst

