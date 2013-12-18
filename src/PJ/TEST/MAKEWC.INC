# include directory
IL = \paa\inc\\

## these macros are for the watcom 386 c compiler

CC = wcc386 $(CFLAGS)
CPF = -j -3s -fpi -w2 -I$(IL) $(EXTCPF)
# removed -oxt to reduce memory usage

.c.obj:
	$(CC) $*.c $(CPF) 

.c.lst:
	$(CC) $*.c -I$(IL) $(CPF) -d1
	wdisasm $*.obj -l -s -a

AS = $(AFLAGS) 386asm
APF = -NOLIST -TWOCASE -i $(IL) $(EXTAPF)

.asm.obj:
	$(AS) $* $(APF) 

