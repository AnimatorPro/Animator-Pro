#-----------------------------------------------------------------------------
# High C. Makefile for FLC386PS example programs.  Requires Phar Lap
# DOS extender as well as High C.
#-----------------------------------------------------------------------------

LFILE = ..\flc386ps.lib 	# pharlap extender stack-parms library

FLH = ..\pjltypes.h ..\pjprotos.h ..\pjstypes.h ..\pjecodes.h ..\pjbasics.h $(LFILE)

all: circflic.exp copyflic.exp custom.exp playfram.exp quickflc.exp ramflic.exp
	@echo Phew - made all the fliclib internal test programs.

circflic.exp: circflic.c $(FLH)
	hc386 circflic.c -I.. -l$(LFILE)

copyflic.exp: copyflic.c $(FLH)
	hc386 copyflic.c -I.. -l$(LFILE)

farcopy.obj: farcopy.asm
	386asm farcopy.asm -NOLIST -TWOCASE

custom.exp: custom.c farcopy.obj $(FLH)
	hc386 custom.c farcopy.obj -I.. -l$(LFILE) -DUSING_PHARLAP

clocktst.exp: clocktst.c $(FLH)
	hc386 clocktst.c -I.. -l$(LFILE)

playfram.exp: playfram.c $(FLH)
	hc386 playfram.c -I.. -l$(LFILE)

quickflc.exp: quickflc.c $(FLH)
	hc386 quickflc.c -I.. -l$(LFILE)

ramflic.exe: ramflic.c $(FLH) ..\pjfli.h
	hc386 -Hloc ramflic.c -I.. -l$(LFILE)
