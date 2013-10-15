
CFLAGS= -Z -O -w-par -wstv -c -ml -Ic:\tc\include -d

.c.obj:
	tcc $(CFLAGS) $*.c
.asm.obj:
	tasm /Ml $*.asm;

O= comp.obj quickfli.obj int.obj peekpoke.obj unbrun.obj jfile.obj fli.obj


fast: $(O)
	tlink $(LIB)c0l @linkit, quickfli, quickfli, $(LIB)cl

