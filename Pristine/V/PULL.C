/* pull.c - drop-down menu interpreter.  Could tighten this up and
   downsize the Pull structure if wanted to spend a week saving 4K
	sometime.    Arrghh.  It's not too bad considering it's been
	through 3 "windowing" systems since it was first written. */

/* :ts=3 reads better if tab-stop 3.  */
#include "jimk.h"
#include "flicmenu.h"

char menu_ix, sel_ix;

extern Flicmenu fileq_menu;
extern Pull root_pull;

WORD *draw_pull(), *save_behind();
struct pull *cur_pull = &root_pull;


#ifdef SLUFFED
static
max_pull_mem(p)
register Pull *p;
{
UWORD max, acc;

max = 0;
while (p)
	{
	acc = Raster_block(p->width, p->height);
	if (acc > max)
		max = acc;
	p = p->next;
	}
return(max);
}
#endif SLUFFED

in_pblock(x, y, p)
int x, y;
register Pull *p;
{
if (uzx >= x && uzy >= y)
	{
	x += p->width;
	y += p->height;
	if (uzx < x && uzy < y)
		return(1);
	}
return(0);
}

#ifdef SLUFFED
pull_color(x, y, p)
int x, y;
register Pull *p;
{
extern WORD diamond_cursor[];
register WORD color;

color = (int)p->data;
colrop(color, x, y, p->width-1, p->height-1);
if (color == vs.ccolor)
	cdraw_brush(diamond_cursor,
		x+(p->width>>1)-1, y + (p->height>>1)-1, color^255 );
}
#endif /* SLUFFED */


pull_block(x, y, p)
int x, y;
register Pull *p;
{
colrop(swhite, x, y, p->width-1, p->height-2);
hline(y + p->height-1, x, x + p->width-1, sgrey);
}

pull_oblock(x, y, p)
int x, y;
register Pull *p;
{
int x1, y1;

colrop(swhite, x+1, y+1, p->width-3, p->height-3);
x1 = x + p->width - 1;
y1 = y + p->height - 1;
hline(y, x, x1, sgrey);
hline(y1, x, x1, sgrey);
vline(x, y, y1, sgrey);
vline(x1, y, y1, sgrey);
}

pull_text(x, y, p)
int x, y;
Pull *p;
{
gtext(p->data, x+2, y+1, p->disabled ? sgrey : sblack);
}


#ifdef SLUFFED
pull_brush(x, y, p)
int x, y;
register Pull *p;
{
int brush;

brush = (int)p->data;
if (brush == brush_ix)
	colrop(sred, x, y, p->width-1, p->height-1);
draw_brush(brushes[brush], x+3, y+2, p->disabled ? sgrey : swhite);
}
#endif /* SLUFFED */




static WORD cx, cy; /* offset of "menu" level lettering aka amiga terminology */
static WORD ccx, ccy; /* offset of drop downs */
static WORD sx, sy;	/* offset of selection */
static Pull *cchild;
static Pull *select;	
static WORD menu_down;
static WORD sel_hi;
WORD *abehind, *bbehind, *cbehind; /*buffers for
												menu-bar, drop-down, and
												hilit selection */
WORD *mbehind;	/* buffer for behind menu... */

static unselect()
{
if (select)
	{
	undraw_pull(sx, sy, select, cbehind);
	select = NULL;
	sel_hi = -1;
	}
}

static unchild()
{
unselect();
if (cchild)
	{
	undraw_pull(ccx, ccy,  cchild, bbehind);
	cchild = NULL;
	menu_down = -1;
	}
}

static hmpstack;

draw_mp()
{
int ok = 1;
if (hmpstack == 0)
	{
	if (cur_menu)
		{
		if (!sdraw_menu(cur_menu))
			ok = 0;
		}
	if (cur_pull)
		{
		if ((abehind =  
			draw_pull(cur_pull->xoff, cur_pull->yoff, cur_pull)) == NULL)
			ok = 0;
		}
	make_wi_list();
	}
hmpstack++;
return(ok);
}

hide_mp()
{
if (--hmpstack == 0)
	{
	if (cur_pull)
		{
		undraw_pull(cur_pull->xoff, cur_pull->yoff, cur_pull, abehind);
		abehind = NULL;
		}
	if (cur_menu)
		{
		runder_menu(cur_menu, mbehind);
		mbehind = NULL;
		}
	uncheck_cmap();
	make_wi_list();
	zoom_it();
	}
}



