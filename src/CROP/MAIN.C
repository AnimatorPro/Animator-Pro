
/* main.c - entry point to crop.  Make sure we got all the hardware we
   need, then go wait for user to push a button or select something off
   the pull-downs. */

#include <stdio.h>
#include "jimk.h"
#include "flicmenu.h"
#include "main.str"

extern int ivmode;
extern char init_drawer[];
extern char *get_filename(char *prompt, char *suffix);

main()
{
char *name;
char buf[100];


if (init_sys())	/* Go switch to right resolution and all */
	{
	draw_mp();
	cropper_loop();
	}
quit();
}

cropper_loop()
{
for (;;)
	{
	wait_input();
	if (key_hit)
		dokeys();
	else if (in_pblock(0,0,cur_pull))
		{
		pull_disables();
		if (interp_pull())
			{
			hide_mp();
			selit(menu_ix, sel_ix);
			draw_mp();
			}
		}
	}
}

/* set old video mode */
old_video()
{
union regs r;
r.b.ah = 0;
r.b.al = ivmode;
sysint(0x10,&r,&r);
}

cleanup()
{
change_dir(init_drawer);
old_video();
}


quit()
{
cleanup();
exit(0);
}

qquit()
{
if (yes_no_line(main_100 /* "   Quit Converter?   " */))
	quit();
}

char *about_lines[] =
	{
	main_101 /* "    Autodesk Animator" */,
	main_102 /* "  File Format Converter" */,
	main_103 /* "         v. 1.02" */,
	main_104 /* "Copyright 1989,1990,91 Jim Kent" */,
	main_105 /* "         10/03/91" */,
	main_106 /* "  Produced exclusively for" */,
	main_107 /* "       Autodesk Inc." */,
	main_108 /* "            by" */,
	main_109 /* "       Yost Group Inc." */,
	NULL,
	};

about()
{
continu_box(about_lines);
}

qstatus()
{
char *bufs[8];
char b1[40], b2[40], b3[40];
extern unsigned mem_free;
extern unsigned largest_frag();

bufs[0] = main_110 /* "Converter - memory usage" */;
sprintf(b1, main_111 /* "%ld bytes free" */, mem_free*16L);
bufs[1] = "";
bufs[2] = b1;
sprintf(b2, main_113 /* "%ld largest" */, largest_frag()*16L);
bufs[3] = b2;
bufs[4] = NULL;
continu_box(bufs);
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
if (cur_menu != NULL)
	if (menu_keys(cur_menu) )
		return;

hide_mp();
if (c >= 'A' && c <= 'Z')
	c += 'a' - 'A';
switch (c)
	{
	case 0:
		switch (key_in)
			{
			case UARROW:
				start_in();
				break;
			case LARROW:
				break;
			case RARROW:
				next_in();
				break;
			case DARROW:
				view_in();
				break;
			}
		break;
	case 'q':
		qquit();
		break;
	}
draw_mp();
}

char *anim_lines[] = 
	{
	main_114 /* "LOAD RIF FILE" */,
	main_115 /* "LOAD ANIM FILE" */,
	main_116 /* "CANCEL" */,
	};

qload_amimovie()
{
int choice;

if ((choice = qchoice(main_117 /* "Load an Amiga moving picture?" */,
	anim_lines, Array_els(anim_lines))) != 0)
	{
	switch (choice)
		{
		case 1:
			qload_rif();
			break;
		case 2:
			qload_anim();
			break;
		}
	}
}



selit(m,s)
int m,s;
{
switch (m)
	{
	case 0:
		switch (s)
			{
			case 0:
				about();
				break;
			case 1:
				qstatus();
				break;
			case 2:
				qscale();
				break;
			case 3:
				qmove();
				break;
			case 4:
				qslide();
				break;
			case 5:
				break;
			case 6:
				qquit();
				break;
			}
		break;
	case 1:
		switch (s)
			{
			case 0:
				qload_amimovie();
				break;
			case 1:
				qload_seq();
				break;
			case 2:
				qload_fli();
				break;
			case 3:
				view_in();
				break;
			case 4:
				qsave_fli();
				break;
			}
		break;
	case 2:
		switch (s)
			{
			case 0:
				qload_vision();
				break;
			case 1:
				qload_amiga();
				break;
			case 2:
				qload_st();
				break;
			case 3:
				qload_mac();
				break;
			case 4:
				qload_pc();
				break;
			case 5:	/* load gif */
				qload_gif();
				break;
			case 6:	/* dots */
				break;
			case 7:
				qsave_gif();
				break;
			}
		break;
	}
}


