CFLAGS= -c -ml -If:\ -I$(INCLUDE) -I. -I..\COMMON

.c.obj:
	tcc $(CFLAGS) $*.c
.asm.obj:
	tasm /Ml $*.asm;


O= main.obj \
a2blit.obj blit8.obj brush.obj \
cblock.obj clipit.obj closestc.obj comp.obj cursorda.obj \
diskerr.obj dosstuff.obj feelmenu.obj \
filedata.obj filemenu.obj fli.obj gfx.obj \
init.obj initcmap.obj input.obj jfile.obj \
mainpull.obj stubs.obj names.obj peekpoke.obj pull.obj rfont.obj \
prj1.obj scroller.obj seeme.obj seemenu.obj sixhi.obj stringq.obj \
textbox.obj view.obj vbat.obj mainmenu.obj slider.obj gif.obj \
stubs2.obj decoder.obj shrinkin.obj bfile.obj int.obj \
	..\COMMON\a1blit_.obj  \
	..\COMMON\clipit_.obj  \
	..\COMMON\comp_.obj    \
	..\COMMON\peekpok_.obj \
	..\COMMON\unbrun_.obj

play: $(O)
	tlink $(LIB)c0L @linkit, play, play, $(LIB)emu $(LIB)mathl $(LIB)cl

