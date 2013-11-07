
/* init.c  Routines to take the PC into and out of graphics mode, 
	find the mouse,  stow away the startup-directory, etc. */
#include <stdio.h>
#include "jimk.h"
#include "init.str"

extern int ivmode;
extern char init_drawer[];

early_err(s)
char *s;
{
puts(s);
puts(init_100 /* "Hit <enter> to continue" */);
getchar();
}

/* set us to 13H 320x200x256 */
new_video()
{
union regs r;

r.b.ah = 0;
r.b.al = 0x13;
sysint(0x10,&r,&r);
}

struct config vconfg;

read_config()
{
int f;

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
union regs r;

read_config();

/* get old video mode */
r.b.ah = 0xf;
sysint(0x10, &r, &r);
ivmode = r.b.al;


new_video();

/* make sure made int into 13H */
r.b.ah = 0xf;
sysint(0x10, &r, &r);
if (r.b.al != 0x13)
	{
	early_err(init_102 /* "Not a VGA/MCGA display, sorry\n" */);
	return(0);
	}
if (!init_mem())
	return(0);
make_current_drawer();
strcpy(init_drawer, vs.drawer);
if (!init_input())
	return(0);
init_de();
copy_cmap(init_cmap, sys_cmap);
see_cmap();
find_colors();
get_devices();
return(1);
}

