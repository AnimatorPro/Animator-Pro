#include <stdio.h>
#include "menus.h"
#include "rastext.h"

/***************** menu seebg functions ******************/

static void mw_setmc(Menuwndo *m,int menucolor)
{
	pj_set_rast(m,m->w.W_screen->mc_colors[menucolor]);
}
void seebg_none(Menuwndo *m)
{
	(void)m;
}
void seebg_white(Menuwndo *m)
/* default menu background drawer white box with grey border box */
{
	mw_setmc(m,MC_WHITE);
	draw_quad((Raster *)m, m->w.W_screen->SGREY, 0, 0, m->w.width, m->w.height);
}
void seebg_bblack(Menuwndo *m)
/* default menu background drawer black box */
{
	mw_setmc(m,MC_BLACK);
}
void seebg_ulwhite(Menuwndo *m)
/* menu background with white box and grey underline */
{
	mw_setmc(m,MC_WHITE);
	pj_set_hline(m,m->w.W_screen->SGREY,0,m->w.height - 1, m->w.width);
}

/***************** seeme functions *****************************/
/*  _ONLY_ to be called when menus are open and not hidden  */

/* these functions get special colors for states of the button */

Rscale *mb_get_menu_scale(Button *b)
{
Menuhdr *mh;

	mh = get_button_hdr(b);
	return(&(mh->group->screen->menu_scale));
}
int mb_mscale_x(Button *b,int x)
/* usings button's screen scale, scale an x value */
{
Rscale *sc;
	sc = mb_get_menu_scale(b);
  	return(rscale_x(sc,x));
}
int mb_mscale_y(Button *b,int y)
/* usings button's screen scale, scale an y value */
{
Rscale *sc;
	sc = mb_get_menu_scale(b);
  	return(rscale_y(sc,y));
}
void mb_make_clip(Button *b,Clipbox *cb)
/* makes a clipbox to button's size */
{
	pj_clipbox_make(cb, (Raster *)(b->root), b->x, b->y, b->width, b->height);
}
void mb_make_iclip(Button *b,Clipbox *cb)

/* makes clipbox for inside of bordered button 1 pixel size border */
{
	pj_clipbox_make(cb, (Raster *)(b->root), b->x+MB_IBORDER, b->y+MB_IBORDER, 
				 b->width-2*MB_IBORDER, b->height-2*MB_IBORDER);
}
static Boolean is_hilit(Button *b)
{
	switch(b->flags & (MB_HILITE_TYPES | MB_HILIT))
	{
		case MB_NOHILITE:
			return(FALSE);
		case MB_GHILITE:
			return(b->group && (*((SHORT *)(b->group)) == b->identity));
		case MB_B_GHILITE:
			return(b->group && (*((BYTE *)(b->group)) == (BYTE)(b->identity)));
		default:
			return(TRUE);
	}
}
void mb_set_hilite(Button *b,Boolean hilite)
{
	if(hilite)
		b->flags |= MB_HILIT;
	else
		b->flags &= ~MB_HILIT;
}
static int wbg_backcolor(Button *b)
/* returns current "bright" color for button */
{
Wscreen *s = b->root->w.W_screen;

	if(is_hilit(b))
		return(s->SBRIGHT);
	return(s->SWHITE);
}
static int mb_backcolor(Button *b)
/* returns current background color for button */
{
Wscreen *s = b->root->w.W_screen;

	if(is_hilit(b))
		return(s->SBRIGHT);
	return(s->SBLACK);
}
static int mb_textcolor(Button *b)
/* returns "Text" color number for button */
{
Wscreen *s = b->root->w.W_screen;

	if(b->flags & MB_DISABLED)
		return(s->SGREY);
	if(is_hilit(b))
		return(s->SRED);
	return(s->SWHITE);
}
int wbg_textcolor(Button *b)
/* returns highlight color number for button */
{
Wscreen *s = b->root->w.W_screen;

	if(b->flags & MB_DISABLED)
		return(s->SGREY);
	if(is_hilit(b))
		return(s->SRED);
	return(s->SBLACK);
}

/* to get colors from button via root screen */

int mc_bright(Button *b)
{
	return(b->root->w.W_screen->SBRIGHT);
}
int mc_white(Button *b)
{
	return(b->root->w.W_screen->SWHITE);
}
int mc_red(Button *b)
{
	return(b->root->w.W_screen->SRED);
}
int mc_grey(Button *b)
{
	return(b->root->w.W_screen->SGREY);
}
int mc_black(Button *b)
{
	return(b->root->w.W_screen->SBLACK);
}

