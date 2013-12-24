# include directory
HOME = \paa\drivekit\\

IL = $(HOME)inc\\

#############
# Makefile Compiler dependency include file for use with the 
# Metaware High (Haught) C compiler 
#
# Updated for version 1.6  Febuary 3 1991 */
#############

# compiler rules

CFLAGS = -c -I$(IL)

CC = hc386 $(CFLAGS)

CPF =

.c.obj:
	$(CC) $*.c $(CPF)

AS = 386asm $(AFLAGS) 
APF = -NOLIST -TWOCASE -i $(IL) $(EXTAPF)

.asm.obj:
	$(AS) $* $(APF)

.c.lst:
	$(CC) $*.c $(CPF)
	wdisasm $*.obj -l -s -a

