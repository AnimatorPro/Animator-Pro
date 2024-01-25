CC := gcc
CFLAGS := -W -I. -I../COMMON
LDFLAGS :=
LDLIBS := -lSDL

all: quickfli

OBJS := quickfli.o \
	fli.o      \
	sys_sdl.o  \
	$(addprefix ../COMMON/, \
	comp_.o    \
	io_sdl.o   \
	jfile.o    \
	peekpok_.o \
	unbrun_.o  \
	)

quickfli: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

.PHONY: clean
clean:
	$(RM) quickfli $(OBJS)