void color_block1(Button *m,SHORT color) 
/* block 1 pixel less than button size */
{
	pj_set_rect(m->root,color,m->x+1,m->y+1,m->width-2,m->height-2);
}
void m2color_block(Button *m,SHORT fcolor,SHORT bcolor)
{
	a_frame(m,fcolor);
	color_block1(m,bcolor);
}
void a_block(Button *b,Pixel color)
{
	pj_set_rect(b->root,color,b->x,b->y,b->width,b->height);
}
void mc_block(Button *b,int mc_color)
{
	a_block(b,b->root->w.W_screen->mc_colors[mc_color]);
}
void a_frame(Button *b,Pixel color)
{
	draw_quad((Raster *)b->root, color, b->x, b->y, b->width, b->height);
}
void mc_frame(Button *b,int mc_color)
{
	a_frame(b,b->root->w.W_screen->mc_colors[mc_color]);
}
void safe_mc_frame(Button *b, int mc_color)
/* this tests if no button or menu window closed */
{
	if(b != NULL && (b->flags & MB_ROOTISWNDO))
		mc_frame(b,mc_color);
}
void white_block(Button *b)
{
	mc_block(b,MC_WHITE);
}
void black_block(Button *b)
{
	mc_block(b,MC_BLACK);
}
void grey_block(Button *b)
{
	mc_block(b,MC_GREY);
}

/* seeme functions */

static void box_cut_corner(void *rast,SHORT x,SHORT y,
									  USHORT w,USHORT h,Pixel color)
{
	pj_set_hline(rast, color, x+1, y, w-2);
	pj_set_hline(rast, color, x+1, y+h-1, w-2);
	pj_set_vline(rast, color, x, y+1, h-2);
	pj_set_vline(rast, color, x+w-1, y+1, h-2);
}
static void ccorner_inside(void *rast,int x,int y,int w,int h,int color)
{
	pj_set_rect(rast,color,x+1, y+1, w-2, h-2);
}
static void mb_ccorner(Button *b,int color)
{
	box_cut_corner(b->root,b->x,b->y,b->width,b->height,color);
}
void mb_isquare(Button *b,int color)
{
	ccorner_inside(b->root,b->x,b->y,b->width,b->height,color);
}
void mb_dcorner(Button *b,int color)
{
	box_diag_corner(b->root,b->x,b->y,b->width,b->height,color,
		b->root->w.W_screen->bbevel);
}
void mb_dinside(Button *b,int color)
{
	diag_inside(b->root,b->x,b->y,b->width,b->height,color,
		b->root->w.W_screen->bbevel);
}
static void ncorner_back(Button *b)
{
	m2color_block(b, mc_grey(b), mb_backcolor(b));
}
void wbg_ncorner_back(Button *b)
{
	m2color_block(b, mc_grey(b), wbg_backcolor(b));
}
void mb_centimage(Button *b,int color,Image *image)
{
Pixel colors[2];

#ifdef _UNSCALED_
	colors[1] = color;
	tblit_image(image,colors,b->root,
			  	b->x + (b->width - image->width)/2,
			  	b->y + (b->height - image->height)/2);
#endif /* _UNSCALED_ */

USHORT w, h;
Rscale *sc;

	/* calculate integral scale for image and make scale aspect the same */

	colors[1] = color;
	sc = mb_get_menu_scale(b);
	w = sc->xscalep/sc->xscaleq;
	h = sc->yscalep/sc->yscaleq;
	if((w = Min(h,w)) < 1)
		w = 1;
	h = w;
	h *= image->height;
	w *= image->width;

	image_scale_tblit(image,&colors,b->root,
			  			b->x + (b->width - w)/2,
			  			b->y + (b->height - h)/2,
						w,h );
}
void see_centimage(Button *m)
{
	mb_centimage(m, wbg_textcolor(m), m->datme);
}
int mb_centext(Button *b,Pixel color,char *string)

/* centers text in menu button clips top and bottom of text */
{
Raster *rast;
Clipbox cb;
Vfont *f = b->root->font;
int txtwid;

	rast = (Raster *)(b->root);
	pj_clipbox_make(&cb, rast, 0, b->y, rast->width, b->height);
	txtwid = fstring_width(f,string);
	gftext(&cb,f,string,
		   b->x + (1+b->width)/2 - txtwid/2, 
		   font_ycent_oset(f,b->height),
		   color, TM_MASK1 );
	return(txtwid);
}
void black_ctext(Button *b) /* black centered text */
{
	white_block(b);
	mb_centext(b,wbg_textcolor(b),b->datme);
}
void grey_ctext(Button *b) /* grey centered text */
{
	mb_centext(b,mc_grey(b),b->datme);
}

