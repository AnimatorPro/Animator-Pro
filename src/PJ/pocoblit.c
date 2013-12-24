/* pocoblit.c - poco library to blit around things, and functions to
   get various screens */

#include "errcodes.h"
#include "jimk.h"
#include "pocoface.h"
#include "pocolib.h"
#include "flicel.h"

extern Poco_lib po_blit_lib;
extern Flicel *thecel;

static void free_allocated_screens(Poco_lib *lib)
/*****************************************************************************
 * library resources cleanup routine
 ****************************************************************************/
{
Dlheader *sfi = &lib->resources;
Dlnode *node, *next;

for(node = sfi->head; NULL != (next = node->next); node = next)
	{
	pj_rcel_free(((Rnode *)node)->resource);
	pj_free(node);
	}
}


extern char dirty_frame, dirty_file;

static void po_dirties(void)
/*****************************************************************************
 * void PicDirtied(void);
 *	allow a poco/poe to indicate that the picscreen has been dirtied.
 ****************************************************************************/
{
	dirties();
	save_undo();
	zoom_it();
}

Popot po_get_screen(void)
/*****************************************************************************
 * Screen *GetPicScreen(void);
 ****************************************************************************/
{
Popot result;

result.min = result.max = NULL;
result.pt = vb.pencel;
return(result);
}

Popot po_get_swap(void)
/*****************************************************************************
 * Screen *GetSwapScreen(void);
 ****************************************************************************/
{
Popot result;

result.min = result.max = NULL;
result.pt = vl.alt_cel;
return(result);
}

Popot po_get_undo(void)
/*****************************************************************************
 * Screen *GetUndoScreen(void);
 ****************************************************************************/
{
Popot result;

result.min = result.max = NULL;
result.pt = undof;
return(result);
}

static Popot po_get_celscreen(void)
/*****************************************************************************
 * Screen *GetCelScreen(void);
 ****************************************************************************/
{
Popot result;

result.min = result.max = NULL;
if (thecel == NULL)
	result.pt = NULL;
else
	result.pt = thecel->rc;
return(result);
}

void po_a_dot(Popot pscreen, int color, int x, int y)
/*****************************************************************************
 * void SetPixel(Screen *s, int color, int x, int y);
 ****************************************************************************/
{
Raster *r;
if ((r = pscreen.pt) == NULL)
	{
	builtin_err = Err_null_ref;
	return;
	}
pj_put_dot(r, color, x, y);
if (r == (Raster *)vb.pencel)
	{
	dirty_file = dirty_frame = 1;
	}
}

int po_a_get_dot(Popot pscreen, int x, int y)
/*****************************************************************************
 * int GetPixel(Screen *s, int x, int y);
 ****************************************************************************/
{
if (pscreen.pt == NULL)
	return(builtin_err = Err_null_ref);
return(pj_get_dot(pscreen.pt, x, y));
}

void po_get_dims(Popot pscreen, Popot width, Popot height)
/*****************************************************************************
 * void GetScreenSize(Screen *s, int *x, int *y);
 ****************************************************************************/
{

if (pscreen.pt == NULL || width.pt == NULL || height.pt == NULL)
	{
	builtin_err = Err_null_ref;
	return;
	}
vass(width.pt,int)	= ((Raster *)pscreen.pt)->width;
vass(height.pt,int) = ((Raster *)pscreen.pt)->height;
}

static void po_copy_screen(Popot s, Popot d)
/*****************************************************************************
 * void CopyScreen(Screen *source, Screen *dest);
 ****************************************************************************/
{
Rcel *scel, *dcel;
Boolean csame;

if ((scel = s.pt) == NULL || (dcel = d.pt) == NULL)
	{
	builtin_err = Err_null_ref;
	return;
	}
if (scel->width != dcel->width || scel->height != dcel->height)
	{
	builtin_err = Err_wrong_res;
	return;
	}
csame = cmaps_same(vb.pencel->cmap,undof->cmap);
pj_rcel_copy(scel, dcel);
if (dcel == vb.pencel)
	{
	see_cmap();
	dirties();
	}
}

