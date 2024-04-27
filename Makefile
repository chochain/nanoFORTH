EM = em++
CC = g++
CC_FLAG = -std=c++17 -O2 -Wall 

EXE  = tests/n4

OBJS = \
	src/n4_intr.o \
	src/n4_core.o \
	src/n4_asm.o \
	src/n4_vm.o \
	src/n4.o

exe: $(EXE)

%.o: %.cpp
	$(CC) $(CC_FLAG) -Isrc -c -o $@ $<

$(EXE): $(OBJS)
	$(CC) -o $@ $^

clean:
	rm $(EXE) $(OBJS)


