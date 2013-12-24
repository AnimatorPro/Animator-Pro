/* qpocoed.c - Edit some poco source or do other full screen text editing
 * with the Poco font. */

#include "errcodes.h"
#include "jimk.h"
#include "textedit.h"
#include "menus.h"
#include "pocoface.h"

Vfont *get_poco_font();

void etext_undraw_rect(Raster *r, void *data, int x, int y,
	int width, int height);
void etext_undraw_dot(int x, int y, Raster *r);

static void poco_lookup_function(Text_file *gf)
{
char lookup_path[PATH_SIZE];

make_resource_name("lookup.po", lookup_path);
run_poco_stripped_environment(lookup_path);
}

static Errcode open_text_screen_win(Wndo **win)
{
Errcode err;
WndoInit wi;

	clear_mem(&wi,sizeof(wi));
	wi.width = vb.screen->wndo.width;
	wi.height = vb.screen->wndo.height;
	wi.screen = vb.screen;
	wi.over = NULL;
	wi.flags = (WNDO_BACKDROP);

	if((err = open_wndo(win,&wi)) < 0)
		goto error;

	(*win)->ioflags = (KEYHIT|ANY_CLICK);
	return(Success);
error:
	return(softerr(err,"poco_edit"));
}



static long seek_char(char *file, long linepos, short charpos)
/* given a line and character on the line position, convert it to
   absolute character position */
{
long pos = 0;
long lp = 1;
short cp;
char c;

for (;;)
	{
	if ((c = *file++) == 0)
		break;
	if (c == '\n')
		{
		lp++;
		cp=0;
		}
	else
		cp++;
	if (lp >= linepos)
		{
		if (lp > linepos || cp > charpos)
			break;
		}
	pos++;
	}
return(pos);
}


void full_screen_edit(Text_file *gf)
/* Edit existing text full screen in same colors we use for menus over a 
 * blank screen. Don't paste the text. */
{
	unzoom();
	save_undo();
	gf->raster = NULL;
	if(open_text_screen_win((Wndo **)&(gf->raster)) >= Success)
	{
		if (sblack != 0)	/* backdrop window open will have done this case */
			pj_set_rast(gf->raster, sblack);
		gf->ccolor = swhite;
		gf->undraw_data = gf->raster;
		gf->undraw_rect = etext_undraw_rect;
		gf->undraw_dot = etext_undraw_dot;
		copy_rectfields(gf->raster, &gf->twin);
		gf->justify_mode = JUST_LEFT;
		if (vb.screen->is_hires)
			gf->font = vb.screen->mufont;
		else
			gf->font = get_poco_font();
		edit_text_file(gf);
	}
	close_wndo((Wndo *)gf->raster);
	zoom_unundo();
	rezoom();
}

Boolean qedit_poco(long line, int cpos)
/*
 * Edit poco program full screen.  Return TRUE if user makes any changes.
 */
{
#define gf (&lgtf)
	Text_file lgtf;

	clear_struct(gf);
	gf->tcursor_p = vs.ped_cursor_p;
	gf->text_yoff = vs.ped_yoff;
	gf->text_name = poco_source_name;
	if (pj_exists(poco_source_name))
		{
		if (load_text_file(gf, poco_source_name) >= Success)
			{
			if (line >= 0)
				gf->tcursor_p = seek_char(gf->text_buf, line, cpos);
			}
		}
	gf->help_function = poco_lookup_function;
	full_screen_edit(gf);
	vs.ped_cursor_p = gf->tcursor_p;
	vs.ped_yoff = gf->text_yoff;
	save_text_file(gf);
	free_text_file(gf);
	return gf->is_changed;
#undef gf
}

