
/* main.c -  Make sure system initializes ok, and then flop up the menuing
  system. */

#include <stdio.h>
#include "jimk.h"
#include "fli.h"
#include "flicmenu.h"
#include "main.str"

extern selit();
extern Vector pentools[];
extern char zoomcursor;
char goodconf;

static char secret_code[] = "$$$$$$$$$$$$";

static char *noconfig_lines[] =
	{
	main_101 /* "Animator isn't configured." */,
	main_102 /* "Assuming defaults." */,
	NULL,
	};

/* Check for a tempflx file on current scratch device.  If it's there
   use it.  Otherwise check for a default flix file in current directory
   and start up program with that.  If this doesn't exist then try
   to do a empty flic with default settings via empty_tempflx().
   If this doesn't work give up and return 0. */
static
get_tempflx()
{
if (!open_tempflx())
	{
	if (jcopyfile(default_name,tflxname))
		{
		if (!open_tempflx())
			return(empty_tempflx());
		}
	else
		return(empty_tempflx());
	}
return(1);
}

/* Make sure we create a good tempflx.  If can't, abort program. */
static
force_tempflx()
{
char buf[80];

for (;;)
	{
	if (get_tempflx())
		break;
	sprintf(buf,  main_103 /* "Having trouble using drive %c:" */,
		vconfg.scratch_drive + 'A');
	continu_line(buf);
	if (!config_scratch())
		{
		uninit_sys();
		exit(-1);
		}
	}
}


main(argc, argv)
int argc;
char *argv[];
{
if (!init_sys())
	{
	uninit_sys();
	exit(-1);
	}
default_settings();	/* after this safe to use continue alerts */
if (!goodconf)
	{
	continu_box(noconfig_lines);
	rewrite_config();	/* make hard copy of default configuration */
	}
fake_push();
pop_pics();
force_tempflx();
check_loaded_screen();
init_zoom();
check_dfree();
draw_mp();
paint_title_loop();
}


static
paint_title_loop()
{
for (;;)
	{
	wait_input();
	if (key_hit)
		dokeys();
	else if (in_control_space())
		{
		if (in_pblock(0,0,cur_pull) )
			{
			disables();
			if (interp_pull())
				{
				hide_mp();
				selit(menu_ix, sel_ix);
				draw_mp();
				}
			}
		else if (PJSTDN || RJSTDN)
			rsel(cur_menu);
		}
	else if (RJSTDN)
		{
		hide_mp();
		toggle_menu();
		draw_mp();
		}
	else	/* in 'drawing' area */
		{
		if (vs.zoom_mode)
			{
			zoomcursor = 1;
			get_gridxy();	/* 'rezoom' mouse */
			}
		brushcursor = dot_pens[0];
		(*pentools[vs.tool_ix])();
		brushcursor = NULL;
		zoomcursor = 0;
		}
	}
}

