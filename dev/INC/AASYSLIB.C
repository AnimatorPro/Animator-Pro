#include <stdio.h>
#include "aasyslib.h"
#include "ptrmacro.h"
#include "makehdr.c"

char fmt1[] = "sysl_%s equ +0%xH\n";

#define sloset(f) sfoset(fmt1,Syslib,f,f)

main(int argc,char **argv)
{
	openit(argc,argv);

	sloset(malloc);
	sloset(zalloc);
	sloset(free);
	sloset(memset);
	sloset(memcpy);
	sloset(memcmp);
	sloset(strcpy);
	sloset(strlen);
	sloset(strcmp);
	sloset(pj_get_path_suffix); 
	sloset(pj_get_path_name);

	sloset(pj_rex_load);
	sloset(pj_rex_free);
 	sloset(pj_rexlib_load);
	sloset(pj_rexlib_init);
	sloset(pj_rexlib_free);

	sloset(pj_ioerr);
	sloset(pj_open);
	sloset(pj_create);
	sloset(pj_close);
	sloset(pj_read);
	sloset(pj_write);
	sloset(pj_seek);
	sloset(pj_tell);
	sloset(pj_delete);
	sloset(pj_rename);
	sloset(pj_exists);

	sloset(pj_clock_1000);

	sloset(boxf);
	closeit();
}
