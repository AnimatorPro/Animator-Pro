/* This file implements the Poco Text and Titling libraries.
 */

#include "errcodes.h"
#include "jimk.h"
#include "pocolib.h"
#include "pocolib.h"
#include "rastext.h"
#include "wordwrap.h"
#include "title.h"


void get_uvfont_name(char *buf);
void qfont_text();
char *pj_get_path_name(char *path);
Errcode do_titles(Boolean	with_menu);	/* aka do text */

static int check_font_width(int width)
/*
 * Make sure width is at least as wide as minimum font width.
 */
{
int widest;
widest = widest_char(uvfont);
if (width < widest)
	width = widest;
return width;
}

static void po_word_wrap(int x, int y, int width, int height, Popot text)
/*****************************************************************************
 * void WordWrap(int x, int y, int width, int height, char *text)
 ****************************************************************************/
{
Rectangle rect;

if (text.pt == NULL)
	{
	builtin_err = Err_null_ref;
	return;
	}
width = check_font_width(width);
rect.x = x;
rect.y = y;
rect.width = width;
rect.height = height;
set_gradrect(&rect);
wwtext(vb.pencel, uvfont, text.pt,
	x, y, width, height, 0, vs.tit_just, vs.ccolor, TM_RENDER, sblack);
if (vs.cycle_draw)
	cycle_redraw_ccolor();
dirties();
}

static int po_word_wrap_count_lines(int width, Popot text)
/*****************************************************************************
 * int WordWrapCountLines(int width, char *text)
 *		count the number of lines text will create when word-wrapping
 *		into a rectangle so wide.
 ****************************************************************************/
{
	SHORT maxwid;

	if (text.pt == NULL)
		return builtin_err = Err_null_ref;
	width = check_font_width(width);
	return wwcount_lines(uvfont, text.pt, width, &maxwid);
}

static void po_ink_string(int x, int y, Popot string)
/*****************************************************************************
 * void Text(int x, int y, char *string)
 ****************************************************************************/
{
if (string.pt == NULL)
	{
	builtin_err = Err_null_ref;
	return;
	}
gftext(vb.pencel, uvfont, string.pt, x, y, vs.ccolor, TM_RENDER,
	vs.inks[0]);
if (vs.cycle_draw)
	cycle_redraw_ccolor();
dirties();
}

static void po_get_font_name(Popot name)
/*****************************************************************************
 * void GetFontName(char *name)
 ****************************************************************************/
{
if (Popot_bufcheck(&name, PATH_SIZE) >= Success)
	get_uvfont_name(name.pt);
}

static Errcode po_load_font(Popot name)
/*****************************************************************************
 * ErrCode LoadFont(char *name)
 ****************************************************************************/
{
	if(name.pt == NULL)
		return(builtin_err = Err_null_ref);
	return(load_the_font(name.pt));
}


static int po_string_width(Popot string)
/*****************************************************************************
 * int StringWidth(char *string)
 ****************************************************************************/
{
if (string.pt == NULL)
	return(Err_null_ref);
return(fstring_width(uvfont, string.pt));
}

static int po_font_height(void)
/*****************************************************************************
 * int FontHeight(void)
 ****************************************************************************/
{
return(font_cel_height(uvfont));
}

static int po_tallest_char(void)
/*****************************************************************************
 * int TallestChar(void)
 ****************************************************************************/
{
return(tallest_char(uvfont));
}

static void po_get_font_dir(Popot dir)
/*****************************************************************************
 * void GetFontDir(char *dir)
 ****************************************************************************/
{
if (Popot_bufcheck(&dir, PATH_SIZE) >= Success)
	{
	vset_get_path(FONT_PATH+vs.font_type,dir.pt);
	*pj_get_path_name(dir.pt) = 0;
	}
}

static int po_get_justify(void)
/*****************************************************************************
 * int GetJustify(void)
 ****************************************************************************/
{
return(vs.tit_just);
}

static void po_set_justify(int just)
/*****************************************************************************
 * void SetJustify(int just)
 ****************************************************************************/
{
if (just < 0 || just > 3)
	builtin_err = Err_parameter_range;
else
	vs.tit_just = just;
}

static Boolean po_can_scale_font()
/*****************************************************************************
 * Boolean CanScaleFont(void);
 *		return TRUE if it's a font that can be scaled
 *		(Type1 or other outline font.)
 ****************************************************************************/
{
	return ((uvfont->flags & VFF_SCALEABLE) ? TRUE : FALSE);
}

static Errcode po_scale_font(int height)
/*****************************************************************************
 *	Set font to a new height.  (Only effective if CanScaleFont() is true.)
 ****************************************************************************/
{
	vs.font_height = height;
	return fset_height(uvfont, height);
}

