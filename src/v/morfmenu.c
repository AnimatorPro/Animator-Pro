/* morfmenu.c - contains the code for the morf screen except the
   pull-down data structures in morfpull.c and the button data
   structures in morfdata.c */

#ifdef MORPH
#include "jimk.h"
#include "flicmenu.h"
#include "morf.h"

extern struct pull morf_pull;
extern Flicmenu quick_menu;
extern Flicmenu dsel_hanger_sel;
extern Flicmenu morf_tool_group_sel;
extern int morf_tool;


qload_faces()
{
char *title;

if ((title = get_filename("load faces", 
	".FAC")) != NULL)
	{
	load_faces(title);
	draw_moos();
	}
}

qsave_faces()
{
char *title;

if ((title = get_filename("save faces", 
	".FAC")) != NULL)
	{
	save_faces(title);
	}
}


static
morf_selit(menu, sel)
int menu, sel;
{
char buf[80];

hide_mp();
switch (menu)
	{
	case 0:
		switch (sel)
			{
			case 0:
				wait_click();
				break;
			case 1:
				draw_moos();
				break;
			case 2:
				see_morf_points();
				break;
			default:
				goto NOBUTTON;
			}
		break;
	case 1:
		switch (sel)
			{
			case 0:
				morf_new_face();
				break;
			default:
				goto NOBUTTON;
			}
		break;
	case 3:
		switch (sel)
			{
			case 0:
				qload_faces();
				break;
			case 1:
				qsave_faces();
				break;
			case 2:
				clear_all_moos();
				clear_all_faces();
				draw_moos();
				break;
			default:
				goto NOBUTTON;
			}
		break;
	default:
		goto NOBUTTON;
	}
draw_mp();
return;
NOBUTTON:
sprintf(buf, "menu %d sel %d\n", menu, sel);
continu_line(buf);
draw_mp();
}

morf_func()
{
hide_mp();
switch (morf_tool)
	{
	case 0:
		morf_new_face();
		break;
	case 3:
		morf_make_face();
		break;
	case 4:
		morf_link_face();
		break;
	default:
		{
		char buf[80];
		sprintf(buf, "morf tool %d", morf_tool);
		continu_line(buf);
		}
		break;
	}
draw_mp();
}


go_morf()
{
Flicmenu *omenu;
Pull *opull;
Flicmenu *odselg;

unzoom();
if (push_screen())
	{
	draw_moos();
	opull = root_pull.children;
	omenu = cur_menu;
	root_pull.children = &morf_pull;
	odselg = dsel_hanger_sel.children;
	dsel_hanger_sel.children = &morf_tool_group_sel;
	cur_menu = &quick_menu;
	draw_mp();
	sub_menu_loop(morf_selit,morf_func);
	hide_mp();
	dsel_hanger_sel.children = odselg;
	cur_menu = omenu;
	root_pull.children = opull;
	pop_screen();
	}
rezoom();
}

#endif /* MORPH */
