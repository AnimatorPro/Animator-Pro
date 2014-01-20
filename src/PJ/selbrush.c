
/* selbrush.c - Menu that has the brush size slider and not much else */

#include "jimk.h"
#include "brush.h"
#include "softmenu.h"

extern Button gel_brush_group;

static void see_bsize_slider(), feel_bsize_slider(), see_circle_button(),
	see_square_button(), see_line_button(), mb_set_brush_type();
	see_brush_sel(), update_brush_sel(), see_lang_slider(),
	zero_lang_slider(), feel_lang_slider(), rclick_line_sel(),
	see_abovetext_qslider();


/* -----------------Block for gel brush group options --------------*/

extern void see_gel_brush();
static Button gel_brush_sel = MB_INIT1(
	NONEXT,
	NOCHILD,
	136,49,1,4,
	NOTEXT,
	see_gel_brush,
	NOFEEL,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);

static void gelb_update()
{
	draw_buttontop(&gel_brush_sel);
}

static Qslider gel_brush_sl = 
	QSL_INIT1( 0, 30, &vs.gel_brush_size, TRUE, gelb_update, leftright_arrs);

static Button gel_brush_sl_sel = MB_INIT1(
	&gel_brush_sel,
	NOCHILD,
	126,11,6,64,
	&gel_brush_sl,
	see_abovetext_qslider,
	feel_qslider,
	NOOPT,
	NODATA,0,
	NOKEY,
	0
	);
extern void see_gel_brush();
Button gel_brush_group = MB_INIT1(
	NONEXT,
	&gel_brush_sl_sel,
	138,82,0,0,
	NODATA,
	NOSEE,
	NOFEEL,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);

/* ------------- block for brush selector options ------------------ */ 

static Button bmu_circle_sel = MB_INIT1(
	NONEXT,
	NOCHILD,
	23,20,115,0,
	NOTEXT,
	see_circle_button,
	mb_set_brush_type,
	mb_set_brush_type,
	&vs.pen_brush_type,CIRCLE_BRUSH,
	NOKEY,
	MB_B_GHILITE|MB_SCALE_ABS
	);
static Button bmu_square_sel = MB_INIT1(
	&bmu_circle_sel,
	NOCHILD,
	23,20,115,19,
	NOTEXT,
	see_square_button,
	mb_set_brush_type,
	mb_set_brush_type,
	&vs.pen_brush_type,SQUARE_BRUSH,
	NOKEY,
	MB_B_GHILITE|MB_SCALE_ABS
	);
static Button bmu_line_sel = MB_INIT1(
	&bmu_square_sel,
	NOCHILD,
	23,20,115,38,
	NOTEXT,
	see_line_button,
	mb_set_brush_type,
	rclick_line_sel,
	&vs.pen_brush_type,LINE_BRUSH,
	NOKEY,
	MB_B_GHILITE|MB_SCALE_ABS
	);

static Button bmu_langlable_sel = MB_INIT1(
	NONEXT,
	NOCHILD,
	37,11,1,44,
	NODATA, /* "Angle", */
	black_label,
	NOFEEL,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	MB_SCALE_ABS
	);

static SHORT lang;
static Qslider lang_sl = 
	QSL_INIT1( -90, 90, &lang, 0,update_brush_sel, leftright_arrs);

static Button bmu_langsl_sel = MB_INIT1(
	&bmu_langlable_sel,
	NOCHILD,
	78,11,38,44,
	&lang_sl,
	see_lang_slider,
	feel_lang_slider,
	zero_lang_slider,
	NOGROUP,0,
	NOKEY,
	MB_SCALE_ABS
	);

static Button bmu_langhang_sel = MB_INIT1(
	&bmu_langsl_sel,
	NOCHILD,
	110,52,3,3,
	&lang_sl,
	NOSEE,
	NOFEEL,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	MB_SCALE_ABSW
	);

static Button bmu_brush_sel = MB_INIT1(
	&bmu_line_sel,
	NOCHILD,   		/* may have langhang attached */
	110,52,3,3,
	NOTEXT,
	see_brush_sel,
	NOFEEL,
	NOOPT,
	NULL,0,
	NOKEY,
	MB_SCALE_ABS	
	);

