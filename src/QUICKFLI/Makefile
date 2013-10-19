CC := gcc
CFLAGS := -W
LDFLAGS :=
LDLIBS := -lSDL

all: quickfli

OBJS := quickfli.o \
	comp1.o    \
	fli.o      \
	io_sdl.o   \
	jfile.o    \
	peekpok1.o \
	unbrun1.o

quickfli: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

.PHONY: clean
clean:
	$(RM) quickfli $(OBJS)