/* Function: mb_leftext
 *
 *  Left justified text.
 */
static void
mb_leftext(Button *b, Pixel color, char *string)
{
Vfont *f = b->root->font;

	if (string == NULL)
		return;

	gftext(b->root,f,string,
		   b->x + (fchar_spacing(f," ")>>1) + 1,
		   b->y + font_ycent_oset(f,b->height),
		   color,TM_MASK1 );
}
void black_ltext(Button *b)
{
	a_block(b,wbg_backcolor(b));
	mb_leftext(b,wbg_textcolor(b),b->datme);
}
void black_label(Button *b)
{
	white_block(b);
	mb_centext(b, mc_black(b),b->datme);
}
void black_leftlabel(Button *b)
{
	white_block(b);
	mb_leftext(b,mc_black(b),b->datme);
}
void ccorner_text(Button *b)
{
	mb_isquare(b,mb_backcolor(b));
	mb_ccorner(b,mc_grey(b));
	mb_centext(b,mb_textcolor(b),b->datme);
}
void dcorner_text(Button *b)
{
	mb_dinside(b,mb_backcolor(b));
	mb_dcorner(b,mc_grey(b));
	mb_centext(b, mb_textcolor(b),b->datme);
}
void ncorner_text(Button *b)
{
	ncorner_back(b);
	mb_centext(b,mb_textcolor(b),b->datme);
}
void wbg_ncorner_text(Button *b)
{
	wbg_ncorner_back(b);
	mb_centext(b,wbg_textcolor(b),b->datme);
}
void wbg_ncorner_ltext(Button *b)
{
	wbg_ncorner_back(b);
	mb_leftext(b,wbg_textcolor(b),b->datme);
}

static void centshort(register Button *b,Pixel fore,SHORT offset)
{
char buf[10];

	sprintf(buf, "%d", *((SHORT *)(b->datme)) + offset);
	mb_centext(b,fore,buf);
}
void gbshortint(Button *b)
/* grey box short int */
{
	mc_block(b,MC_GREY);
	centshort(b,mb_textcolor(b),0);
}
static void ncnum(Button *b,int offset)
{
	mb_isquare(b,mb_backcolor(b));
	mc_frame(b,MC_GREY);
	centshort(b,mb_textcolor(b),offset);
}
void ncorner_short_plus1(Button *b)
{
	mb_isquare(b,wbg_backcolor(b));
	mc_frame(b,MC_GREY);
	centshort(b,wbg_textcolor(b),1);
}
void ncorner_short(Button *b)
{
	ncnum(b, 0);
}

#ifdef SLUFFED
void ccorner_short(Button *b)
{
	mb_isquare(b,mb_backcolor(b));
	mb_ccorner(b,MC_GREY);
	centshort(b,mb_textcolor(b),0);
}
#endif /* SLUFFED */

void wbg_ncorner_image(Button *b)
{
	mc_frame(b,MC_GREY);
	mb_isquare(b,wbg_backcolor(b));
	mb_centimage(b, wbg_textcolor(b), b->datme);
}
void ncorner_image(Button *b)
{
	mc_frame(b,MC_GREY);
	mb_isquare(b,mb_backcolor(b));
	mb_centimage(b, mb_textcolor(b), b->datme);
}

void ccorner_image(Button *b)
{
	mb_ccorner(b,mc_grey(b));
	mb_isquare(b,mb_backcolor(b));
	mb_centimage(b, mb_textcolor(b), b->datme);
}
/****** simple group hilighting and de-highlighting stuff *******/

struct uhdata {
	void *group;
	SHORT change;
	SHORT save_flags;
};

static void uh_group(Button *b,struct uhdata *uhd)

/* unhilights a group of buttons with same identity as *group */
{
	while(b != NULL)
	{
		if(b->group != uhd->group)
			goto next_button;

		switch(b->flags & MB_HILITE_TYPES)
		{
			case MB_GHILITE:
				if (*((SHORT *)(uhd->group)) != b->identity)
					goto next_button;
				break;
			case MB_B_GHILITE:
				if (*((BYTE *)(uhd->group)) != b->identity)
					goto next_button;
				break;
			default:
				goto next_button;
		}
		uhd->save_flags = b->flags;
		if(uhd->change)
			b->flags &= ~MB_HILITE_TYPES;
		draw_buttontop(b);
		b->flags = uhd->save_flags;

	next_button:

		if(b->children != NULL)
			uh_group(b->children,uhd);
		b = b->next;
	}
}

