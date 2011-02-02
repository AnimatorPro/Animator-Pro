
/* selbrush.c - Menu that has the brush size slider and not much else */

#include "jimk.h"
#include "flicmenu.h"
#include "fli.h"
#include "selbrush.str"

extern int gary_menu_back(), gary_see_title(),
	blacktext();

extern int move_menu(), bottom_menu(),
	see_qslider(), feel_qslider();


extern Flicmenu pbrush_menu;


static struct qslider brush_sl =
	{
	0,
	10,
	&vs.pen_width,
	1,
	};

set_pbrush()
{
hide_mp();
rmove_menu(&pbrush_menu, 
	cur_menu->x - pbrush_menu.x, cur_menu->y-pbrush_menu.y); 
do_menu(&pbrush_menu);
if (vs.pen_width != 0)
	vs.large_pen = vs.pen_width;
draw_mp();
}

static Flicmenu brush_1f2 = 
	{
	NONEXT,
	NOCHILD,
	87, 188,	319-87, 10,
	(char *)&brush_sl,
	see_qslider,
	feel_qslider,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
static Flicmenu movet_sel = {
	&brush_1f2,
	NOCHILD,
	0,188,87,10,
	selbrush_100 /* "Set Brush Size" */,
	blacktext,
	move_menu,
	NOGROUP, 0,
	NOKEY,
	bottom_menu,
	};
static Flicmenu pbrush_menu =
	{
	NONEXT,
	&movet_sel,
	0, 188,	319, 10,
	NULL,
	gary_menu_back,
	NOFEEL,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};



