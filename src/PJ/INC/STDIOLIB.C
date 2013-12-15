#define REXLIB_INTERNALS
#include <stdio.h>
#include "stdiolib.h"
#include "ptrmacro.h"
#include "makehdr.c"

char fmt1[] = "siol_%s equ +0%xH\n";

#define sioset(f) outf(fmt1,#f,OFFSET(Stdiolib,f))

main(int argc,char **argv)
{
	openit(argc,argv);

	sioset(fdata);
	sioset(pj__get_pto_errno);
	sioset(clearerr);
	sioset(feof);
	sioset(ferror);
	sioset(pj_errno_errcode);

	sioset(fopen);
	sioset(fclose);

	sioset(fseek);
	sioset(ftell);
	sioset(fflush);
	sioset(rewind);

	sioset(fread);
	sioset(fgetc);
	sioset(fgets);
	sioset(ungetc);

	sioset(fwrite);
	sioset(fputc);
	sioset(fputs);
	sioset(fprintf);

	sioset(printf);
	sioset(sprintf);

	closeit();
}
