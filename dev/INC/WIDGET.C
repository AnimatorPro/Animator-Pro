#define WIDGET_INTERNALS
#include <stdio.h>
#include "widget.h"
#include "ptrmacro.h"
#include "makehdr.c"

char fmt1[] = "whl_%s equ +%02xH\n";

#define hloset(name) sfoset(fmt1,Widge_rexlib,name,hl.name)

main(int argc,char **argv)
{
	openit(argc,argv);
	hloset(self_free);
	hloset(free_wlib);
	hloset(pwlib);
	name_equ(BRACKOFF);
	closeit();
}



