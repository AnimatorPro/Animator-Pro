#include "errcodes.h"
#include "pjbasics.h"
#include "scroller.h"
#include "softmenu.h"


extern Image ctridown, ctriup;
static void pick_entry(), see_pick_entry(), show_entry_info(), 
	see_info_button(), verify_pick_close();


static Name_scroller sscroller;

/*** Button Data ***/
static Button drv_pick_sel = MB_INIT1(
	NONEXT, /* next */
	NOCHILD, /* children */
	294, 11, 21, 67, /* w,h,x,y */
	NODATA, 		/* datme */
	see_pick_entry,
	NOFEEL,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0 /* flags */
	);
static Button drv_ok_sel = MB_INIT1(
	&drv_pick_sel, /* next */
	NOCHILD, /* children */
	77, 11, 21, 81, /* w,h,x,y */
	NODATA, /* "Ok", */
	dcorner_text,
	verify_pick_close,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0 /* flags */
	);
static Button drv_cancel_sel = MB_INIT1(
	&drv_ok_sel, /* next */
	NOCHILD, /* children */
	77, 11, 223, 81, /* w,h,x,y */
	NODATA, /* "Cancel", */
	dcorner_text,
	mb_gclose_cancel,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0 /* flags */
	);
static Button drv_info_sel = MB_INIT1(
	&drv_cancel_sel, /* next */
	NOCHILD, /* children */
	77, 11, 121, 81, /* w,h,x,y */
	NODATA, /* "Info", */
	see_info_button,
	show_entry_info,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0 /* flags */
	);
static Button drv_swin_sel = MB_INIT1(
	&drv_info_sel, /* next */
	NOCHILD, /* children */
	294, 52, 21, 12, /* w,h,x,y */
	&sscroller,
	see_scroll_names,
	feel_scroll_cels,
	NOOPT,
	&sscroller,0,
	NOKEY,
	0 /* flags */
	);
static Button drv_incdn_sel = MB_INIT1(
	&drv_swin_sel, /* next */
	NOCHILD, /* children */
	14, 10, 5, 54, /* w,h,x,y */
	&ctridown,  /* datme */
	ccorner_image,
	scroll_incdown,
	NOOPT,
	&sscroller,0,
	DARROW,
	0 /* flags */
	);
static Button drv_sbar_sel = MB_INIT1(
	&drv_incdn_sel, /* next */
	NOCHILD, /* children */
	14, 30, 5, 23, /* w,h,x,y */
	&sscroller,
	see_scrollbar,
	rt_feel_scrollbar,
	NOOPT,
	&sscroller,0,
	NOKEY,
	0 /* flags */
	);
static Button drv_incup_sel = MB_INIT1(
	&drv_sbar_sel, /* next */
	NOCHILD, /* children */
	14, 10, 5, 12, /* w,h,x,y */
	&ctriup, 	/* datme */
	ccorner_image,
	scroll_incup,
	NOOPT,
	&sscroller,0,
	UARROW,
	0 /* flags */
	);
static Button drv_title_sel = MB_INIT1(
	&drv_incup_sel, /* next */
	NOCHILD, /* children */
	320, 9, 0, 0, /* w,h,x,y */
	NULL,
	see_titlebar,
	feel_titlebar,
	mb_menu_to_bottom,
	&tbg_moveclose,0,
	NOKEY,
	0 /* flags */
	);

static Menuhdr driver_menu = {
	{320,95,0,0,},
	0,				/* id */
	PANELMENU,		/* type */
	&drv_title_sel,	/* buttons */
	SCREEN_FONT,	/* font */
	SCREEN_CURSOR,	/* cursor */
	seebg_white,	/* seebg */
	NULL,			/* data */
	NULL,			/* domenu */
	(MBPEN|MBRIGHT|KEYHIT),	/* ioflags */
};

static Smu_button_list drv_smblist[] = {
	{ "title", &drv_title_sel },
	{ "ok", &drv_ok_sel },
	{ "cancel", &drv_cancel_sel },
	{ "info", &drv_info_sel },
};


static void *info_pick_dat;
static Boolean (*info_func)(Names *entry,void *dat);
static Errcode (*pick_func)(Names *entry,void *dat);