interp_pull()
{
WORD cline_size, cbehind_size;  /* dimensions of hilight box in bytes */
WORD x, y;  /* root offset */
WORD i, j;
Pull *child;
Pull *scratch;
WORD scx, scy;	/* scratch (selection) offset */
unsigned char c;
WORD ret;
Pull *p;
char in_cchild;

reuse_input();		/* make nice for loop */
p = cur_pull;
ret = 0;
select = cchild = NULL;
menu_down = sel_hi = -1;
bbehind = cbehind = NULL;
x = p->xoff;
y = p->yoff;
for (;;)
	{
	wait_input();
	if (key_hit)
		{
		reuse_input();
		break;
		}
	if (in_pblock(0,0,p))
		{
		child = p->children;
		i = 0;
		while (child)
			{
			cx = x + child->xoff;
			cy = y + child->yoff;
			if (in_pblock(cx, cy, child))
				{
				if (menu_down != i)
					{
					unchild();
					if ((cchild = child->children) != NULL)
						{
						ccx = cx + cchild->xoff;
						ccy = cy + cchild->yoff;
						if ((bbehind =  draw_pull(ccx, ccy, cchild) ) == NULL)
							{
							goto outta_pul;
							}
						}
					menu_down = i;
					}
				break;
				}
next_child:
			child = child->next;
			i++;
			}
		continue;
		}
	if (cchild  != NULL)
		{
		cchild->height += CH_HEIGHT;	/* let them go a bit below end */
		in_cchild = in_pblock(ccx, ccy, cchild);
		cchild->height -= CH_HEIGHT;
		if (in_cchild)
			{
			scratch = cchild->children;
			j = 0;
			while (scratch)
				{
				scx = ccx + scratch->xoff;
				scy = ccy + scratch->yoff;
				if (in_pblock(scx, scy, scratch))
					{
					if (sel_hi != j)
						{
						unselect();
						select = scratch;
						sx = scx;
						sy = scy;
						sel_hi = j;
						cline_size = cbehind_size = Raster_line(scratch->width);
						cbehind_size *= scratch->height;
						if ( (cbehind = askmem(cbehind_size) ) == NULL)
							{
							goto outta_pul;
							}
						blit8(scratch->width, scratch->height, scx, scy, 
						   vf.p,  vf.bpr, 
							0, 0, cbehind, cline_size);
						draw_frame(sred, scx, scy, scx+scratch->width,
							scy+scratch->height);
						}
					if (PJSTDN)
						{
						if (select != NULL)
							{
							if (in_pblock(sx, sy, select))
								{
								int s1,s2;

								if (!select->disabled)
									{
									menu_ix = menu_down;
									sel_ix = sel_hi;
									ret = 1;
									}
								wait_penup();
								goto outta_pul;
								}
							}
						}
					break;
					}
				scratch = scratch->next;
				j++;
				}
			continue;
			}
		}
	reuse_input();
	break;
	}
outta_pul:
unchild();
return(ret);
}




static
undraw_pull(x, y, p, abehind)
register int x, y;
register Pull *p;
WORD *abehind;
{
WORD abehind_size;
WORD aline_size;

if (abehind != NULL)
	{
	aline_size = abehind_size = Raster_line(p->width);
	abehind_size *= p->height;
	blit8(p->width, p->height, 0, 0, abehind, aline_size, 
		x, y, vf.p, vf.bpr);
	freemem(abehind);
	}
}

static WORD *
save_behind(x, y, p)
register int x, y;
register Pull *p;
{
WORD *abehind;
WORD abehind_size;
WORD aline_size;

aline_size = abehind_size = Raster_line(p->width);
abehind_size *= p->height;
if ( (abehind = askmem(abehind_size) ) == NULL)
	return(NULL);	/* not enough memory */
blit8(p->width, p->height, x, y, vf.p,  vf.bpr, 
	0, 0, abehind, aline_size);
return(abehind);
}


see_pull(x,y,p)
register int x, y;
register Pull *p;
{
(*p->see)(x, y, p);
p = p->children;
while (p)
	{
	(*p->see)(x + p->xoff, y+p->yoff, p);
	p = p->next;
	}
}

static WORD *
draw_pull(x, y, p)
register int x, y;
register Pull *p;
{
WORD *abehind;

if ((abehind = save_behind(x, y, p)) == NULL)
	return(NULL);	/* not enough memory */
see_pull(x,y,p);
return(abehind);
}

#ifdef SLUFFED
unxmenu(p)
register Pull *p;
{
while (p)
	{
	p->data[0] = ' ';
	p = p->next;
	}
}

xone(p, one)
register Pull *p;
register int one;
{
while (--one >= 0)
	p = p->next;
p->data[0] = '*';
}
#endif SLUFFED

xonflag(p, flag)
register Pull *p;
WORD flag;
{
if (flag)
	p->data[0] = '*';
else
	p->data[0] = ' ';
}

enable_pulls(p)
register struct pull *p;
{
if (!p)
	return;
p->disabled = 0;
enable_pulls(p->next);
enable_pulls(p->children);
}

#ifdef LATER
update_mask_pulls()
{
extern Pull usem_pull,createm_pull, snap_pull;

xonflag(&usem_pull, vs.use_mask);
xonflag(&createm_pull, vs.make_mask);
xonflag(&snap_pull, vs.use_grid);
}
#endif LATER

static
which_key_pull(p, c)
Pull *p;
UBYTE c;
{
int abehind;
int ix, i;
UBYTE d;

ix = -1;
p = p->children;
i = 0;
c = upc_char(c);
while (p != NULL)
	{
	d = p->data[0];
	if (d == ' ' || d == '*')
		d = p->data[1];
	d = upc_char(d);
	if (c == d && !p->disabled)
		{
		ix = i;
		break;
		}
	p = p->next;
	i++;
	}
return(ix);
}

static
hidemc()
{
if (cur_menu == NULL)
	uncheck_cmap();
}

pull_keys(p)
Pull *p;
{
int ret;

if ((menu_ix = which_key_pull(p, key_in)) < 0)
	return(0);
cchild = list_el(p->children, menu_ix);
ccx = cchild->xoff;
ccy = cchild->yoff;
cchild = cchild->children;
ccx += cchild->xoff;
ccy += cchild->yoff;
if (cur_menu == NULL)
	find_colors();
if ((bbehind =  draw_pull(ccx, ccy, cchild) ) == NULL)
	{
	hidemc();
	return(0);
	}
hide_mouse();
wait_click();
show_mouse();
ret = -1;	/* got key but not 2nd key */
if (key_hit)
	{
	if ((sel_ix = which_key_pull(cchild, key_in)) >= 0)
		ret = 1;
	}
unchild();
hidemc();
return(ret);
}

