#----------------------------------------------------------------------------
# variables naming source, object, related, and extra files for poco...
#----------------------------------------------------------------------------

VERSION = 185

OBJ	= main.obj bop.obj token.obj poco.obj runops.obj \
		  pocoop.obj pocoface.obj fold.obj pocodis.obj pocotype.obj \
		  statemen.obj code.obj linklist.obj safefile.obj struct.obj \
		  chopper.obj pp.obj trace.obj declare.obj funccall.obj pocmemry.obj \
		  varinit.obj \
		  ppeval.obj runccall.obj pocoutil.obj \
		  pocolib.obj mathlib.obj strlib.obj \
		  postring.obj dummylib.obj

SRC1	= main.c bop.c token.c poco.c runops.c pocoutil.asm
SRC2	= escape.c pocoop.c pocoface.c fold.c pocodis.c pocotype.c
SRC3	= statemen.c code.c linklist.c safefile.c struct.c
SRC4	= chopper.c pp.c trace.c declare.c funccall.c pocmemry.c varinit.c
SRC5	= ppeval.c runccall.asm pocolib.c mathlib.c strlib.c postring.c dummylib.c
SRC6	= bcrcall.c bcutil.c # special workalike C code for use with TC/BC

HDR1	= *.h

EXTRAS	= pocfiles.inc poco.lnk tcpoco.lnk vers*.doc integrat.doc make*.*
POC	= *.poc
