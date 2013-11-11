

CFLAGS= -O -w-par -wstv -c -ml -I$(INCLUDE) -d -I. -I..\COMMON

.c.obj:
	tcc $(CFLAGS) $*.c
.asm.obj:
	tasm /Ml $*.asm;


O= blit.obj brush.obj \
cel.obj choices.obj cursorda.obj \
diskerr.obj dosstuff.obj feelmenu.obj \
filedata.obj filemenu.obj gfx.obj \
init.obj initcmap.obj input.obj \
main.obj mmouse.obj names.obj peekpoke.obj rfont.obj \
scroller.obj seeme.obj seemenu.obj sixhi.obj slider.obj stringq.obj \
wordwrap.obj \
crop.obj croppull.obj stubs.obj globals.obj pull.obj memory.obj \
qnumber.obj cpack.obj rgbcmap.obj scale.obj slide.obj \
amigapic.obj unrif.obj readrif.obj \
seq.obj stpic.obj \
pcx.obj \
vision.obj \
macpic.obj \
anim.obj \
writegif.obj gif.obj decoder.obj comprs.obj \
skip.obj fccomp.obj lccomp.obj comp.obj \
fli.obj writefli.obj \
screen.obj jfile.obj bfile.obj \
foolwcc2.obj summa.obj textbox.obj \
	..\COMMON\a1blit_.obj  \
	..\COMMON\a2blit_.obj  \
	..\COMMON\blit8_.obj   \
	..\COMMON\clipit_.obj  \
	..\COMMON\cblock_.obj  \
	..\COMMON\closest_.obj \
	..\COMMON\comp_.obj    \
	..\COMMON\peekpok_.obj \
	..\COMMON\unbrun_.obj

fast: $(O)
	tlink $(LIB)c0l @linkit, crop, crop, $(LIB)cl

overlay: $(O)
	plink86 file $(LIB)c0l lib $(LIB)cl @crop.lnk