static void unhi_group(Button *mbs, SHORT *mgroup)
{
struct uhdata uhd;

	uhd.group = mgroup;
	uhd.change = 1;
	uh_group(mbs,&uhd);
}
static void hi_group(Button *mbs, SHORT *mgroup)
{
struct uhdata uhd;

	uhd.group = mgroup;
	uhd.change = 0;
	uh_group(mbs,&uhd);
}
void mb_unhi_group(Button *b)
{
	unhi_group(b->root->hdr->mbs,b->group);
}
void mb_hi_group(Button *b)
{
	hi_group(b->root->hdr->mbs,b->group);
}
static void draw_ghi_group(Button *b,void *g)
{
	if(!b)
		return;
	draw_ghi_group(b->next,g);
	draw_ghi_group(b->children,g);
	if(b->group == g && b->flags & (MB_HILITE_TYPES))
		draw_buttontop(b);
}
void mb_draw_ghi_group(Button *b)
/* draws all buttons in same group with a group hilite flag on */
{
	if(get_button_wndo(b))
		draw_ghi_group(b->root->hdr->mbs,b->group);
}
void mh_draw_group(Menuhdr *mh, void *group)
/* draws all buttons with group */
{
	if(mh->mw != NULL)
		draw_ghi_group(mh->mbs,group);
}
void toggle_bgroup(Button *b)
{
	switch(b->flags & MB_HILITE_TYPES)
	{
		case MB_GHILITE:
			*((SHORT *)(b->group)) = !*((SHORT *)(b->group));
			break;
		case MB_B_GHILITE:
			*((BYTE *)(b->group)) = !*((BYTE *)(b->group));
			break;
		default:
			return;
	}
	mb_draw_ghi_group(b);
}
void change_mode(Button *b)
{
Button *mbs;

	mbs = get_button_hdr(b)->mbs;
	unhi_group(mbs, b->group);
	switch(b->flags & MB_HILITE_TYPES)
	{
		default:
		case MB_GHILITE:
			*((SHORT *)(b->group)) = b->identity;
			break;
		case MB_B_GHILITE:
			*((BYTE *)(b->group)) = (BYTE)(b->identity);
			break;
	}
	hi_group(mbs, b->group);
}

void hilight(Button *m)

/* explicitly one time hilites a button group pointer */
{
USHORT oflags;

	if (m->seeme != NULL)
	{
		oflags = m->flags;
		m->flags |= MB_HILIT;
		draw_buttontop(m);
		m->flags = oflags;
	}
}
/********** attachment of standard menu parts to other buttons ********/

/* these are called as seemes they will move children to relative offset
 * to button attached to when the seeme is called. these children will
 * be drawn after since the children's seemes are called after the parent */

struct hchiledat {
	SHORT dx, dy;
	Menuwndo *root;
	USHORT orflags;
	Boolean load_group;
	void *group;	
	Rscale menu_scale;
};

static void move_link_children(Button *b, struct hchiledat *hcd)
{
	while(b)
	{
		b->root = hcd->root;
		b->flags = (b->flags & ~(MB_ROOTISWNDO)) | hcd->orflags;
		scale_button(b,&hcd->menu_scale);
		move_link_children(b->children,hcd);
		b->x += hcd->dx;
		b->y += hcd->dy;
		if(b->flags & MB_GHANG)
			b->group = hcd->group;
		b = b->next;
	}
}
void mb_hang_chiles_oset(Button *m,SHORT xoset, SHORT yoset)
{
struct hchiledat hcd;
Button *child;

	if(m == NULL || (child = m->children) == NULL)
		return;

	hcd.orflags = m->flags & MB_ROOTISWNDO;
	hcd.menu_scale = m->root->w.W_screen->menu_scale;
	scale_button(child,&hcd.menu_scale);
	hcd.dx = xoset + (m->x - child->x);
	hcd.dy = yoset + (m->y - child->y);
	hcd.root = m->root;
	hcd.group = m->group;
	move_link_children(child,&hcd);
}
void hang_children(Button *b)
{
	mb_hang_chiles_oset(b,0,0);
}
void bg_hang_children(Button *b)
/* draws 2color background then hangs children */
{
	wbg_ncorner_back(b);
	hang_children(b);
}
