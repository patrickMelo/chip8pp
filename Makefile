#
# Makefile
#

# Common Variables

CXX			= clang++
CXX_FLAGS	= -O3 -std=c++0x -fno-rtti -Wno-sign-compare -Wno-write-strings -Wno-narrowing -D_FILE_OFFSET_BITS=64
DEBUG_FLAGS	= -g3 -DCHIP8_DEBUG=1
INCLUDES	= -I./ $(shell pkg-config --cflags sdl2)
LIBS		= -lm $(shell pkg-config --libs sdl2)
STRIP		= @true
OBJECTS		= Chip8.o Interface.o Main.o

ifndef TYPE
	TYPE = debug
endif

ifndef BITS
	BITS = 64
endif

ifeq ($(BITS), 32)
	ARCH = i686
else
	ARCH = x86_64
endif

ifeq ($(TYPE), debug)
	CXX_FLAGS	+= $(DEBUG_FLAGS)
endif

ifeq ($(TYPE), release)
	STRIP = strip -s
endif

ifeq ($(BITS), 32)
	CXX_FLAGS += -m32
else
	CXX_FLAGS += -m64
endif

# Targets

all: $(OBJECTS)
	$(CXX) $(CXX_FLAGS) $(INCLUDES) $(OBJECTS) $(LIBS) -o Chip8.$(ARCH)
	$(STRIP) ./Chip8.$(ARCH)

clean:
	@find -type f -iname "*.o" -exec rm -fv {} \;

%.o: %.cxx
	$(CXX) $(CXX_FLAGS) $(INCLUDES) -c $< -o $@

help:
	@echo ""
	@echo "Usage: make TYPE=<debug*|release> BITS=<32|64*>"
	@echo ""
