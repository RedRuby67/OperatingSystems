CC = gcc
CFLAGS = -c -g -Wall -Wextra
LFLAGS = -g -Wall -Wextra

INPUTFILESIZEMEGABYTES = 1

KILO = 1024
MEGA = $(shell echo $(KILO)\*$(KILO) | bc)
INPUTFILESIZEBYTES = $(shell echo $(MEGA)\*$(INPUTFILESIZEMEGABYTES) | bc)
INPUTBLOCKSIZEBYTES = $(KILO)
INPUTBLOCKS = $(shell echo $(INPUTFILESIZEBYTES)\/$(INPUTBLOCKSIZEBYTES) | bc)

.PHONY: all clean

all: cpu io mixed

cpu: cpu.o
	$(CC) $(LFLAGS) $^ -o $@ -lm

io: io.o rwinput
	$(CC) $(LFLAGS) io.o -o $@ -lm


mixed: mixed.o rwinput
	$(CC) $(LFLAGS) mixed.o -o $@ -lm

cpu.o: cpu.c
	$(CC) $(CFLAGS) $<

io.o: io.c
	$(CC) $(CFLAGS) $<

mixed.o: mixed.c
	$(CC) $(CFLAGS) $<

rwinput: Makefile
	dd if=/dev/urandom of=./rwinput bs=$(INPUTBLOCKSIZEBYTES) count=$(INPUTBLOCKS)

clean: testclean
	rm -f cpu io mixed
	rm -f rwinput
	rm -f *.o
	rm -f *~

testclean:
	rm -f rwoutput*
