
/* mask.c - Routines to maintain and select our screen-sized bitplane
   used mostly for write-protecting pixels.  */
#include "jimk.h"
#include "flicmenu.h"
#include "mask.str"




extern PLANEPTR mask_plane;

save_mask(name)
char *name;
{
if (mask_plane != NULL)
	return(write_gulp(name, mask_plane, (long)MASK_SIZE));
}

load_mask(name)
char *name;
{
free_mask();
if (newmask())
	{
	return(read_gulp(name, mask_plane, (long)MASK_SIZE));
	}
return(0);
}


static
free_mask()
{
gentle_freemem(mask_plane);
mask_plane = NULL;
}

static
qfree_mask()
{
free_mask();
vs.make_mask = vs.use_mask = 0;
set_stencil_ast();
}

static
newmask()
{
free_mask();
if ((mask_plane = begmemc(MASK_SIZE)) != NULL)
	{
	return(1);
	}
return(0);
}

static
qcreate_mask()
{
vs.make_mask = !vs.make_mask;
if (vs.make_mask)
	{
	if (newmask())
		{
		vs.make_mask = 1;
		vs.use_mask = 0;
		}
	}
set_stencil_ast();
}

quse_mask()
{
vs.use_mask = !vs.use_mask;
if (vs.use_mask)
	{
	vs.make_mask = 0;
	if (mask_plane == NULL)
		{
		vs.use_mask = 0;
		}
	}
set_stencil_ast();
}

static
paste1_mask()
{
if (mask_plane != NULL)
	{
	render_full_screen();
	if (make_render_cashes())
		{
		render_bitmap_blit(XMAX,YMAX,0,0,mask_plane,Mask_line(XMAX),0,0);
		free_render_cashes();
		return(1);
		}
	}
return(0);
}

static
qpaste_mask()
{
uzauto(paste1_mask);
}

static
blitmask(color)
int color;
{
a1blit(XMAX,YMAX,0,0,mask_plane,Mask_line(XMAX), 
	0,0,render_form->p, BPR,color);
zoom_it();
wait_a_jiffy(10);
}


static
qshow_mask()
{
if (mask_plane != NULL)
	{
	save_undo();
	blitmask(swhite);
	blitmask(vs.ccolor);
	wait_click();
	unundo();
	zoom_it();
	}
}


static
qinvert_mask()
{
if (mask_plane != NULL)
	{
	xor_words(0xffff, mask_plane, MASK_SIZE/16);
	qshow_mask();
	}
}

static
qgrab_mask()
{
register UBYTE *s, m;
UBYTE *d, c;
int i, j;

if (newmask())
	{
	s = render_form->p;
	d = mask_plane;
	i = MASK_SIZE;
	while (--i >= 0)
		{
		m = 0x80;
		c = 0;
		j = 8;
		while (--j >= 0)
			{
			if (*s++ != vs.inks[0])
				{
				c |= m;
				}
			m>>=1;
			}
		*d++ = c;
		}
	qshow_mask();
	}
}

static char *stencil_options[] = {
	mask_100 /* " Use" */,
	mask_101 /* " Create" */,
	mask_102 /* " Clip" */,
	mask_103 /* " Invert" */,
	mask_104 /* " View" */,
	mask_105 /* " Paste" */,
	mask_106 /* " Release" */,
	mask_107 /* " Files..." */,
	mask_108 /* " Exit Menu" */,
	};


static
stencil_go_files()
{
go_files(10);
}


extern close_menu(), quse_mask(), qcreate_mask(), qgrab_mask(),
	qinvert_mask(), qshow_mask(), qpaste_mask(), qfree_mask(),
	stencil_go_files();

static Vector stencil_feelers[] =
	{
	quse_mask,
	qcreate_mask,
	qgrab_mask,
	qinvert_mask,
	qshow_mask,
	qpaste_mask,
	qfree_mask,
	stencil_go_files,
	close_menu,
	};

static
set_stencil_ast()
{
char *pt;

pt = stencil_options[0];
if (vs.use_mask)
	pt[0] = '*';
else
	pt[0] = ' ';
pt = stencil_options[1];
if (vs.make_mask)
	pt[0] = '*';
else
	pt[0] = ' ';
}

qstencil()
{
set_stencil_ast();
qmenu(mask_109 /* "Mask Menu" */, stencil_options, Array_els(stencil_options),
	stencil_feelers);
}

mgo_stencil()
{
hide_mp();
qstencil();
draw_mp();
}
