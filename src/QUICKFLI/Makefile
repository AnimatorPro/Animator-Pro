
CFLAGS= -Z -O -w-par -wstv -c -ml -Ic:\tc\include -d

.c.obj:
	tcc $(CFLAGS) $*.c
.asm.obj:
	tasm /Ml $*.asm;

O= quickfli.obj \
	comp.obj comp1.obj fli.obj int.obj jfile.obj peekpoke.obj unbrun1.obj

fast: $(O)
	tlink $(LIB)c0l @linkit, quickfli, quickfli, $(LIB)cl

