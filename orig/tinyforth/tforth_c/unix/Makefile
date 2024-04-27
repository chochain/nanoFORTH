CC=gcc
CFLAGS=-Wall -Os

all : tforth

tforth : tforth.o system.o conio.o
	$(CC) $(CFLAGS) -o $@ $+

tforth.o : tforth.c system.h
	$(CC) $(CFLAGS) -c $<

tforth.c : ../tforth.c
	cp $< $@

system.o : system.c system.h
	$(CC) $(CFLAGS) -c $<

conio.o : conio.c conio.h
	$(CC) $(CFLAGS) -c $<

clean : 
	rm -f tforth tforth.o tforth.c system.o conio.o
