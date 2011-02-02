#ifndef SCROLLER_H
#define SCROLLER_H

#ifndef MENUS_H
	#include "menus.h"
#endif

typedef struct name_scroller {

/* items loaded before init_scroller() */

	struct names *names;  /* the list of entrys */
	Button *scroll_sel;  /* scroll bar button */ 
	Button *list_sel;    /* cel display button */
	Vfont *font;       /* font for draw_1_cel() use and initializing 
						* if NULL will be screen->font by init_scroller() */

	/* function to draw one cel of a scroller get pointer to list_sel and
	 * pointer to raster (clip box) to draw the cel on and the x and y of
	 * the cel's top left corner and the entry for this cel. Entry may be
	 * NULL and cel is to be blank. MUST be supplied.
	 * rast is a Clipbox of the list_sel button */

	void (*draw_1_cel)(Button *list_sel,void *rast,int x,int y,Names *entry);

	/* highlite a cel when clicking on a cel this is called twice
	 * before the feel_1_cel() with hilite as TRUE the a wait pen up
	  * and immediately before the feelme with hilite == FALSE 
	 * rast is a Clipbox of the list_sel button */

	void (*high_1_cel)(Button *list_sel,void *rast,
					   int x,int y,Names *entry, Boolean highlite);

	/* This feelme will be called by feel_scroll_cels() when a right click
	 * occurrs over the cel. The is_dhit argument will indicate a second
	 * consecutive hit over the same cel. This function can close the menu 
	 * or redraw the scroller rast is a Clipbox of the list_sel button */

	void (*feel_1_cel)(Button *list_sel,void *rast,int x,int y,Names *entry,
					   int why); 
	SHORT top_name;     /* current top entry for scroller */
	SHORT cels_per_row; /* number of cels in a row if 0 will be 1 */
	SHORT cels_per_col; /* number of cels in a column if 0 will be 1 */

	UBYTE border;	   /* size of border wanted around list_sel */ 
	UBYTE col_xoset;   /* x offset to left column of cels in list_sel */
	UBYTE row_yoset;  /* y offset to top row of cels in list_sel */
	UBYTE no_key_mode; /* set this to true if you wish to disable the 
						* keyboard entry mode which calls the feel_1_sel
						* and uses cursor keys to move around */

/* items loaded and maintained by init_scroller() */

	SHORT dcount;		/* # of items can see at once set by init */
	SHORT name_count;	/* number of items total set by init */
	SHORT cel_height;  /* calculated from font or list_sel size */
	SHORT cel_width;   /* calculated from list_sel size */

	SHORT yoff;			/* y offset for use by scroll bar button */
	SHORT knob_height;  /* used by scroll bar */
	UBYTE xoset;	  /* set by init to border + x_oset */
	UBYTE yoset;	  /* set by init to border + x_oset */
	SHORT cliptest;  
	SHORT endtop;
	SHORT key_name;   /* used for keyboard selector */
	char reserved[2];
} Name_scroller;

/* why defines for feel_1_cel note they are flags, they also will be mutually
 * exclusive */

#define SCR_MHIT	0 	 /* mouse pen hit */
#define SCR_ARROW   0x01 /* by arrow key hit during key mode */

#define SCR_MDHIT	0x02 /* mouse pen double hit */
#define SCR_ENTER	0x04 /* enter key hit during key mode */

extern void scroll_pick_opt();

void init_scroller(Name_scroller *scr,Wscreen *s);
void draw_scroll_cels(Button *b);
void scroll_incdown(Button *b);
void scroll_incup(Button *b);
void see_scrollbar(Button *b);
void feel_scrollbar(Button *b, Boolean realtime);
void rt_feel_scrollbar(Button *b);  /* realtime == TRUE */
void slow_feel_scrollbar(Button *b); /* realtime == FALSE */

/* This function will call the feel_1_cel() and hiliters */
void feel_scroll_cels(Button *b);

/* these do not call the feelme but do call the highliter */
void *which_sel(Button *b);
void *why_which_sel(Button *b, Boolean *pwhy);

/* will get root window relative x and y for a scroller cel given index 
 * of cel relative to top name */

void get_scroll_cel_pos(Name_scroller *s, int ix, Short_xy *ppos);

/* stuff specific to a simple name list scroller */

/* init_name_scroller() will set the draw_1_cel() and high_1_cel()
 * the border to 1 and offsets by font and will calculate the number in a 
 * column */

void init_name_scroller(Name_scroller *scr,Wscreen *s);
void see_scroll_names(Button *b); /* seeme for list_sel button */

#endif /* SCROLLER_H */
