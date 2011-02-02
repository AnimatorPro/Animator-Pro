## These macros and rules are for the watcom 386 c compiler and Turbo make...
##
## Note that all C programs in the flilib are compiled with the macro
## FLILIB_CODE defined, due to the -D switch on the CC command line.
## This allows small diferences between code shared by Animator and the
## client library to be handled with a conditional section in the code.
##
## A Turbo make feature is employed by callers of this INC file to build
## object libraries.  A command sequence appears as follows:
##	 libname: $(OBJ)
##	     wlib libname &&%
##	 $(OBJ)
##	 %
## This sequence causes Turbo make to create a temp file, dump the contents
## of the $(OBJ) macro into it, then run the wlib program passing the name
## of the temp file in place of the &&%...% sequence.  Other make programs
## may or may not have a similar feature.  If not, you'll have to cobble
## together some .LBC files by hand and tweak all the makefiles to remove
## the turbomake stuff and put in your .LBC filenames.
##

C_IL	= ..\inc\ # include path list for C   (format: \path\;\path\;...)
A_IL	= ..\inc\ # include path list for ASM (format: \path\,\path\,...)

CC  = wcc386p

#add -d2 to the CPF to get debugging info
CPF = -j -3s -oxt -fpi -w2 -DFLILIB_CODE -fi=compiler.h 

AS  = 386asm
APF = -80386 -NOLIST -TWOCASE

.c.obj:
	$(CC) $< $(CPF) -I$(C_IL) $(EXTCPF) $(CFLAGS)

.asm.obj:
	$(AS) $< $(APF) -i $(A_IL) $(EXTAPF) $(AFLAGS)

