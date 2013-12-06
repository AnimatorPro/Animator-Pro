
/* status.c - display continue alert box with program info */

#include <stdio.h>
#include "jimk.h"
#include "status.str"

static char *about_lines[] =
	{
	status_108 /* "    Autodesk Animator" */,
	status_109 /* "  v. 1.02      s/n $$$$$$$$$$$$" */,
	status_110 /* "Copyright 1989,1990,1991 by Jim Kent." */,
	status_111 /* "          3/30/90" */,
	status_112 /* "  Produced exclusively for" */,
	status_113 /* "        Autodesk Inc." */,
	status_114 /* "             By" */,
	status_115 /* "       Yost Group Inc." */,
	status_116 /* cst_ */,
	NULL,
	};

#ifdef LATER
char *credits_lines[] =
	{
	"Programming             Jim Kent",
	"Production             Gary Yost",
	"                     Bob Bennett",
	"Alpha Testing        Jack Powell",
	"                     Doug Thomas",
	"                      Ann Phelan",
	"Moral Support    Heidi Brumbaugh",  
	"                      Eric Lyons",
	"Torture Testing       Chris Kent",
	"                     Terry Fritz",
	"                  Cindy Peringer",
	NULL,
	};
#endif /* LATER */

about()
{
continu_box(about_lines);
#ifdef LATER
continu_box(credits_lines);
#endif /* LATER */
}

