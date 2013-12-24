## makefile for Watcom C using -3s calling standard

EXTCPF = -w3 -3s
LFILE  = std.lnk

C_IL	= ..\	  # include path list for C   (format: \path\;\path\;...)
A_IL	= ..\	  # include path list for ASM (format: \path\,\path\,...)

CC	= wcc386
CPF = -j -fpi -w2 -DFLILIB_CODE
# removed -oxt to reduce memory usage

AS	= 386asm
APF = -80386 -NOLIST -TWOCASE

.c.obj:
	$(CC) $< $(CPF) -I$(C_IL) $(EXTCPF) $(CFLAGS)

.asm.obj:
	$(AS) $< $(APF) -i $(A_IL) $(EXTAPF) $(AFLAGS)


FLH = ..\pjltypes.h ..\pjprotos.h ..\pjstypes.h ..\pjecodes.h \
	  ..\pjbasics.h ..\flic386p.lib

all: flicinfo.exp stepflic.exp flicplay.exp
	@echo Phew - made all the fliclib internal test programs.

flicinfo.exp flicinfo: flicinfo.obj $(FLH)
	wlink @$(LFILE) FILE flicinfo.obj NAME flicinfo.exp

stepflic.exp stepflic: stepflic.obj $(FLH)
	wlink @$(LFILE) FILE stepflic.obj NAME stepflic.exp

flicplay.exp flicplay: flicplay.obj $(FLH)
	wlink @$(LFILE) FILE flicplay.obj NAME flicplay.exp

linktest.exp linktest: linktest.obj $(FLH)
	wlink @$(LFILE) FILE linktest.obj FILE ..\flic386p.lib NAME linktest.exp OPTION MAP
	del linktest.exp # don't need exe, just link map
	del linktest.obj # don't need obj
