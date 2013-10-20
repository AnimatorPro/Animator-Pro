/* #include <conio.h>  for kbdhit */
#include "jimk.h"
#include "flicmenu.h"
#include "fli.h"
#include "prjctor.h"
#include "mainmenu.str"

extern long clock1;
extern char sl_overfl, sl_underfl;
extern char gif_loaded;
extern char is_gif;

extern char notice_keys;
extern int cur_frame_num;
extern long get80hz();

extern dcorner_text(), ccorner_text(), ncorner_text(), gary_menu_back(),
	ccolor_box(), ppalette(), see_pen(), toggle_pen(), set_pbrush(),
	ccorner_cursor(), blacktext(), greytext(), toggle_group(),
	insert_a_frame(), kill_a_frame(), set_total_frames(),
	see_time_slider(), feel_time_slider(), see_range_width(),
	see_qslider(), feel_qslider(), ncorner_number(),
	mplayit(), mfirst_frame(), mlast_frame(), minsert(), mdelete_frame(),
	jump_to_frame(), jump_to_mark(), set_mark(),
	frame_double(), go_multi(),
	use_range_button(), set_range_button(),
	move_tab_text(), move_menu(), bottom_menu(),
	multi_preview(), multi_use(), close_menu(),
	hang_child(), toggle_group(), change_mode(),  bcursor();


#define TR1 155
#define TR2 186

extern Video_form alt_vf;

extern struct cursor cdown, cleft, cright, csleft,
	cinsert, cappend, cright2, cleft2, csright, cup, size_cursor,
	default_c, ckill;

extern struct fli_head fh;
extern char global_file_name[];
extern char file_is_loaded;
extern int loaded_file_fd;

int frame_val=0;  /* ldg */
int speed_val;
int global_frame_count;  /* ldg */

struct qslider frame_sl = {0, 0, &frame_val, 1, NULL};
struct qslider speed_sl = { 0, 120, &fh.speed, 0, };

struct flicmenu tmu_spdsl_sel = {
	NONEXT,
	NOCHILD,
	201,TR2,115,10,
	&speed_sl,
	see_qslider,
	feel_qslider, 	/* feelme */
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
struct flicmenu tmu_spdtag_sel = {
	&tmu_spdsl_sel,
	NOCHILD,
	166,TR2,35,10,
	mainmenu_100 /* "SPEED" */,
	blacktext,
 	NOFEEL, 	/* feelme */
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};

struct flicmenu tmu_frame_sl_sel = {
	&tmu_spdtag_sel,
	NOCHILD,
	11,186,115,10,
	&frame_sl,
	see_time_slider,
	feel_time_slider, 	/* feelme */
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
struct flicmenu tmu_play_sel = {
	&tmu_frame_sl_sel,
	NOCHILD,
	129,187,10,8,
	&cright2,
	bcursor,
	mplayit, 	/* feelme */
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
struct flicmenu tmu_down_sel = {
	&tmu_play_sel,
	NOCHILD,
	140,187,8,8,
	&cdown,
	bcursor,
	mlast_frame, 	/* feelme */
	NOGROUP, 0,
	DARROW,
	NOOPT,
	};
struct flicmenu tmu_up_sel = {
	&tmu_down_sel,
	NOCHILD,
	2,187,8,8,
	&cup,
	bcursor,
	mfirst_frame, 	/* feelme */
	NOGROUP, 0,
	UARROW,
	NOOPT,
	};

struct flicmenu main_menu = 
	{
	NONEXT,  	/* next */
	&tmu_up_sel, 	/* children */
	0, 183, 319, 16,
	NOTEXT,
	gary_menu_back,  /* seeme */
	NOFEEL,		/* feelme */
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};



mfirst_frame ()
{
int old_speed;

if (gif_loaded) return;
if (!file_is_loaded) return;
old_speed=fh.speed;
close_file();
hide_mp();
load_frame1(global_file_name,&vf,1,0);
set_frame_val(1);
fh.speed=old_speed;
draw_mp();
}

mlast_frame ()
{
if (gif_loaded) return;
if (!file_is_loaded) return;
hide_mp();
goto_frame(frame_val, fh.frame_count-1);
draw_mp();
}



mplayit ()
{
if (gif_loaded) return;
if (!file_is_loaded) return;

hide_mp();
notice_keys=1;
clock1 = get80hz();
hide_mouse();
is_gif=0;
while (1)
	{
	advance_frame(&vf,1);
	clock1 += fh.speed;
	if (!wait_til2(clock1))
		break;
	if (clock1 > get80hz())
		clock1 = get80hz();
	}

/* clear input */
draw_mp();
show_mouse();
}



jump_to_frame ()
{
}


feel_time_slider(m)
Flicmenu *m;
{
int new_val;
int old_val;

if (gif_loaded) return;

sl_overfl = sl_underfl = 0;

old_val=frame_val;
feel_qslider(m);
if (sl_overfl)
	new_val=0;
else if (sl_underfl)
	new_val=fh.frame_count-1;
else 
	new_val=frame_val;
hide_mp();
goto_frame(old_val,new_val);
draw_mp();
}



see_time_slider(m)
Flicmenu *m;
{
if (!gif_loaded) frame_sl.max =  global_frame_count;
see_qslider(m);
}


set_frame_val(val)
{
frame_val= (val > 0) ? val-1: 0;
cur_frame_num=val;
}
