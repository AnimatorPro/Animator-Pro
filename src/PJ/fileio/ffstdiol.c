/** ffstdiol.c - a standard io library jumptable that gets used by loadable
 ** modules.   A bit complicated by having to exist in two versions, one
 ** using the C library stdio routines, and the other PJ local emulators
 ** of the same.  
 **/

#ifdef USE_LFILE
	#include "lstdio.h"
	#include "formatf.h"
	extern int boxf();
	#define fpos_t long
	#define size_t unsigned int
#else
	#include <stdio.h>
	#include <errno.h>
	#define boxf printf
	#define local_sprintf sprintf
	#ifdef __WATCOMC__
		#define get_perrno __get_errno_ptr
	#endif
#endif
#define REXLIB_INTERNALS
#include <stdarg.h>
#include "stdtypes.h"
#include "stdiolib.h"




/********* shels needed for lfile **********/

static void clearerr_shell(FILE *fp)
{
	clearerr(fp);
#undef clearerr
#define clearerr clearerr_shell
}
static int feof_shell(FILE *fp)
{
	return(feof(fp));
#undef feof
#define feof feof_shell
}
static int ferror_shell(FILE *fp)
{
	return(ferror(fp));
#undef ferror
#define ferror ferror_shell
}
Errcode pj_errno_errcode()
{
	return(errno);
}

/****************** interface for remote code ***************/

#ifndef get_perrno
static int *get_perrno()
{
	return(&errno);
}
#endif
/********** we only need this if SEEK codes are different *********/

#if (SEEK_SET != 0) || (SEEK_CUR != 1) || (SEEK_END != 2)

static int fseek_shell(FILE *fp,long int offset,int whence)
{
ststic int whences[] = { SEEK_SET, SEEK_CUR, SEEK_END };

	if((unsigned)whence > 2)
		return(-1);
	return(fseek_shell(fp,offset,whences[whence]);
}
#define fseek fseek_shell

#endif /* ***************** seek modes not same */



Stdiolib aa_stdiolib = {
	/* header */
	{
		sizeof(Stdiolib),
		AA_STDIOLIB, AA_STDIOLIB_VERSION,
	},
	NULL,
	get_perrno,
	clearerr,
	feof,
	ferror,
	pj_errno_errcode,

	fopen,
	fclose,

	fseek,
	ftell,
	fflush,
	rewind,

	fread,
	fgetc,
	fgets,
	ungetc,

	fwrite,
	fputc,
	fputs,
	fprintf,

	boxf,
	local_sprintf,
};