void po_swap_screen(Popot s, Popot d)
/*****************************************************************************
 * void TradeScreen(Screen *a, Screen *b);
 ****************************************************************************/
{
Rcel *scel, *dcel;
Boolean csame;

if ((scel = s.pt) == NULL || (dcel = d.pt) == NULL)
	{
	builtin_err = Err_null_ref;
	return;
	}
if (scel->width != dcel->width || scel->height != dcel->height)
	{
	builtin_err = Err_wrong_res;
	return;
	}
csame = cmaps_same(vb.pencel->cmap,undof->cmap);
swap_pencels(scel, dcel);
if (dcel == vb.pencel)
	{
	see_cmap();
	dirties();
	}
}


struct rectpix_parms
	{
	Popot r, pixbuf;
	int x,y,width,height;
	};

Errcode check_rectpix_parms(struct rectpix_parms *p)
/*****************************************************************************
 * service routine
 ****************************************************************************/
{
if (p->width < 0 || p->height < 0)
	return builtin_err = Err_parameter_range;
if (p->r.pt == NULL)
	{
	return(builtin_err = Err_null_ref);
	}
if (Popot_bufcheck(&p->pixbuf,p->width*p->height) < Success)
	{
	return builtin_err;
	}
return(Success);
}

static void po_put_rectpix(struct rectpix_parms p)
/*****************************************************************************
 * void SetBlock(Screen *s, char *pixbuf, int x, int y, int width, int height);
 ****************************************************************************/
{
char *pixbuf = p.pixbuf.pt;

if (check_rectpix_parms(&p) < Success)
	return;
/* We do this one line at a time since put_hseg does clipping but
 * put_rectpix does not. */
while (--p.height >= 0)
	{
	pj_put_hseg(p.r.pt, pixbuf,  p.x, p.y, p.width);
	pixbuf += p.width;
	p.y += 1;
	}
if (p.r.pt == (void *)vb.pencel)
	{
	dirties();
	}
}

static void po_get_rectpix(struct rectpix_parms p)
/*****************************************************************************
 * void GetBlock(Screen *s, char *pixbuf, int x, int y, int width, int height);
 ****************************************************************************/
{

if (check_rectpix_parms(&p) < Success)
	return;
pj_get_rectpix(p.r.pt, p.pixbuf.pt, p.x, p.y, p.width, p.height);
}

struct iconblit_parms
	{
	Popot msource;
	int mbpr;
	int mx, my;
	int width, height;
	Popot dest;
	int dx, dy;
	int color;
	};

static void po_icon_blit(struct iconblit_parms p)
/*****************************************************************************
 * void IconBlit(char *source, int snext, int sx, int sy, int width, int height
 ****************************************************************************/
{

if (p.msource.pt == NULL || p.dest.pt == NULL)
	{
	builtin_err = Err_null_ref;
	return;
	}
if (p.width < 0 || p.height < 0)
	{
	builtin_err = Err_parameter_range;
	return;
	}
p.color &= 0xff;
pj_mask1blit(p.msource.pt, p.mbpr, p.mx, p.my,
	p.dest.pt, p.dx, p.dy, p.width, p.height, p.color);
if (p.dest.pt == (void *)vb.pencel)
	dirties();
}

struct blit_parm
	{
	Popot source;
	int sx, sy;
	int width, height;
	Popot dest;
	int dx, dy;
	};

static void po_blit(struct blit_parm p)
/*****************************************************************************
 * void Blit(Screen *source, int sx, int sy, int width, int height
 ****************************************************************************/
{
if (p.source.pt == NULL || p.dest.pt == NULL)
	{
	builtin_err = Err_null_ref;
	return;
	}
if (p.width < 0 || p.height < 0)
	{
	builtin_err = Err_parameter_range;
	return;
	}
pj_blitrect(p.source.pt, p.sx, p.sy, p.dest.pt, p.dx, p.dy,
	p.width, p.height);
if (p.dest.pt == (void *)vb.pencel)
	dirties();
}

struct keyblit_parm
	{
	Popot source;
	int sx, sy;
	int width, height;
	Popot dest;
	int dx, dy;
	int key_color;
	};

