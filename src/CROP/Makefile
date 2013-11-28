CC := gcc
CFLAGS := -W -I. -I../COMMON
LDFLAGS :=
LDLIBS := -lSDL

all: crop

OBJS := main.o     \
	amigapic.o \
	anim.o     \
	bfile.o    \
	blit.o     \
	brush.o    \
	cel.o      \
	choices.o  \
	comprs.o   \
	cpack.o    \
	crop.o     \
	croppull.o \
	cursorda.o \
	decoder.o  \
	feelmenu.o \
	filedata.o \
	filemenu.o \
	fli.o      \
	gfx.o      \
	gif.o      \
	globals.o  \
	init.o     \
	initcmap.o \
	input.o    \
	sys_sdl.o  \
	jfileerr.o \
	macpic.o   \
	main.o     \
	memory.o   \
	names.o    \
	pcx.o      \
	pull.o     \
	qnumber.o  \
	readrif.o  \
	rgbcmap.o  \
	scale.o    \
	screen.o   \
	scroller.o \
	seeme.o    \
	seemenu.o  \
	seq.o      \
	slide.o    \
	slider.o   \
	stpic.o    \
	stringq.o  \
	stubs.o    \
	sys_sdl.o  \
	textbox.o  \
	unrif.o    \
	vision.o   \
	wordwrap.o \
	writefli.o \
	writegif.o \
	$(addprefix ../COMMON/, \
	blit8_.o   \
	cblock_.o  \
	clipit_.o  \
	closest_.o \
	comp_.o    \
	fccomp.o   \
	fs.o       \
	fs_unix.o  \
	io_sdl.o   \
	jfile.o    \
	lccomp.o   \
	peekpok_.o \
	ptr.o      \
	rfont.o    \
	sixhi.o    \
	unbrun_.o  \
	)

crop: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

.PHONY: clean
clean:
	$(RM) crop $(OBJS)
