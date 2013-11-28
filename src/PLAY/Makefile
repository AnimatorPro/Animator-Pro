CC := gcc
CFLAGS := -g -W -I. -I../COMMON
LDFLAGS :=
LDLIBS := -lSDL

all: play

OBJS := main.o     \
	bfile.o    \
	brush.o    \
	cursorda.o \
	debug.o    \
	decoder.o  \
	feelmenu.o \
	filedata.o \
	filemenu.o \
	fli.o      \
	gif.o      \
	gfx.o      \
	init.o     \
	initcmap.o \
	input.o    \
	sys_sdl.o  \
	jfileerr.o \
	mainmenu.o \
	mainpull.o \
	names.o    \
	prj1.o     \
	pull.o     \
	scroller.o \
	seeme.o    \
	seemenu.o  \
	shrinkin.o \
	slider.o   \
	stringq.o  \
	stubs.o    \
	stubs2.o   \
	textbox.o  \
	vbat.o     \
	view.o     \
	$(addprefix ../COMMON/, \
	blit8_.o   \
	cblock_.o  \
	closest_.o \
	clipit_.o  \
	comp_.o    \
	fs.o       \
	fs_unix.o  \
	io_sdl.o   \
	jfile.o    \
	peekpok_.o \
	rfont.o    \
	sixhi.o    \
	unbrun_.o  \
	)

play: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

.PHONY: clean
clean:
	$(RM) play $(OBJS)