static void po_key_blit(struct keyblit_parm p)
/*****************************************************************************
 * void KeyBlit(Screen *source, int sx, int sy, int width, int height
 ****************************************************************************/
{
if (p.source.pt == NULL || p.dest.pt == NULL)
	{
	builtin_err = Err_null_ref;
	return;
	}
if (p.width < 0 || p.height < 0)
	{
	builtin_err = Err_parameter_range;
	return;
	}
pj_tblitrect(p.source.pt, p.sx, p.sy, p.dest.pt, p.dx, p.dy,
	p.width, p.height, p.key_color);
if (p.dest.pt == (void *)vb.pencel)
	dirties();
}

static Errcode po_alloc_screen(Popot pcel, int w, int h)
/*****************************************************************************
 * ErrCode AllocScreen(Screen **screen, int width, int height);
 ****************************************************************************/
{
Errcode err;
Popot *p;
Rnode *r;

if ((p = pcel.pt) == NULL)
	return(builtin_err = Err_null_ref);
if (NULL == (r = pj_zalloc(sizeof(Rnode))))
	return(builtin_err = Err_no_memory);

if (w < 0 || h < 0)
	return Err_parameter_range;

p->min = p->max = NULL;

if ((err = valloc_anycel(&p->pt, w, h)) < Success)
	{
	pj_free(r);
	return(err);
	}

add_head(&po_blit_lib.resources,&r->node);
r->resource = p->pt;
return(Success);
}

static void po_free_screen(Popot pcel)
/*****************************************************************************
 * void FreeScreen(Screen **screen);
 ****************************************************************************/
{
Popot *p;
Rnode *r;

if ((p = pcel.pt) == NULL)
	{
	builtin_err = Err_null_ref;
	return;
	}

if (NULL == (r = po_in_rlist(&po_blit_lib.resources, p->pt)))
	{
	builtin_err = Err_free_resources;
	return;
	}
rem_node((Dlnode *)r);
pj_free(r);

pj_rcel_free(p->pt);
p->pt = NULL;

}

Popot po_get_physical_screen(void)
/*****************************************************************************
 * Screen *GetPhysicalScreen(void);
 ****************************************************************************/
{
Popot result;

result.min = result.max = NULL;
result.pt = vb.screen;
return(result);
}

void po_set_box(Popot s, int color, int x, int y, int width, int height)
/*****************************************************************************
 *  void    SetBox(Screen *s, int color, int x, int y, int width, int height);
 ****************************************************************************/
{
	if (s.pt == NULL)
		{
		builtin_err = Err_null_ref;
		return;
		}
	pj_set_rect(s.pt, color, x, y, width, height);
	if (s.pt == (void *)vb.pencel)
		dirties();
}

void po_menu_text(Popot s, int color, int xoff, int yoff, Popot text)
/*****************************************************************************
 * void	MenuText(Screen *screen, int color, int xoff, int yoff, char *text);
 * 		Draw text on any screen in font used for menus.
 ****************************************************************************/
{
	if (s.pt == NULL || text.pt == NULL)
		{
		builtin_err = Err_null_ref;
		return;
		}
	gftext(s.pt,vb.screen->mufont,text.pt,xoff,yoff,color,TM_MASK1,0);
	if (s.pt == (void *)vb.pencel)
		dirties();
}

int po_menu_text_width(Popot text)
/*****************************************************************************
 * int	MenuTextWidth(char *text);
 * 		Find out width of text string in menu font.
 ****************************************************************************/
{
	if (text.pt == NULL)
		{
		return (builtin_err = Err_null_ref);
		}
	return(fstring_width(vb.screen->mufont, text.pt));
}

int po_menu_text_height(void)
/*****************************************************************************
 * int	MenuTextHeight(void);
 * 		Get height of menu font.
 ****************************************************************************/
{
	return(tallest_char(vb.screen->mufont));
}

static void po_get_menu_colors(Popot black, Popot grey, Popot light
, Popot bright, Popot red)
/*****************************************************************************
 * void	GetMenuColors(int *black, int *grey, int *light, int *bright, int *red)
 *		Return the colors commonly used for the menus.
 ****************************************************************************/
{
	Pixel *colors = vb.screen->mc_colors;

	if (black.pt == NULL || grey.pt == NULL || light.pt == NULL
	|| bright.pt == NULL || red.pt == NULL)
		{
		builtin_err = Err_null_ref;
		}
	*((int *)(black.pt)) = colors[MC_BLACK];
	*((int *)(grey.pt)) = colors[MC_GREY];
	*((int *)(light.pt)) = colors[MC_WHITE];
	*((int *)(bright.pt)) = colors[MC_BRIGHT];
	*((int *)(red.pt)) = colors[MC_RED];
}

