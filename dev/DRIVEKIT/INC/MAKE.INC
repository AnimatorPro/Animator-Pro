
HOME = \paa\drivekit\			# the 'root' for drivekit things

IL = $(HOME)inc\			# where to find C & ASM include files

## these macros are for the watcom 386 c compiler

CC  = wcc386p
CPF = -j -3s -ox -fpi87 -w2 -DREX_LOADABLE

.c.obj:
	$(CC) $(CFLAGS) $*.c $(CPF) -I$(IL) $(EXTCPF)

.c.lst:
	$(CC) $(CFLAGS) $*.c $(CPF) -d1 -I$(IL) $(EXTCPF)
	wdisasm $*.obj -l -s -a

AS = 386asm
APF = -NOLIST -TWOCASE

.asm.obj:
	$(AS) $* $(APF) -i $(IL) $(EXTAPF)

