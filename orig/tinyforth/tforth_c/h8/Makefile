CC=/usr/local/h8/bin/h8300-hms-coff-gcc
OBJCOPY=/usr/local/h8/bin/h8300-hms-coff-objcopy
CFLAGS=-mh -mn -O2 -Wall
LDFLAGS=-nostdlib -nostartfiles -mrelax -Th3694rom.sh -Wl,-Map,$@.map

all : tforth.mot

tforth.mot : tforth.c crt0.S system.o
	$(CC) -o $@.coff $(CFLAGS) $(LDFLAGS) tforth.c crt0.S system.o
	$(OBJCOPY) -Osrec $@.coff $@
	rm -f $@.coff

system.o : system.c system.h
	$(CC) $(CFLAGS) -c $<

clean : 
	rm -f tforth.mot system.o tforth.mot.map
