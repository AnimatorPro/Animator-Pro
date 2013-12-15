# include directory
IL = \paa\rexlib\inc\\

## these macros are for the watcom 386 c compiler

CC = wcc386p $(CFLAGS)
CPF = -j -3s -oxt -fpi87 -w2 -I$(IL) $(EXTCPF)

.c.obj:
	$(CC) $*.c $(CPF) 

.c.lst:
	$(CC) $*.c -I$(IL) $(CPF) -d1
	wdisasm $*.obj -l -s -a

AS = $(AFLAGS) 386asm
APF = -NOLIST -TWOCASE -i $(IL) $(EXTAPF)

.asm.obj:
	$(AS) $* $(APF) 

