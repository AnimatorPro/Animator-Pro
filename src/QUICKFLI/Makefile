CC := gcc
CFLAGS := -W -I. -I../COMMON
LDFLAGS :=
LDLIBS := -lSDL

all: quickfli

OBJS := quickfli.o \
	fli.o      \
	io_sdl.o   \
	$(addprefix ../COMMON/, \
	comp_.o    \
	jfile.o    \
	peekpok_.o \
	unbrun_.o  \
	)

quickfli: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

.PHONY: clean
clean:
	$(RM) quickfli $(OBJS)