/*----------------------------------------------------------------------------
 * library protos...
 *
 * Maintenance notes:
 *	To add a function to this library, add the pointer and prototype string
 *	TO THE END of the following list.  Then go to POCOLIB.H and add a
 *	corresponding prototype to the library structure typedef therein which
 *	matches the name of the structure below.  When creating the prototype
 *	in POCOLIB.H, remember that all arguments prototyped below as pointers
 *	must be defined as Popot types in the prototype in pocolib.h; any number
 *	of stars in the Poco proto still equate to a Popot with no stars in the
 *	pocolib.h prototype.
 *
 *	DO NOT ADD NEW PROTOTYPES OTHER THAN AT THE END OF AN EXISTING STRUCTURE!
 *	DO NOT DELETE A PROTOTYPE EVER! (The best you could do would be to point
 *	the function to a no-op service routine).  Breaking these rules will
 *	require the recompilation of every POE module in the world.  These
 *	rules apply only to library functions which are visible to POE modules
 *	(ie, most of them).  If a specific typedef name exists in pocolib.h, the
 *	rules apply.  If the protos are coded as a generic array of Lib_proto
 *	structs without an explicit typedef in pocolib.h, the rules do not apply.
 *--------------------------------------------------------------------------*/

PolibScreen po_libscreen = {
po_get_screen,
	"Screen  *GetPicScreen(void);",
po_get_swap,
	"Screen  *GetSwapScreen(void);",
po_get_undo,
	"Screen  *GetUndoScreen(void);",
po_get_celscreen,
	"Screen  *GetCelScreen(void);",
po_alloc_screen,
	"ErrCode AllocScreen(Screen **screen, int width, int height);",
po_free_screen,
	"void    FreeScreen(Screen **screen);",
po_get_dims,
	"void    GetScreenSize(Screen *s, int *x, int *y);",
po_a_dot,
	"void    SetPixel(Screen *s, int color, int x, int y);",
po_a_get_dot,
	"int     GetPixel(Screen *s, int x, int y);",
po_put_rectpix,
	"void    SetBlock(Screen *s, char *pixbuf, int x, int y, int width, int height);",
po_get_rectpix,
	"void    GetBlock(Screen *s, char *pixbuf, int x, int y, int width, int height);",
po_icon_blit,
	"void    IconBlit(char *source, int snext, int sx, int sy, int width, int height, "
				"Screen *dest, int dx, int dy, int color);",
po_blit,
	"void    Blit(Screen *source, int sx, int sy, int width, int height, "
				"Screen *dest, int dx, int dy);",
po_key_blit,
	"void    KeyBlit(Screen *source, int sx, int sy, int width, int height, "
				"Screen *dest, int dx, int dy, int key_color);",
po_copy_screen,
	"void    CopyScreen(Screen *source, Screen *dest);",
po_swap_screen,
	"void    TradeScreen(Screen *a, Screen *b);",
po_dirties,
	"void    PicDirtied(void);",
/* New with Ani Pro 1.5 */
po_get_physical_screen,
	"Screen  *GetPhysicalScreen(void);",
po_set_box,
	"void    SetBox(Screen *s, int color, int x, int y, int width, int height);",
po_menu_text,
	"void	MenuText(Screen *screen, int color, int xoff, int yoff, char *text);",
po_menu_text_width,
	"int	MenuTextWidth(char *text);",
po_menu_text_height,
	"int	MenuTextHeight(void);",
po_get_menu_colors,
	"void	GetMenuColors(int *black, int *grey, int *light, int *bright, int *red);",
};

Poco_lib po_blit_lib = {
	NULL, "Screen",
	(Lib_proto *)&po_libscreen, POLIB_SCREEN_SIZE,
	NULL, free_allocated_screens,
	};