static void po_set_font_spacing(int spacing)
/*****************************************************************************
 * void SetFontSpacing(int spacing);
 *		Set the "extra" distance between letters.
 ****************************************************************************/
{
	SHORT space, lead;

	fget_spacing(uvfont, &space, &lead);
	space = spacing;
	fset_spacing(uvfont, space, lead);
}

static int po_get_font_spacing(int spacing)
/*****************************************************************************
 * int GetFontSpacing(int spacing);
 *		Get the "extra" distance between letters.
 ****************************************************************************/
{
	SHORT space, lead;

	fget_spacing(uvfont, &space, &lead);
	return space;
}

static void po_set_font_leading(int leading)
/*****************************************************************************
 * void SetFontLeading(int leading);
 *		Set the "extra" distance between lines of text.
 ****************************************************************************/
{
	SHORT space, lead;

	fget_spacing(uvfont, &space, &lead);
	lead = leading;
	fset_spacing(uvfont, space, lead);
}

static int po_get_font_leading(int leading)
/*****************************************************************************
 * int GetFontLeading(int leading);
 *		Get the "extra" distance between lines of text.
 ****************************************************************************/
{
	SHORT space, lead;

	fget_spacing(uvfont, &space, &lead);
	return lead;
}

/***************************Titling Library*******************************/


static void po_title_set_movement(int movement)
/*****************************************************************************
 *"void	TitleSetMovement(int movement);"
 *		Set how the titles move when rendered.
 *		This will be either up the screen, across the screen, typed on,
 *		or still.  See constants in title.h.
 ****************************************************************************/
 {
	if (movement < TM_SCROLL_UP || movement > TM_STILL)
		builtin_err = Err_parameter_range;
	else
		vs.tit_move = movement;
 }

static int po_title_get_movement(void)
/*****************************************************************************
 *"int	TitleGetMovement(void);"
 *		Get how the titles move when rendered.
 *		This will be either up the screen, across the screen, typed on,
 *		or still.  See constants in title.h.
 ****************************************************************************/
 {
 	return vs.tit_move;
 }

static void po_title_set_scrolling(int scrolling)
/*****************************************************************************
 *"void	TitleSetScrolling(int scrolling);"
 *		Set whether scrolling is done by pixel or by character.
 *		See constants in title.h.
 ****************************************************************************/
 {
 	if (scrolling < TS_BY_PIXEL || scrolling > TS_BY_CHARACTER)
		builtin_err = Err_parameter_range;
	else
		vs.tit_scroll = scrolling;
 }


static int po_title_get_scrolling(void)
/*****************************************************************************
 *"int	TitleGetScrolling(void);"
 *		Get whether scrolling is done by pixel or by character.
 *		See constants in title.h.
 ****************************************************************************/
 {
 	return vs.tit_scroll;
 }


static Errcode po_title_set_text(Popot ptext)
/*****************************************************************************
 *"ErrCode	TitleSetText(char *text);"
 *		Set the titling text to the contents of a string.
 *		If text is NULL then get rid of titling text.
 ****************************************************************************/
 {
	char *text;
	int len;
	int bufsize;

	if ((text = ptext.pt) == NULL)			
		return pj_delete(text_name);
	/* Figure out size of text.  Go by strlen mostly.   However it's
	 * possible the (ab)user forgot to /0 terminate the string.  So
	 * check length against the size of the buffer they pass in too. */
	len = strlen(text);							
	bufsize = Popot_bufsize(&ptext);
	if (len > bufsize)
		len = bufsize;
 	return write_gulp(text_name, text, len);
 }

static Errcode po_title_set_text_from_file(Popot name)
/*****************************************************************************
 *"ErrCode	TitleSetTextFromFile(char *file_name);"
 *		Set the titling text to the contents of a file.
 ****************************************************************************/
{
	if (name.pt == NULL)
		return(builtin_err = Err_null_ref);
	return(load_titles(name.pt));
}

static Popot po_title_get_text(void)
/*****************************************************************************
 *"char 	*TitleGetText(void);"
 *		Read the current titling text file into a string. 
 *		The Poco programmer should free() this string sometime.
 *		Returns NULL if there's no current titling text, or if there's
 *		an error.
 ****************************************************************************/
 {
 	long len;
	Popot ret;
	char *buf;

	if ((len = pj_file_size(text_name)) < Success)
		{
		Popot_make_null(&ret);
		}
	else
		{
		ret = poco_lmalloc(len+1);
		if ((buf = ret.pt) != NULL)
			{
			if (len > 0)
				{
				if (read_gulp(text_name, ret.pt, len) < Success)
					poco_freez(&ret);
				}
			}
		}
	return ret;
 }


