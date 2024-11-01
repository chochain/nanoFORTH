#
# To build ROM only
# 1) make rom
#
CXX      = g++
CXXFLAGS = -std=c++17 -O2 -Wall

EXE  = tests/nanoforth
OBJS = \
	src/n4_intr.o \
	src/n4_core.o \
	src/n4_asm.o  \
	src/n4_vm.o   \
	src/n4.o

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -Isrc -c -o $@ $<

all: $(EXE)

$(EXE): $(OBJS)
	$(CXX) -o $@ $^

clean:
	rm $(EXE) $(OBJS)


