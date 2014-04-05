## "j" dos device and directory level calls that call dos only
set(JFDEV_SRC_FILES
	jdevlist.c
	)

## file name parsing modules (string functions etc)
set(FPARSE_SRC_FILES
	fpaddsub.c
	fparsdev.c
	fparsdir.c
	fpconcat.c
	fpdirlen.c
	fpgetdir.c
	fpgetful.c
	fpgetnam.c
	fpincnam.c
	fpissuff.c
	fpmkfile.c
	fpnamein.c
	fpnsplit.c
	fppfxlen.c
	fpremnam.c
	fpsetnam.c
	fpspltcp.c
	fpspltpa.c
	fpsuffin.c
	fpsuffix.c
	fpsufget.c
	fpsufhas.c
	fpsufrem.c
	)

## jfile io modules that have ram disk temp file handling and temp file handling
## this calls doso modules for operating system calls

## note fpgetdrv is a filepath module but is here because of the temp drive
## handling
set(JFILETR_SRC_FILES
	fpgetdrv.c
	jfile.c
	rfile.c
	tfile.c
	)

## jfile utilities that call jfile functions
set(JFUTILS_SRC_FILES
	jfclosez.c
	jfcopy.c
	jfcopyin.c
	jfcpdato.c
	jfcpfile.c
	jfcpydat.c
	jfinsert.c
	jfpisfix.c
	jfrdgulp.c
	jfreadec.c
	jfreados.c
	jfwrite0.c
	jfwritec.c
	jfwritos.c
	jfwrtglp.c
	unchcopy.c
	unchread.c
	unchunk.c
	unchupd.c
	wrtchunk.c
	)

set(XFILE_SRC_FILES
	xfile.c
	)

set(FS_UNIX_SRC_FILES
	fs_unix.c
	)

## library with temp and ram file handling file system and all utilities
add_library(trdfile
	${JFDEV_SRC_FILES}
	${FPARSE_SRC_FILES}
	${JFILETR_SRC_FILES}
	${JFUTILS_SRC_FILES}
	${XFILE_SRC_FILES}
	${FS_UNIX_SRC_FILES})
