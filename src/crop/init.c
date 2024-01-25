
/* init.c  Routines to take the PC into and out of graphics mode, 
	find the mouse,  stow away the startup-directory, etc. */
#include <stdio.h>
#include <string.h>
#include "jimk.h"
#include "fs.h"
#include "init.str"
#include "jfile.h"
#include "memory.h"
#include "peekpok_.h"

extern int ivmode;
extern char init_drawer[];

early_err(s)
char *s;
{
puts(s);
puts(init_100 /* "Hit <enter> to continue" */);
getchar();
}

struct config vconfg;

read_config()
{
FILE *f;

if ((f = jopen("aa.cfg",0))!= 0)
	{
	jread(f, &vconfg, (long)sizeof(vconfg));
	jclose(f);
	}
}

init_sys()
{
WORD i;
long t1, dt;

read_config();

if (!init_system())
	return(0);

make_current_drawer(vs.drawer, sizeof(vs.drawer));
strcpy(init_drawer, vs.drawer);

copy_cmap(init_cmap, sys_cmap);
see_cmap();
find_colors();
get_devices();
return(1);
}
