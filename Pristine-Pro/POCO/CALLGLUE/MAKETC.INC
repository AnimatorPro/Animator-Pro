## these macros are for the turbo c compiler

CFLAGS= -O -wstv -c -ml -d 

# cc and "c post flags"
CC = tcc $(CFLAGS)
CPF = 

.c.obj:
	tcc $(CFLAGS) $*.c $(CPF) 


AFLAGS = /Ml
APF = ;
AS = masm $(AFLAGS)

.asm.obj:
	$(AS) $*.asm $(APF)

