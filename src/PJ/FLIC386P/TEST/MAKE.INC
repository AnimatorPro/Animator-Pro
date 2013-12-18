## These macros and rules are for the watcom 386 c compiler and Turbo make...
##

C_IL	= ..\	  # include path list for C   (format: \path\;\path\;...)
A_IL	= ..\	  # include path list for ASM (format: \path\,\path\,...)

CC  = wcc386
CPF = -j -3s -fpi -w2 -DFLILIB_CODE
# removed -oxt to reduce memory usage

AS  = 386asm
APF = -80386 -NOLIST -TWOCASE

.c.obj:
	$(CC) $< $(CPF) -I$(C_IL) $(EXTCPF) $(CFLAGS)

.asm.obj:
	$(AS) $< $(APF) -i $(A_IL) $(EXTAPF) $(AFLAGS)

