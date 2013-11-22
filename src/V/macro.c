
/* macro.c - Stuff to help record user input (usually first into a temp file)
   and then play it back without c_input() routine suspecting too much. */

#include <stdio.h>
#include "jimk.h"
#include "fli.h"
#include "macro.str"

#define MACMAGIC 1811


struct machead 
	{
	WORD mmagic;
	long mcount;
	long mindex;
	char realtime;
	char reserved[5];
	};
static struct machead mh;
static Bfile macfile;

struct menu_event {char menuix, selix;};
union menukey { struct menu_event menu; WORD key;};

struct irec
	{
	unsigned char dt;
	unsigned char buttons;
	union menukey mk;
	WORD mouse_x, mouse_y;
	};
static struct irec ir;

int recordall = 1;
char clickonly;
static char realtimemac;
static UWORD macrorpt;
static char defmacro;
char usemacro;
static long itime, oitime;
char inwaittil;

static
close_macfile()
{
bclose(&macfile);
}

start_macro()
{
realtimemac = 0;
make_macro();
}

realtime_macro()
{
realtimemac = 1;
make_macro();
}

static
make_macro()
{
close_macro();
oitime = get80hz();
if (!defmacro)
	{
	if (!bcreate(tmacro_name, &macfile) )
		{
		cant_create(tmacro_name);
		goto BADOUT;
		}
	zero_structure(&mh, sizeof(mh) );
	mh.mmagic = MACMAGIC;
	mh.realtime = realtimemac;
	if (bwrite(&macfile, &mh, sizeof(mh)) < sizeof(mh))
		{
		truncated(tmacro_name);
		goto BADOUT;
		}
	defmacro = 1;
	}
return(1);
BADOUT:
close_macfile();
return(0);
}

use_macro()
{
umacr(1);
}

repeat_macro()
{
int rep;

rep = fhead.frame_count-1;
if (qreq_number(macro_100 /* "Repeat macro how many times?" */, 
		&rep, 1, 100))
	if (rep >= 0)
		umacr(rep);
}

static
umacr(rpt)
WORD rpt;
{
if (usemacro)
	return;
if (defmacro)
	close_macro();
oitime = get80hz();
if (bopen(tmacro_name, &macfile) == 0)
	{
	nomac_defined();
	return;
	}
if (bread(&macfile, &mh, sizeof(mh)) != sizeof(mh))
	{
	truncated(tmacro_name);
	return;
	}
if (mh.mmagic != MACMAGIC)
	{
	nogood_mac();
	return;
	}
usemacro = 1;
mh.mindex = 0;
macrorpt = rpt;
}

static
nogood_mac()
{
continu_line(macro_101 /* "Not a good macro file." */);
}

static
nomac_defined()
{
continu_line(macro_102 /* "No macro recording defined." */);
}


close_macro()
{
if (defmacro)
	{
	defmacro = 0;
	bseek(&macfile, 0L, 0);
	bwrite(&macfile, &mh, sizeof(mh));
	close_macfile();
	}
}

static
read_next_macro()
{
if (mh.mindex == mh.mcount)
	{
	if (--macrorpt <= 0)
		{
		usemacro = 0;
		close_macfile();
		return(0);
		}
	else
		{
		mh.mindex = 0;
		bseek(&macfile, (long)sizeof(mh), 0);
		}
	}
if (bread(&macfile, &ir, sizeof(ir)) != sizeof(ir))
	{
	usemacro = 0;
	close_macfile();
	continu_line(macro_103 /* "Macro file truncated." */);
	return(0);
	}
mh.mindex++;
return(1);
}

/* process macro into input stream*/
get_macro()
{
if (!macfile.fd)
	return;
if (key_hit)
	{
	WORD omx, omy, ob;

	ob = omouse_button;
	mouse_button = 0;
	key_hit = 0;
	if (mouse_on)
		ucursor();
	usemacro = 0;
	if (yes_no_line(macro_104 /* "   Stop Macro Playback?   " */) )
		{
		close_macfile();
		return;
		}
	usemacro = 1;
	omouse_button = ob;
	}
if (inwaittil || ir.buttons & SYNC_FLAG)
	{
	/* dont go on to next record until synced up */
	}
else
	{
	if (!read_next_macro())
		return;
	}
key_hit = 0;
mouse_button = ir.buttons;
if (ir.mk.key)
	{
	key_hit = 1;
	key_in = ir.mk.key;
	}
itime = get80hz();
rwait(get80hz() + ir.dt - (itime - oitime));
oitime = itime;
uzx = ir.mouse_x;
uzy = ir.mouse_y;
}

static
rwait(time)
long time;
{
while (time > get80hz())
	;
}

static
pmac()
{
mh.mcount++;
if (bwrite(&macfile, &ir, sizeof(ir)) != sizeof(ir))
	{
	defmacro = 0;
	continu_line(macro_105 /* "Write error on macro file!" */);
	close_macfile();
	jdelete(tmacro_name);
	}
}

static
pmacro()
{
ir.buttons = mouse_button;
ir.mk.key = (key_hit ? key_in : 0);
if (realtimemac)
	{
	itime = get80hz();
	ir.dt = itime - oitime;
	oitime = itime;
	}
else
	ir.dt = 0;
ir.mouse_x = uzx;
ir.mouse_y = uzy;
pmac();
}


put_macro(click)
int click;
{
if ((defmacro && macfile.fd != 0 && !inwaittil) &&  
	(recordall || key_hit || mouse_button != omouse_button ||
	 (realtimemac || !click) && mouse_moved))
	{
	pmacro();
	}
}

save_macro()
{
char *title;

close_macro();
if (!jexists(tmacro_name))
	nomac_defined();
else
	{
	if ((title = get_filename(macro_106 /* "Save a macro recording file?" */, ".REC"))!= NULL)
		{
		if (overwrite_old(title))
			jcopyfile(tmacro_name, title);
		}
	}
}

load_macro()
{
char *title;
int f;

if ((title =  get_filename(macro_108 /* "Load a macro recording file?" */, 
	".REC"))!= NULL)
	{
	close_macro();
	if ((f = jopen(title, 0)) == NULL)
		{
		cant_find(title);
		return;
		}
	if ((jread(f,&mh,(long)sizeof(mh))) != sizeof(mh) || mh.mmagic != MACMAGIC)
		{
		nogood_mac();
		jclose(f);
		return;
		}
	jclose(f);
	jcopyfile(title, tmacro_name);
	}
}



macrosync()
{
if (defmacro)
	{
	mouse_button |= SYNC_FLAG;
	pmacro();
	}
else if (usemacro)
	{
	while (!(ir.buttons & SYNC_FLAG))
		{
		if (!read_next_macro())
			return;
		}
	ir.buttons &= ~SYNC_FLAG;
	}
}