static Boolean po_title_has_text(void)
/*****************************************************************************
 *"Boolean	TitleHasText(void);"
 *		Returns TRUE if there is some titling text.
 ****************************************************************************/
 {
 	return pj_exists(text_name);
 }


static void po_title_set_position(int x, int y, int w, int h)
/*****************************************************************************
 *"void	TitleSetPosition(int x, int y, int w, int h);"
 *		Set position of  the rectangle in which the titles will be rendered.
 ****************************************************************************/
 {
	vs.twin.x = x;
	vs.twin.y = y;
	vs.twin.width = w;
	vs.twin.height = h;
 }


static void po_title_get_position(Popot px, Popot py,  Popot pw, Popot ph)
/*****************************************************************************
 *"void	TitleGetPosition(int *x, int *y, int *w, int *h);"
 *		Get position of the rectangle in which the titles will be rendered.
 ****************************************************************************/
 {
 	int *x, *y, *w, *h;

	if	((x = px.pt) == NULL || (y = py.pt) == NULL
	||	(w = pw.pt) == NULL || (h = ph.pt) == NULL)
		builtin_err = Err_null_ref;
	else
		{
		*x = vs.twin.x;
		*y = vs.twin.y;
		*w = vs.twin.width;
		*h = vs.twin.height;
		}
 }


static void po_title_edit(void)
/*****************************************************************************
 *"void	TitleEdit(void);"
 *		Invoke the titling menu "place text" function, which lets the
 *		user type in text and move around the text rectangle.
 ****************************************************************************/
 {
	qpwtitles(0);
 }


static Errcode po_title_render(void)
/*****************************************************************************
 *"ErrCode	TitleRender(void);"
 *		Render current titling text over time.  (Uses the FSA mode
 *		to decide whether to go over a single frame, the segment, or
 *		over all.)
 ****************************************************************************/
 {
	Errcode err;

	free_render_cashes();		/* AAARRRR */
 	err = do_titles(FALSE);
	make_render_cashes();
	return err;
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

PolibText po_libtext = {
po_ink_string,
	"void    Text(int x, int y, char *string);",
po_word_wrap,
	"void    WordWrap(int x, int y, int width, int height, char *text);",
po_set_justify,
	"void    SetJustify(int just);",
po_get_justify,
	"int     GetJustify(void);",
po_string_width,
	"int     StringWidth(char *string);",
po_font_height,
	"int     FontHeight(void);",
po_tallest_char,
	"int     TallestChar(void);",
po_get_font_name,
	"void    GetFontName(char *name);",
po_load_font,
	"ErrCode LoadFont(char *name);",
po_get_font_dir,
	"void    GetFontDir(char *dir);",
qfont_text,
	"void    Qfont(void);",
/* From here on new with Ani Pro 1.5 */
po_can_scale_font,
	"Boolean CanScaleFont(void);",
po_scale_font,
	"Errcode ScaleFont(int height);",
po_set_font_spacing,
	"void SetFontSpacing(int spacing);",
po_get_font_spacing,
	"int GetFontSpacing(void);",
po_set_font_leading,
	"void SetFontLeading(int leading);",
po_get_font_leading,
	"int GetFontLeading(void);",
po_word_wrap_count_lines,
	"int    WordWrapCountLines(int width, char *text);",
};

Poco_lib po_text_lib = {
	NULL, "Text",
	(Lib_proto *)&po_libtext,POLIB_TEXT_SIZE,
	};

/***************************Titling Library*******************************/
PolibTitle po_libtitle = {
po_title_set_movement,
	"void	TitleSetMovement(int movement);",
po_title_get_movement,
	"int	TitleGetMovement(void);",
po_title_set_scrolling,
	"void	TitleSetScrolling(int scrolling);",
po_title_get_scrolling,
	"int	TitleGetScrolling(void);",
po_title_set_text,
	"ErrCode	TitleSetText(char *text);",
po_title_set_text_from_file,
	"ErrCode	TitleSetTextFromFile(char *file_name);",
po_title_get_text,
	"char 	*TitleGetText(void);",
po_title_has_text,
	"Boolean	TitleHasText(void);",
po_title_set_position,
	"void	TitleSetPosition(int x, int y, int w, int h);",
po_title_get_position,
	"void	TitleGetPosition(int *x, int *y, int *w, int *h);",
po_title_edit,
	"void	TitleEdit(void);",
po_title_render,
	"ErrCode	TitleRender(void);",
};

Poco_lib po_title_lib = {
	NULL, "Title",
	(Lib_proto *)&po_libtitle,POLIB_TITLE_SIZE,
	};
