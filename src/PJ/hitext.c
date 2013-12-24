
#include "errcodes.h"
#include "textedit.h"
#include "jimk.h"

/* hi level vpaint specific text routines */

extern char text_name[];

#ifdef SLUFFED
ted_wndo_doit(Wndo *w)
/* glue  to attatch text editor to window */
{
ted_doit(w->doit_data);
}
#endif /* SLUFFED */

#ifdef SLUFFED
void edit_text_button(Button *b)
/* glue to attatch text editor to  a button */
{
Text_file *gf = b->datme;

copy_rectfields(b, &gf->twin);
edit_text_file(gf);
}
#endif /* SLUFFED */

static void attatch_vs_to_gf(Vsettings *vset, Text_file *gf)
/* Move PJ global data from the vset and elsewhere into the text
 * file structure */
{
gf->text_name = text_name;
gf->justify_mode = vset->tit_just;
gf->font = uvfont;
gf->is_movable = TRUE;
gf->twin = vset->twin;
gf->tcursor_p = vset->tcursor_p;
gf->text_yoff = vset->text_yoff;
}

static void detatch_vs(Vsettings *vset,  Text_file *gf)
{
vset->twin = gf->twin;
vset->tcursor_p = gf->tcursor_p;
vset->text_yoff = gf->text_yoff;
}


static void led_text(Text_file *gf)
/* call to edit_text_file bracketed by some initialization of the 
   Text_file structure common to all uses of the editor in this file */
{
extern Boolean check_zoom_drag(void *dat);

attatch_vs_to_gf(&vs, gf);
edit_text_file(gf);
detatch_vs(&vs, gf);
save_text_file(gf);
}


void qplace_titles(void)
/* Edit existing text and paste a copy on screen */
{
	qpwtitles(1);
}

static void ttext_undraw_rect(Raster *r, void *data, int x, int y,
	int width, int height)
{
	pj_blitrect(data, x, y, r, 
			 x, y, width, height);
}

void etext_undraw_rect(Raster *r, void *data, int x, int y,
	int width, int height)
{
	pj_set_rect(r, sblack, x, y, width, height);
}

void etext_undraw_dot(int x, int y, Raster *r)
{
pj_put_dot(r, sblack, x, y);
}

void qpwtitles(int paste)
/* Edit existing text and optionally past a copy on pencel.  */
{
Text_file lgtf;
Text_file *gf = &lgtf;

hide_mp();
save_undo();
clear_struct(gf);
gf->text_name = text_name;
if (pj_exists(gf->text_name))
	load_text_file(gf, gf->text_name);
gf->ccolor = vs.ccolor;
gf->raster = (Raster *)vb.pencel;
gf->undraw_rect = ttext_undraw_rect;
gf->undraw_data = (Raster *)undof;
gf->undraw_dot = NULL;
led_text(gf);
zoom_unundo();
if (paste)
	rendr_twin(gf);
free_text_file(gf);
show_mp();
}

void ttool(int paste)
/* Define a box and then let 'em type in it.  When done optionally
   paste text to screen. */
{
Text_file lgtf;
Text_file *gf = &lgtf;

	if (!pti_input())
		return;
	/* define a new rectangle for the text */

	clear_struct(gf);
	gf->raster = (Raster *)vb.pencel;
	if(get_rub_twin(gf,0) < 0)
		return;
	vs.twin =  gf->twin;
	save_undo();

	/* clear out old text */
	free_text_file(gf);
	hide_mp();
	gf->ccolor = vs.ccolor;
	gf->undraw_rect = ttext_undraw_rect;
	gf->undraw_dot = NULL;
	gf->undraw_data = (Raster *)undof;
	led_text(gf);
	zoom_unundo();
	if (paste)
	{
		rendr_twin(gf);
	}
	free_text_file(gf);
	show_mp();
}

Errcode load_and_paste_text(char *name)
/* Load up text from temporary file and paste it in current position */
{
Text_file lgtf;
Errcode err;

clear_struct(&lgtf);
if ((err = load_text_file(&lgtf, lgtf.text_name = name)) < Success)
	return(err);
attatch_vs_to_gf(&vs, &lgtf);	/* Move text into position */
rendr_twin(&lgtf);				/* and paste. */
free_text_file(&lgtf);			/* Free text buffer. */
return(Success);
}

Errcode text_tool(void)
/* A pentool for text.  On pendown make a box, and if this works out
   then let them type in it.  When done paste text to screen */
{
	ttool(1);
	save_redo_text();
	return(Success);
}

static Errcode open_edit_win(Wndo **win)
{
Errcode err;
WndoInit wi;

	clear_mem(&wi,sizeof(wi));
	copy_rectfields(vb.pencel,&wi);
	wi.screen = vb.screen;
	wi.flags = (WNDO_BACKDROP);

	if((err = open_wndo(win,&wi)) < 0)
		goto error;

	(*win)->ioflags = (KEYHIT|ANY_CLICK);
	return(Success);
error:
	return(softerr(err,"edit_window"));
}

void qedit_titles()
/* Edit existing text in same colors we use for menus over a blank screen.
   Don't paste the text. */
{
Text_file lgtf;
#define gf (&lgtf)

clear_struct(gf);
hide_mp();
unzoom();
save_undo();
gf->raster = NULL;
if(open_edit_win((Wndo **)&(gf->raster)) < Success)
	goto OUT;
if (sblack != 0)		/* backdrop window open will have done this case */
	pj_set_rast(gf->raster, sblack);
if (pj_exists(text_name))
	load_text_file(gf, text_name);
gf->ccolor = swhite;
gf->undraw_data = gf->raster;
gf->undraw_rect = etext_undraw_rect;
gf->undraw_dot = etext_undraw_dot;
led_text(gf);
free_text_file(gf);
OUT:
close_wndo((Wndo *)gf->raster);
zoom_unundo();
save_undo();
rezoom();
show_mp();
#undef gf
}


static void rendr_twin(Text_file *gf)
/* Plot text window permanently on rendering screen considering inks and all. */
{
extern void render_mask_blit();

	if (gf->text_buf != NULL)
	{
		set_twin_gradrect();
		if(make_render_cashes() >= 0)
		{
			wwtext(vb.pencel, gf->font, gf->text_buf, 
				gf->twin.x, gf->twin.y, gf->twin.width, gf->twin.height, 
				gf->text_yoff, gf->justify_mode, gf->ccolor, TM_RENDER,
				sblack );
			if (vs.cycle_draw) 
				cycle_redraw_ccolor();

			free_render_cashes();
			dirties();
		}
	}
}

