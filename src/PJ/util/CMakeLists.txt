add_definitions(-DTRD_MEMORY)

## very low level utilities, assembler interfaces etc
set(LOWUTIL_SRC_FILES
	dosecode.c
	i86dswap.c
	i86swap.c
	ptrenorm.c
	)

## temporary file system compatible memory management
set(MEMTRD_SRC_FILES
	memalloc.c
	memtrd.c
	memtrdlo.c
	)

## basic memory oriented utilities
set(MEM_SRC_FILES
	blockall.c
	membkcpy.c
	membnon0.c
	memcask.c
	memcrc.c
	memeallo.c
	memfreez.c
	memgfree.c
	memnulod.c
	mempeek.c
	mempoke.c
	memswap.c
	memzero.c
	)

## string and data oriented utilities
set(STR_SRC_FILES
	newname.c
	parsemi.c
	strclone.c
	strcrc.c
	trstring.c
	txtcmp.c
	txtncmp.c
	upc.c
	wildcmp.c
	)

## string formatting utilities sprintf and the like
set(FORMATF_SRC_FILES
	folenf.c
	fonsprtf.c
	formatf.c
	formatxt.c
	forparse.c
	fosprint.c
	fovnsptf.c
	ftextf.c
	ftextfmt.c
	snftextf.c
	vnftextf.c
	)

## linked list utilities including a shell sort
set(LINKLIST_SRC_FILES
	argparse.c
	dladhead.c
	dladtail.c
	dlclone.c
	dlfndhdr.c
	dlgethd.c
	dlinit.c
	dlinsaft.c
	dlinsbef.c
	dlinscmp.c
	dlisort.c
	dlistlen.c
	dlremnod.c
	dlseehd.c
	dlseetl.c
	dlswap.c
	dltohead.c
	dltotail.c
	nalngest.c
	namein.c
	namesort.c
	shelsort.c
	slfree.c
	slistel.c
	slistix.c
	slistlen.c
	slremel.c
	slrevers.c
	slstjoin.c
	slstlast.c
	slstsort.c
	textin.c
	)

## integer and fixed point math utilities
set(IMATH_SRC_FILES
	imclcdst.c
	imlcmult.c
	imptlrot.c
	imrotate.c
	imrsclby.c
	imsincos.c
	imsqroot.c
	imssclby.c
	imusclby.c
	imintmin.c
	imintmax.c
	imintabs.c
	fixpoint.c
	)

## floating point math utilities
set(FMATH_SRC_FILES
	fmath.c
	)

## utilities library with all modules and temp file (trd) memory allocation
add_library(trdutil
	${FMATH_SRC_FILES}
	${FORMATF_SRC_FILES}
	${IMATH_SRC_FILES}
	${LINKLIST_SRC_FILES}
	${LOWUTIL_SRC_FILES}
	${MEMTRD_SRC_FILES}
	${MEM_SRC_FILES}
	${STR_SRC_FILES})
