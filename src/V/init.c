
/* init.c - Stuff done once only during program start-up.  Entry-point
   and only global - init_sys.c.   Crosslinks with cleanup.c */


#include <stdio.h>
#include <dos.h>
#include "jimk.h"
#include "flicmenu.h"
#include "init.str"
#include "jfile.h"
#include "peekpok_.h"
#include "ptr.h"

extern int ivmode;
extern char init_drawer[71];
extern union regs mouse_regs;


static
early_err(s)
char *s;
{
puts(s);
puts(init_100 /* "Hit <enter> to continue" */);
getchar();
}

/* set us to 13H 320x200x256 */
static
new_video()
{
union regs r;

r.b.ah = 0;
r.b.al = 0x13;
sysint(0x10,&r,&r);
}

extern char uf_in_emm;
extern int emm_handle;

static
alloc_uf()
{
union regs r;
int i;

if (emm_present())
	{
	/* get emm page pointer... */
	r.b.ah = 0x41;
	sysint(0x67,&r,&r);
	if (r.b.ah == 0)	/* no error */
		{
		uf.p = make_ptr(0,r.w.bx);
		/* now allocate 4 pages */
		r.b.ah = 0x43;
		r.w.bx = 4;
		sysint(0x67,&r,&r);
		if (r.b.ah == 0)
			{
			emm_handle = r.w.dx;
			uf_in_emm = 1;
			for (i=0; i<4; i++)
				{
				r.b.ah = 0x44;
				r.b.al = i;
				r.w.bx = i;
				r.w.dx = emm_handle;
				sysint(0x67,&r,&r);
				}
			return(1);
			}
		}
	}
if ((uf.p = laskmem(64016L)) != NULL)
	{
	uf.p = make_ptr(0, ptr_seg(uf.p)+1); /* make sure uf.p is on a paragraph */
	return(1);
	}
return(0);
}

static
init_mem()
{
register unsigned size;
unsigned err;
union regs r;
char *pool;

#ifdef CHECKIT
copy_bytes(NULL, lomem, 4);
#endif /* CHECKIT */
r.w.bx = 0xffff;	/* ask for a meg.... */
r.b.ah = 0x48;
sysint(0x21,&r,&r);
if (_osmajor >= 5)		/* IBM PC DOS 5.0 wants a little more free memory */
	size = r.w.bx-512;		/* leave 8k for DOS 5.0+*/
else
	size = r.w.bx-256;		/* leave 4K for DOS */
for (;;)
	{
	if (size < 9000)
		{
		early_err(init_101 /* "Not enough memory, sorry" */);
		return(0);
		}
	r.w.bx = size;
	r.b.ah = 0x48;
	if (!(sysint(0x21,&r,&r) & 1))	/* no error, we got it! */
		break;
	size = size*15L/16;
	}
mfree(make_ptr(0,r.w.ax), size);
return(1);
}

static char *nomouse_lines[] =
	{
	NULL,
	init_102 /* "Use arrow keys to move cursor." */,
	init_103 /* "Left shift for left button" */,
	init_104 /* "Right shift for right button" */,
	 NULL,
	};

init_input()
{
char	c;
char *bufs[6];
extern char got_mouse;
char **pt;
char buf[100];

got_mouse = 0;	/* keyboard in charge of input until further notice */
switch (vconfg.dev_type)
	{
	case INP_MMOUSE:	/* it's micky the microsoft mouse */
		mouse_regs.w.ax = mouse_regs.w.bx = 0;
		pt = 0L;
		pt += MOUSEINT;
		if (*pt != NULL)	/* check mouse vector is there */
			mouse_int(0);		/* Initialize the mouse  */
		if (mouse_regs.w.ax != 0xffff)
			{
			nomouse_lines[0] = init_105 /* "Mouse not installed." */;
			}
		else
			got_mouse = 1;
		break;
	case INP_SUMMA2:		/* some summa graphics dudette */
		if (init_summa())
			{
			got_mouse = 1;
			}
		else
			nomouse_lines[0] = init_107 /* "Tablet not installed." */;
		break;
#ifdef WACOM
	case INP_WACOM2:		/* pressure sensitive mouse thing */
		if (!init_wacom())
			{
			nomouse_lines[0] = init_107 /* "Tablet not installed." */;
			}
		else
			{
			got_mouse = 1;
			}
		pressure_sensitive = 1;
		break;
#endif /* WACOM */
	}
if (!got_mouse)
	{
	continu_box(nomouse_lines);
	}
check_input();
init_hr();
return(1);
}

/* stuff to deal with the v.cfg file.  (IE slow-moving system configuration
   things like what's the input device, what's the scratch-file device,
   where are the fonts...)  */
extern char goodconf;
static struct config ivconfg = {VCFGMAGIC,2,0,"", 0, 0};

/* Attempt to read v.cfg file and check file type.  If can't find v.cfg
   then assume default values. */
static
init_config()
{
FILE *jcf;
int dev;
extern char conf_name[];


dev = get_device();
if ((jcf = jopen(conf_name, 0)) != 0)
	{
	if (jread(jcf, &vconfg, sizeof(vconfg)) == sizeof(vconfg) )
		{
		if (vconfg.cmagic == VCFGMAGIC)
			{
			/* deal with variable sized configuration extention */
			zero_structure(&vconfg_ext, sizeof(vconfg_ext));
			if (vconfg.extention_size > 0)
				{
				if (vconfg.extention_size > sizeof(vconfg_ext) )
					vconfg.extention_size = sizeof(vconfg_ext);
				if (jread(jcf, &vconfg_ext, vconfg.extention_size) )
					goodconf = 1;
				}
			else
				goodconf = 1;
			vconfg.extention_size = sizeof(vconfg_ext);
			}
		}
	jclose(jcf);
	}
if (!goodconf)
	{
	copy_structure(&ivconfg, &vconfg, sizeof(vconfg));
	vconfg.scratch_drive = dev;
	zero_structure(&vconfg_ext, sizeof(vconfg_ext) );
	}
path_temps(vconfg.scratch_drive+'A');
}
static ret1()
{
return(1);
}

/* Make sure everybody is there and ready to go.  Return 1 if prospects
   look bright.   The routine to close up everything this guy opens up
   is in cleanup.c as uninit_sys().  Beware though that uninit_sys()
   leaves some things such as memory for MS-DOS to take care of on exit() */
init_sys()
{
WORD i;
long t1, dt;
union regs r;

/* trap control C */
ctrlbrk(ret1);
/* signal(SIGINT, SIG_IGN); portable but big way */

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
	early_err(init_109 /* "Not a VGA/MCGA display, sorry\n" */);
	return(0);
	}
init_config();
copy_structure(&default_vs, &vs, sizeof(vs));
if (!init_mem())
	return(0);
/* grab undo screen */
if (!alloc_uf())
	{
	early_err(init_110 /* "Not enough memory" */);
	return(0);
	}
copy_cmap(init_cmap, sys_cmap);
init_seq();
make_current_drawer();
strcpy(init_drawer, vs.drawer);
if (!init_input())
	return(0);
config_ints();
init_de();

/* find out what devices are connected */
get_devices();

return(1);
}
