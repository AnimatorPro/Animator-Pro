#include <stdio.h>
#include "jimk.h"
#include "flicmenu.h"  /*just for cur menu etc */
#include"prjctor.h"
#include "main.str"

extern Flicmenu main_menu;
extern char *get_filename(char *prompt, char *suffix);
extern char exit_word;
char notice_keys=DEFAULT_KBD_NOTICE;

unsigned char alt_cmap[COLORS*3];
char *alt_screen;
Video_form alt_vf = { 0,0,XMAX,YMAX,Raster_line(XMAX),(void *)(NULL),alt_cmap, };

extern WORD mouse_connected;
char *mouse_mess[] = {
	main_100 /* "NOTE: Mouse driver not found--" */,
	main_101 /* "System will operate without mouse." */,
	NULL,
	};


main(argc, argv)
int argc;
char *argv[];
{
char *name;
char buf[100];
int num_loops;

cur_menu = &main_menu;

if (!init_sys())
	{
	quit();
	exit(-1);
	}

/* allocate extra screen for moving around pictures */
if ( (alt_screen=begmem(SCREEN_SIZE) )==NULL) 
	{
	quit();
	exit(-1);
	}
alt_vf.p = (void *)alt_screen;
/* copy_cmap(vf.cmap,alt_vf.cmap); */

if (do_args(argc,argv)!=0)  /* non-zero retrn means go on with main loop */
	{
	
	if (!mouse_connected) 
		{
		continu_box(mouse_mess);
		/* quit(); */
		}		
	do_main_loop();
	quit();
	}
else quit();
}


quit()
{
cleanup();
exit(0);
}


do_args(argc, argv)
int argc;
char *argv[];
{
char buff[85];

if (argc < 2) 
	{
	return(1);
	}

if (!play_pic(argv[1],0,0,0,0,0,1 ))  /* otherwise assume its a bat file */
	{
	exit_word=1;
	strcpy(buff,argv[1]);
	while (process_bat(buff)); /* this while takes care of linked files */
	if (!exit_word) 
		{
		reset();
		my_reset_cmap();
		notice_keys=1;
		return(1); 
		}
	return(0);
     	}
return(0); /* 1 means to go with with main loop */
}




do_main_loop()
{
reset();
draw_mp();

for (;;)
	{
	wait_input();
	if (key_hit)
		{
		/* hide_mp();*/	/* life is simpler if menu's aren't up when do keys */
		dokeys();
		/* draw_mp();*/
		}
	else if (cur_pull != NULL && in_pblock(0,0,cur_pull) )
		{
		/*disables(); */
		if (interp_pull())
			{
			hide_mp();
			selit(menu_ix, sel_ix);
			draw_mp();
			}
		}
	else if (cur_menu != NULL && in_menu(cur_menu))
		{
		if (PJSTDN || RJSTDN)
			{
			rsel(cur_menu);
			}
		}
	else if (RJSTDN)
		{
		hide_mp();
		wait_click();
		draw_mp();
		}
	}
}

