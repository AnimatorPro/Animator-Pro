/* analog to vpaint.c  in v */

#include <stdio.h>
#include "jimk.h"
#include "flicmenu.h"
#include "fli.h"
#include "prjctor.h"
#include "prj1.str"

char gif_loaded;
char file_is_loaded;
int loaded_file_fd;
char global_file_name[100];

extern char path_buf[];
extern char stop_bat;
extern struct flicmenu tmu_frame_sl_sel, tmu_spdsl_sel;
extern struct qslider speed_sl;
extern int global_frame_count; 
extern int frame_val, speed_val;
extern char exit_word;
extern int orig_speed;

extern struct fli_head fh;
extern long get80hz();

extern struct flicmenu fileq_menu;

p_load_fli()
{
char *title;

if ((title = get_filename(prj1_100 /* "Load an animation file?" */, ".FLI"))!=NULL)
	{
	reset();
	if (load_frame1(title,&vf,1,0)!=0)
		{
		speed_sl.max=SL_MAX_SPEED;
		file_is_loaded=1;
		strcpy(global_file_name,title);
		set_frame_val(1);
		global_frame_count=fh.frame_count-1;
		orig_speed=fh.speed;
		}
	}
}


p_load_gif()
{
char *title;

if ((title =  get_filename(prj1_102 /* "Load a Picture?" */, ".GIF"))!=NULL)
	{
/*        zero_palette(); */
	reset();
	if (load_gif(title,&vf)!=0)
		{
		gif_loaded=1;
		strcpy(global_file_name,title);
		}
	else gif_loaded=0;
	}
}


zero_palette()
{
UBYTE  *d;
int i;

d=vf.cmap;
for (i=0; i < COLORS * 3; i++) *d++ = 63;  /* set screen to white */
wait_sync();
jset_colors(0, COLORS, vf.cmap);
}



p_load_bat()
{
char *title;

if ((title = get_filename(prj1_104 /* "Load and play a script file?" */, ".*"))!=NULL)
	{
	reset();
	strcpy(global_file_name,title);
	exit_word=0;
	stop_bat=0;
	while (process_bat(global_file_name)); /* this while takes care of linked files */
	if (exit_word) quit();
	reset();
	my_reset_cmap();
	}
}




selit(menu, select)
int menu, select;
{
int buf[80];
WORD *behind;
char *title;

sprintf(buf, prj1_106 /* "Selected menu %d sel %d" */, menu, select);
switch (menu)
	{
	case 0:
		switch (select)
			{
			case 0:
				about(); 
				break;
			case 1:	/* --- */
				break;
			case 2:	/* quit */
				if (yes_no_line(prj1_108 /* "Exit player program?" */))
					quit();
				break;
			}
		break;
	case 1:  /* files */
		switch (select)
			{
			case 0:  /* load fli */
 				p_load_fli(); 
				break;
			case 1: /* load gif */
				p_load_gif(); 
				break;
			case 2: /* load bat */
				p_load_bat();
				break;
			}
		break;
	default:
/*		continu_line(buf);  */
		break;
	}
}



beep()
{
sound(70);
delay(300);
nosound();
}



dokeys()
{
unsigned char c = key_in;
int ok;

ok = pull_keys(&root_pull);
if (ok < 0)	/* good 1st pull-equiv key, but then blew it */
	return;
else if (ok > 0)	/* got valid pull-equiv key selection */
	{
	hide_mp(); 
	selit(menu_ix, sel_ix);
	draw_mp();
	return;
	}
/* else drop through to let someomne else process keystroke */
if (cur_menu != NULL && c !=' ' && key_in!=F11 && key_in!=F12)  /* real kludge for F11 here --ugh! */
	if (menu_keys(cur_menu) )
		return;

hide_mp();
if (c >= 'A' && c <= 'Z')
	c += 'a' - 'A';
switch (c)
	{
	case 'q':
		if (yes_no_line(prj1_108 /* "Exit player program?" */))
			quit();
		break;
	case '\r':
		mplayit();
		break;
	case '?':
		status(); 
		break;
	case ' ':
		wait_click();  /* let user view the scene */
		break;
	default:
		switch (key_in)
			{
			case PLUS_KEY:
			case NUM_PLUS_KEY:
				fh.speed =( (fh.speed+1 < speed_sl.max) ? 
						fh.speed+1: speed_sl.max);
				break;
			case MINUS_KEY:
			case NUM_MINUS_KEY:
				fh.speed =( (fh.speed-1 > speed_sl.min) ? 
						fh.speed-1: speed_sl.min);
				break;
			case LARROW:
				prev_frame(); 
				break;
			case RARROW:
				if (file_is_loaded) advance_frame(&vf,1); 
				break;
			case DARROW:
				mlast_frame();
				break;
			case UARROW:
				mfirst_frame();
				break;
			case F1:
			case F2:
			case F3:
			case F4:
			case F5:
			case F6:
			case F7:
			case F8:
			case F9:
			case F10:
				if (!file_is_loaded) break;
				do_speed_change(key_in);
				break;			
			case F11:
				if (!file_is_loaded) break;
				fh.speed = ((fh.speed<=0) ? 0:fh.speed-1);;
				break;
			case F12:
				if (!file_is_loaded) break;
				fh.speed = ((fh.speed>=120)?120:fh.speed+1);
				break;			
			default:
				{
				}
			}
		break;
	}
draw_mp();
}






about()
{
char *bufs[9];

bufs[0] = prj1_109 /* "     Autodesk Animator" */,
bufs[1] = prj1_110 /* "   Public Domain Player" */,
bufs[2] = prj1_111 /* "          V. 1.02" */,
bufs[3] = "  ",
bufs[4] = prj1_113 /* " Produced Exclusively for " */,
bufs[5] = prj1_114 /* "      Autodesk, Inc." */;
bufs[6] = prj1_115 /* "            by" */,
bufs[7] = prj1_116 /* "     Yost Group, Inc." */,
bufs[8] = NULL;
continu_box(bufs);
}

status()
{
}


clear_screen()
/* put color 0 everywhere */
{
cblock(VGA_SCREEN, 0, 0, 320, 200, 0);
}



reset()
{
if (file_is_loaded) close_file();
/* path_buf[0]=0; */
gif_loaded=0; 
clear_screen();
set_frame_val(1);
fh.speed=0;
speed_sl.max=0;
global_frame_count=0;
global_file_name[0]='\0';
/* draw_mp(); */
}


extern unsigned char sys_space_colors[];
extern unsigned char sys_start_colors[];

my_reset_cmap()
{
copy_bytes(sys_space_colors, vf.cmap+3*(249-1), 8*3); 
copy_bytes(sys_start_colors, vf.cmap, 15*3); 
wait_sync();
jset_colors(248, 8, sys_space_colors); 
jset_colors(0, 15, sys_start_colors);
}
