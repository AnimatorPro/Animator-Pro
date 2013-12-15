# include directory 
IL = \paa\inc\\

## these macros are for the watcom 386 c compiler

CFLAGS=

# cc and "c post flags"

# CC = wcc386 $(CFLAGS)
# CPF = -I$(IL) -j -3s -ox -fpc -w2

CC = wcc386p $(CFLAGS)
CPF = -I$(IL) -j -3s -ox -fpc -w2

.c.obj:
	$(CC) $*.c $(CPF)

.c.lst:
	$(CC) $*.c $(CPF) -d1
	wdisasm $*.obj -l -s -a

AS = 386asm $(AFLAGS) 
APF = -NOLIST -TWOCASE -i $(IL) 

.asm.obj:
	$(AS) $* $(APF)

MAKEARGS=\
 IL=\paa\$(IL)