static SHORT pwidth;
static Qslider bsize_sl = 
	QSL_INIT1( 0, 31, &pwidth, 1,update_brush_sel, leftright_arrs);

static Button bmu_sizesl_sel = MB_INIT1(
	&bmu_brush_sel,
	NOCHILD,
	138,11,0,57,
	&bsize_sl,
	see_bsize_slider,
	feel_bsize_slider,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	MB_SCALE_ABSW
	);
static Button bmu_sizelab_sel = MB_INIT1(
	&bmu_sizesl_sel,
	NOCHILD,
	81,9,1,70,
	NODATA, /* "Brush Size", */
	black_label,
	NOFEEL,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	MB_SCALE_ABS
	);

#ifdef NOT_USED /* at present we can't load and save brushes */
static Button bmu_load_sel = MB_INIT1(
	&bmu_sizelab_sel,
	NOCHILD,
	53,9,82,70,
	"Load",
	ccorner_text,
	NOFEEL,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);
#endif /* NOT_USED */

Button pen_brush_group = MB_INIT1(
	&bmu_sizelab_sel,
	NOCHILD,
	138,82,0,0,
	NOTEXT,
	NOSEE,
	NOFEEL,
	NOOPT,
	NOGROUP,0,
	NOKEY,
    MB_SCALE_ABS	
	);

static Button bmu_hanger_sel = MB_INIT1(
	NONEXT,
	NOCHILD,
	138,82,0,8,
	NODATA, 	/* datme */
	hang_children,
	NOFEEL,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	MB_SCALE_ABS /* flags */
	);

static Button bmu_title_sel = MB_INIT1(
	&bmu_hanger_sel,
	NOCHILD,
	138, 9, 0, 0, 	/* w,h,x,y */
	NODATA, /* "Brush menu", */
	see_titlebar,
	feel_titlebar,
	NOOPT,
	&tbg_moveclose,0,
	NOKEY,
	MB_SCALE_ABS /* flags */
	);

static Menuhdr pbrush_menu = {
	{138,90,0,0,},	/* width, height, x, y */
	BRUSH_MUID,		/* id */
	PANELMENU,		/* type */
	&bmu_title_sel, /* buttons */
	SCREEN_FONT,	/* font */
	&menu_cursor,	/* cursor */
	seebg_white, 	/* seebg */
	NULL,			/* dodata */
	NULL,			/* domenu */
	(MBPEN|MBRIGHT|KEYHIT) /* ioflags */
};

/* nested alloc because these get used in two places in options and in the 
 * selbrush menu.  They must bracket things one free for each alloc so 
 * nesting will remain in sync */

static Smu_button_list sel_smblist[] = {
	{ "title", &bmu_title_sel },
	{ "angle", &bmu_langlable_sel },
	{ "bsize", &bmu_sizelab_sel },
/* texts note first char is 'T' */
	{ "Tgel_size", &gel_brush_sl_sel.group },
};

void *smbs;
char allocs = 0;
Errcode nest_alloc_brush_texts()
{
	if(allocs++ > 0 && smbs != NULL)
		return(Success);
	return(soft_buttons("brush_panels", sel_smblist, 
						 Array_els(sel_smblist), &smbs));
}
void nest_free_brush_texts()
{
	if(--allocs <= 0)
	{
		smu_free_scatters(&smbs);
		allocs = 0;
	}
}

