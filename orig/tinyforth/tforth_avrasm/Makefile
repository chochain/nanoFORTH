AS=avrasm2
AFLAGS=-fI

all : tforth.hex

tforth.hex : tforth.asm
	$(AS) $(AFLAGS) $<

clean : 
	rm -f tforth.hex
