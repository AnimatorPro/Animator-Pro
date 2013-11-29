
CFLAGS= -Z -O -w-par -wstv -c -ml -I$(INCLUDE) -I. -I..\COMMON -d

.c.obj:
	tcc $(CFLAGS) $*.c
.asm.obj:
	tasm /Ml $*.asm;

#.c.obj:
#	wcc386 $*.c /c

O= a3d.obj a3ddat.obj a3dpull.obj alt.obj arctan.obj \
auto.obj autoseg.obj backward.obj \
bfile.obj blue.obj browse.obj brush.obj \
circle.obj cel.obj cfont.obj choices.obj \
cleanup.obj cline.obj \
cluster.obj colorave.obj commonst.obj comp.obj comprs.obj \
config.obj cpack.obj csort.obj cthread.obj cursorda.obj \
decoder.obj diag.obj diskerr.obj dissolve.obj dosstuff.obj \
drawmode.obj ellipse.obj emmchek.obj \
fccomp.obj feelmenu.obj filemenu.obj filedata.obj files.obj \
findfree.obj fli.obj flood.obj freem.obj \
gel.obj geometry.obj gfx.obj gif.obj globals.obj grid.obj \
hlstorgb.obj init.obj initcmap.obj input.obj int.obj \
jfile.obj lccomp.obj \
onepal.obj options.obj palet2.obj palpull.obj peekpoke.obj pentools.obj \
polytool.obj pstamp.obj pull.obj qnumber.obj quickdat.obj \
macro.obj main.obj MAINPULL.OBJ marqi.obj mask.obj memory.obj \
morfdata.obj morfface.obj morfmenu.obj morfobj.obj morfpull.obj \
multimen.obj names.obj overlay.obj random.obj render.obj \
rfont.obj rotate.obj savefli.obj savepic.obj \
scanedge.obj screen.obj scroller.obj \
seeme.obj seemenu.obj selbrush.obj sep.obj \
sintab.obj sixhi.obj \
slider.obj spiral.obj splice.obj spline.obj \
sqrroot.obj status.obj stretch.obj stringq.obj summa.obj \
tablet.obj \
tempflx.obj textbox.obj textedit.obj textwind.obj \
thikline.obj time.obj timemenu.obj title.obj \
truecolor.obj tseg.obj \
tween.obj \
vip.obj vpaint.obj vpsubs.obj \
wacom.obj window.obj wordwrap.obj writefli.obj \
zoom.obj \
	..\COMMON\blit8_.obj   \
	..\COMMON\cblock_.obj  \
	..\COMMON\clipit_.obj  \
	..\COMMON\closest_.obj \
	..\COMMON\comp_.obj    \
	..\COMMON\peekpok_.obj \
	..\COMMON\unbrun_.obj


fast: $(O)
	tlink $(LIB)c0l @linkit, v, v, $(LIB)cl

compile: $(O)
	@echo All compiled!


