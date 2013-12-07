CC := gcc
CFLAGS := -W -Wno-missing-field-initializers -Wno-sign-compare -I. -I../COMMON
LDFLAGS :=
LDLIBS := -lSDL

all: v

OBJS := main.o     \
	a3d.o      \
	a3ddat.o   \
	a3dpull.o  \
	alt.o      \
	arctan.o   \
	auto.o     \
	autoseg.o  \
	backward.o \
	bfile.o    \
	blue.o     \
	browse.o   \
	brush.o    \
	cel.o      \
	cfont.o    \
	choices.o  \
	circle.o   \
	cline.o    \
	cluster.o  \
	commonst.o \
	comprs.o   \
	cpack.o    \
	csort.o    \
	cthread.o  \
	cursorda.o \
	decoder.o  \
	dissolve.o \
	drawmode.o \
	ellipse.o  \
	feelmenu.o \
	filedata.o \
	filemenu.o \
	files.o    \
	findfree.o \
	fli.o      \
	flood.o    \
	freem.o    \
	gel.o      \
	geometry.o \
	gfx.o      \
	gif.o      \
	globals.o  \
	grid.o     \
	hlstorgb.o \
	initcmap.o \
	input.o    \
	jfileerr.o \
	macro.o    \
	mainpull.o \
	marqi.o    \
	mask.o     \
	multimen.o \
	names.o    \
	onepal.o   \
	options.o  \
	overlay.o  \
	palet2.o   \
	palpull.o  \
	pentools.o \
	polytool.o \
	pstamp.o   \
	pull.o     \
	qnumber.o  \
	quickdat.o \
	random.o   \
	render.o   \
	rotate.o   \
	savefli.o  \
	savepic.o  \
	scanedge.o \
	screen.o   \
	scroller.o \
	seeme.o    \
	seemenu.o  \
	selbrush.o \
	sep.o      \
	sintab.o   \
	slider.o   \
	spiral.o   \
	splice.o   \
	spline.o   \
	sqrroot.o  \
	status.o   \
	stretch.o  \
	stringq.o  \
	sys_sdl.o  \
	tempflx.o  \
	textbox.o  \
	textedit.o \
	textwind.o \
	thikline.o \
	time.o     \
	timemenu.o \
	title.o    \
	truecolo.o \
	tseg.o     \
	tween.o    \
	vpaint.o   \
	vpsubs.o   \
	window.o   \
	wordwrap.o \
	writefli.o \
	zoom.o     \
	$(addprefix ../COMMON/, \
	blit8_.o   \
	cblock_.o  \
	clipit_.o  \
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
	truecol_.o \
	unbrun_.o  \
	)

v: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

.PHONY: clean
clean:
	$(RM) v $(OBJS)
