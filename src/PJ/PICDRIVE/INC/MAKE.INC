## These macros and rules are for the Watcom 386 C compiler and Watcom make...
##
## Note that POE modules MUST be compiled with inline 80387 code (-fpi87)!

C_IL	   = ..\inc\			# include files path for C
A_IL	   = ..\inc\			# include files path for ASM
REXSTARTUP = ..\lib\rexentry.obj	# startup, 1st file in link obj list

CC  = wcc386				# can use wcc386 or wcc386p
AS  = 386asm				# can use 386asm or 386asmp

CPF = -j -3s -oxt -fpi87 -w2		# default compile options
APF = -80386 -NOLIST -TWOCASE		# default assembler options

.c.obj:
	$(CC) $*.c $(CPF) -I$(C_IL) $(EXTCPF) $(CFLAGS)

.asm.obj:
	$(AS) $*.asm $(APF) -i $(A_IL) $(EXTAPF) $(AFLAGS)