static void see_info_button(Button *b)
{
	if(info_func == NULL)
		return;
	dcorner_text(b);
}
static void show_entry_info(Button *b)
/* call the function tied to the 'info' button, if any.
 * Note:
 *	 The pocolib caller (Qscroll) counts on the fact that this function may
 *	 be called when there is no item currently picked from the scrolling list.
 *	 (This behavior is documented in the pocolib docs).  If other users of
 *	 go_driver_scroller need to ensure that something is selected when the
 *	 Info button is hit, they must check for a NULL pointer passed as the
 *	 first parm to the info_func.
 */
{
	if(info_func == NULL)
		return;
	if((*info_func)((Names *)drv_pick_sel.datme,info_pick_dat))
		mb_gclose_ok(b);
}
static void see_pick_entry(Button *b)
{
Names *entry;

	if((entry = b->datme) != NULL)
		b->datme = entry->name;
	else
		b->datme = "";
	wbg_ncorner_ltext(b);
	b->datme = entry;
}
static void feel_1_scroll(Button *list_sel,void *rast,int x,int y,
						  Names *entry, int why)
{
	drv_pick_sel.datme = entry;
	draw_buttontop(&drv_pick_sel);
	if(why & (SCR_MDHIT|SCR_ENTER))
		verify_pick_close(list_sel);
}
Errcode go_driver_scroller(char *title,
						   Names *list,
						   Names *initial_entry,
						   Errcode (*pick_entry)(Names *entry,void *dat),
						   Boolean (*show_info)(Names *entry,void *dat),
						   void *dat,
						   char **button_texts	)

/* will put up a big wide scroller for driver selection etc. will call pick
 * entry when ok button or double hit is pressed, will install an "info" button
 * if show_info is non NULL, and will call it when the "info" button is
 * hit. initial_entry is the entry "selected" at menu startup, if NULL it is
 * left blank.	the button_texts parm is an optional array of 3 pointers to
 * char strings; if NULL, defaults (Ok, Info, Cancel) will be used.
 * if the info_func returns FALSE, a hit on the info button will NOT close
 * the menu and return, if it returns TRUE, the dialog will be closed and
 * control will be returned to the caller of go_driver_scroller.  (in
 * practice, only the pocolib Qscroll caller uses the button_texts and a
 * TRUE return from info_func right now.)
 */
{
Errcode err;
void *ss;

	hide_mp();

	if((err = soft_buttons("driver_panel", drv_smblist, Array_els(drv_smblist),
					 &ss)) < Success)
	{
		goto error;
	}
	/* set static info function and data */

	info_func = show_info;
	pick_func = pick_entry;
	info_pick_dat = dat;

	/* outside title to tilebar button */

	drv_title_sel.datme = title;

	/* handle custom/default button texts */

	if (button_texts != NULL)
	{
		drv_ok_sel.datme	 = button_texts[0];
		drv_info_sel.datme	 = button_texts[1];
		drv_cancel_sel.datme = button_texts[2];
	}


	/* initialize scroller data */

	clear_struct(&sscroller);
	sscroller.names = list;
	sscroller.scroll_sel = &drv_sbar_sel;
	sscroller.list_sel = &drv_swin_sel;
	sscroller.font = vb.screen->mufont;
	sscroller.cels_per_row = 1;
	sscroller.feel_1_cel = feel_1_scroll;
	init_name_scroller(&sscroller,vb.screen);

	drv_pick_sel.datme = initial_entry;

	err = do_reqloop(vb.screen,&driver_menu,NULL,NULL,NULL);
error:
	smu_free_scatters(&ss);
	show_mp();
	return(err);
}
static void verify_pick_close(Button *b)
/* call the function tied to the Ok button.
 * Note:
 *	unlike the Info button function, the Ok button requires that some item
 *	be selected before calling the associated function.  Again this is
 *	documented behavior.
 */
{
	if (pick_func != NULL)
	{
		/* do not exit menu unless something is selected */
		if(!drv_pick_sel.datme) 
		{
			soft_continu_box("!%s", "pick_entry", drv_cancel_sel.datme );
			return;
		}
		hide_mp();
		if(((*pick_func)((Names *)drv_pick_sel.datme,info_pick_dat)) >= Success)
			mb_gclose_ok(b);
		else
			show_mp();
	}
	else
		mb_gclose_ok(b);
}
Errcode cant_query_driver(Errcode err, char *name)
{
	return(softerr(err, "!%s", "driver_query", name));
}
