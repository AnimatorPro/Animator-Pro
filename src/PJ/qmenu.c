
/* qmenu.c - build up simple list of selections type menus out of
   strings etc.  */

#include <string.h>
#include "errcodes.h"
#include "rastext.h"
#include "jimk.h"
#include "scroller.h"
#include "softmenu.h"
#include "commonst.h"
#include "ftextf.h"
#include "rastext.h"


static Pixel cgr_result;

static void cgr_inner_colors(struct menuwndo *r,
	Coor x, Coor y, Ucoor width, Ucoor height, int divx, int divy)
{
int i, j;
int jstart, jend, jsize;
int istart, iend;
int color = 0;
int divx2 = divx/2, divy2 = divy/2;

jstart = y;
for (j=1; j<=divy; j++)
	{
	jend = y + (height * j + divy2)/divy;
	jsize = jend - jstart;
	istart = x;
	for (i=1; i<=divx;  i++)
		{
		iend = x + (width * i + divx2)/divx;
		pj_set_rect(r,color,istart,jstart,iend-istart,jsize);
		++color;
		istart = iend;
		}
	jstart = jend;
	}
}

static void cgr_see_palette(Button *b)
{
	mc_frame(b,MC_GREY); /* draw border frame */
	cgr_inner_colors(b->root, b->x+1, b->y+1, b->width-2, b->height-2, 16, 16);
}

static void cgr_pick_color(Button *b)
{
if (ptinside_rect((Rectangle *)(&b->RECTSTART),icb.mx,icb.my,1))
	{
	cgr_result = pj_get_dot(b->root, icb.mx, icb.my);
	mb_close_ok(b);
	}
}

/* Stuff associated with color grid mini menu */
static Button cgr_pal_sel = MB_INIT1(
	NONEXT,
	NOCHILD,
	66,66,0,0,
	NOTEXT,
	cgr_see_palette,
	cgr_pick_color,
	mb_close_cancel,
	NULL,0,
	NOKEY,
	0
	);

static Menuhdr cgr_menu = MENU_INIT0(
	66,66,0,0,		/* width, height, x, y */
	PALETTE_MUID,   	/* id */
	PANELMENU,			/* type */
	&cgr_pal_sel,		/* buttons */
	SCREEN_FONT, 		/* font */
	&menu_cursor,		/* cursor */
	NULL,				/* seebg */
	NULL,					/* dodata */
	NULL,					/* domenu */
	MBPEN|MBRIGHT, 			/* ioflags */
	0,				/* flags */
	NULL,			/* procmouse */
	NULL, 			/* on_showhide */
	NULL,			/* cleanup */
);

static int ab_get_color(Wndo *w)
{
SHORT occolor = vs.ccolor;

	if(JSTHIT(MBRIGHT))
		return(check_pen_abort());
	if(JSTHIT(MBPEN))
		cgr_result = pj_get_dot(vb.screen->viscel, icb.sx, icb.sy);
	close_menu(&cgr_menu);
	return(TRUE);
}

int qcolor()
{
Errcode err;

menu_to_cursor(vb.screen,&cgr_menu);
err = do_reqloop(vb.screen,&cgr_menu,NULL,NULL,ab_get_color);
err = softerr(err, "qcolor");
if (err < Success)
	return(err);
return(cgr_result);
}

void go_color_grid(Button  *b)
{
int color;

	if ((color = qcolor()) >= Success)
	{
		update_ccolor(color);
		draw_buttontop(b); /* this button most likely is a ccolor button */
	}
}

void qfont_text()
/* Call the font requestor using settings file path and scroller setup */
{
Vset_path pinfo;

	vset_get_pathinfo(FONT_PATH,&pinfo);
	font_req(pinfo.path, pinfo.wildcard, &pinfo.scroller_top,
			 &vs.font_height, uvfont, vb.screen, &vs.font_unzag);
	fget_spacing(uvfont, &vs.font_spacing, &vs.font_leading);
	vset_set_pathinfo(FONT_PATH,&pinfo);
}
