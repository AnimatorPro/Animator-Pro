## These macros and rules are for the Metaware High C compiler and Turbo make...
##
## Note that POE modules MUST be compiled with inline 80387 code (-Hon=387)!

C_IL	   = ..\inc\			    # include files path for C
A_IL	   = ..\inc\			    # include files path for ASM
REXSTARTUP = ..\lib\rexentry.obj	    # startup, 1st file in link obj list

CC  = hc386				    # compiler
AS  = 386asm				    # can use 386asm or 386asmp

CPF = -c "-Hon=387" "-Hon=floating_point"   # default compiler options
APF = -80386 -NOLIST -TWOCASE		    # default assembler options

.c.obj:
	$(CC) $(CPF) -I$(C_IL) $(CFLAGS) $*.c

.asm.obj:
	$(AS) $*.asm $(APF) -i $(A_IL) $(AFLAGS)