static void see_bsize_slider(Button *b)
{
	pwidth = get_brush_size();
	see_qslider(b);
}
static void brush_redraw(Button *b)
{
extern Menuhdr options_menu;
extern Button sh1_brush_sel;
	if(get_button_hdr(b) == &options_menu)
		draw_buttontop(&sh1_brush_sel);
}
static void feel_bsize_slider(Button *b)
{
	feel_qslider(b);
	vs.use_brush = (pwidth != 0);
	set_brush_size(pwidth);
	brush_redraw(b);
}
void set_pbrush(Button *b)
{
int called_by;

	called_by = (get_button_hdr(b))->wndoid;

	hide_mp();
	if(nest_alloc_brush_texts() < Success)
		goto error;

	bmu_hanger_sel.children = NULL;

	if(vl.ptool->ot.id == GEL_PTOOL)
		bmu_hanger_sel.children = &gel_brush_group; 
	else
		bmu_hanger_sel.children = &pen_brush_group; 

	menu_to_cursor(vb.screen,&pbrush_menu);

	if(called_by == QUICK_MUID || called_by == OPT_MUID)
	{
		do_menuloop(vb.screen,&pbrush_menu,NULL,NULL,check_pen_abort);
	}
	else
		do_reqloop(vb.screen,&pbrush_menu,NULL,NULL,NULL);

error:
	nest_free_brush_texts();
	show_mp();
}
static void see_circle_button(Button *b)
{
	wbg_ncorner_back(b);
	circle(b->root,wbg_textcolor(b),
		   b->x + (b->width>>1),b->y + (b->height>>1),
		   mb_mscale_x(b,14),TRUE);
}
static void see_square_button(Button *b)
{
Coor rad;

	wbg_ncorner_back(b);
	rad = mb_mscale_x(b,7);
	pj_set_rect(b->root,wbg_textcolor(b),b->x + (b->width>>1) - rad,
					 b->y + (b->height>>1) - rad,
					 rad * 2, rad * 2);
}
static void see_line_button(Button *b)
{
Coor rad;
Short_xy cent;

	wbg_ncorner_back(b);
	rad = mb_mscale_x(b,7);
	cent.x = b->x + (b->width>>1);
	cent.y = b->y + (b->height>>1);
	line(b->root,wbg_textcolor(b),cent.x + rad,cent.y - rad,
								  cent.x - rad,cent.y + rad);
}
static void mb_set_brush_type(Button *b)
{
	color_block1(&pen_brush_group,mc_white(b)); 
	set_brush_type(b->identity);
	vs.use_brush = (get_brush_size() != 0);
	draw_buttonlist(&pen_brush_group);
	brush_redraw(b);
}
static void see_brush_sel(Button *b)
{
Short_xy cent;
Clipbox cb;
SHORT size;
SHORT oht;

	oht = b->height;

	if(vs.pen_brush_type == LINE_BRUSH)
	{
		b->children = &bmu_langhang_sel;
		hang_children(b);
		b->height = bmu_langsl_sel.y - b->y - 2;
	}
	else
		b->children = NULL;

	cent.x = (b->width>>1);
	cent.y = (b->height>>1);
	white_block(b);

	if(b->group)
		size = *(SHORT *)(b->group);
	else
	{
		lang = vs.line_brush_angle;
		size = get_brush_size();
	}

	mb_make_clip(b,&cb);

	switch(vs.pen_brush_type)
	{
		case CIRCLE_BRUSH:
			circle(&cb,mc_grey(b),cent.x,cent.y,size+1,TRUE);
			break;
		case SQUARE_BRUSH:
			++size;
			pj_set_rect(&cb,mc_grey(b),
					 cent.x-(size>>1),cent.y-(size>>1),size,size);
			break;
		case LINE_BRUSH:
			draw_line_brush(&cb,&cent,mc_grey(b),size,lang);
			break;
		default:
			break;
	}
	b->height = oht;
}
static void update_brush_sel()
{
void *ogroup;

	ogroup = bmu_brush_sel.group;
	bmu_brush_sel.group = &pwidth;
	draw_buttontop(&bmu_brush_sel);
	bmu_brush_sel.group = NULL;
}
static void see_lang_slider(Button *b)
{
	lang = vs.line_brush_angle;
	see_qslider(b);
}
static void feel_lang_slider(Button *b)
{
	feel_qslider(b);
	vs.use_brush = (vs.line_brush_size > 1);
	vs.line_brush_angle = lang;
	set_brush_type(LINE_BRUSH);
	brush_redraw(b);
}
static void zero_lang_slider(Button *b)
{
	vs.line_brush_angle = lang = 0;
	see_qslider(b);
	set_brush_type(LINE_BRUSH);
	update_brush_sel();
	brush_redraw(b);
}
static void rclick_line_sel(Button *b)
{
	if(vs.pen_brush_type != LINE_BRUSH)
	{
		mb_set_brush_type(b);
		return;
	}

	if(lang >= 90)
		lang = -45;
	else 
		lang = ((((lang + 90)/45)+1)*45)-90;

	vs.line_brush_angle = lang;
	draw_buttontop(&bmu_langsl_sel);
	set_brush_type(LINE_BRUSH);
	update_brush_sel();
	brush_redraw(b);
}
