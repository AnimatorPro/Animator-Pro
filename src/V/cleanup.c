
/* cleanup.c - stuff to install and take out interrupt drivers.  Code
   to exit cleanly from vpaint.  Emm support such as it is (will use
   64K, can't be bothered switching banks on the fly.) */

#include "jimk.h"

extern int ivmode;
extern char init_drawer[71];
static char cfi;
char uf_in_emm;		/* undo buffer in EMM? */
int emm_handle;		/* EMM handle if any. */

/* Set up clock interrupt to our custom routine (unless config file tells
   us not to... */
config_ints()
{
if (!cfi)
	{
	if (!vconfg.noint)
		{
#ifdef LATER
		Startvbl();
#endif LATER
		setint();
		fastint();
		}
	cfi = 1;
	}
}

/* Remove any interrupt handlers we've installed */
unconfig_ints()
{
if (cfi)
	{
	if (!vconfg.noint)
		Restoreint();
	cfi = 0;
	}
}


/* set old video mode */
old_video()
{
union regs r;
r.b.ah = 0;
r.b.al = ivmode;
sysint(0x10,&r,&r);
}

/* free up EMM used by undo buffer if any */
static 
free_uf()
{
union regs r;

if (uf_in_emm)
	{
	r.b.ah = 0x45;
	r.w.dx = emm_handle;
	sysint(0x67,&r,&r);
	}
}

cleanup_input()
{
pressure_sensitive = 0;
cleanup_summa();
}

/* Remove interrupt handlers, restore device and directory, change back to
   start-up video mode, and do other miscellanious program cleanup/termination
   stuff */
uninit_sys()
{
union regs r;

close_tflx();
change_dir(init_drawer);
unconfig_ints();
free_uf();
cleanup_input();
old_video();
}

