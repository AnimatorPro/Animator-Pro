
/* vpaint.c - handle high level user button pushes.  Main pull-down
   interpreter switch and main keyboard interpreter switch.  Implementations
   of many of routines called by above.  The first layer under main(). 
   If you #define NOSAVE this will disable saving. */

#include "jimk.h"
#include "cblock_.h"
#include "commonst.h"
#include "fli.h"
#include "flicmenu.h"
#include "peekpok_.h"
#include "vpaint.str"


extern long get80hz();

extern Flicmenu fileq_menu,time_menu;
extern Flicmenu quick_menu,title_menu,edit_menu;
extern int sdot(), xdot(), a1bdot(), rbdot();
extern int xysdot(), xycopydot();
extern int render_hline();
extern Vector pentools[];
extern Option_list *options_list;
extern auto_shrink();


get_color()
{
check_input();
vs.ccolor = getd(render_form->p,grid_x,grid_y);
vs.cycle_draw = 0;
}






static
qreset_seq()
{
extern char init_drawer[];

if (yes_no_line(vpaint_100 /* "Reset to default.flx?" */) )
	{
	jdelete(optics_name);	/* get rid of optics moves */
	jdelete(ppoly_name);	/* and optics path */
	jdelete(poly1_name);	/* and tweening polys... */
	jdelete(poly2_name);
	copy_structure(&default_vs, &vs, sizeof(vs) );
	strcpy(vs.drawer, init_drawer);
	copy_cmap(init_cmap, render_form->cmap);
	rethink_settings();
	if (zoom_form)
		unset_zoom();
	close_tflx();
	if ((jcopyfile(vpaint_101 /* "DEFAULT.FLX" */, 
		tflxname) && open_tempflx()))
		{
		fli_abs_tseek(render_form,0);
		fhead.session = fhead.strokes = 0;	/* erase strokes in default.flx */
		vs.frame_ix = 0;
		see_cmap();
		}
	else
		kill_seq();
	zoom_it();
	}
}


static
qkill_seq()
{
if (yes_no_line(vpaint_102 /* "New Flic - abandon current flic?" */))
	{
	kill_seq();
	zoom_it();
	}
}

qload_defaults()
{
char *title;

if ((title = get_filename(vpaint_103 /* "Load settings file?" */, 
	".SET")) != NULL)
	{
	load_defaults(title);
	}
}

qsave_defaults()
{
char *title;

if ((title = get_filename(vpaint_105 /* "Save settings file?" */, 
	".SET")) != NULL)
	{
	if (overwrite_old(title))
		save_defaults(title);
	}
}


static
load_defaults(name)
char *name;
{
Vsettings new;

if (!read_gulp(name, &new, (long)sizeof(new)))
	{
	return(0);
	}
if (new.type != SETTINGS_MAGIC)
	{
	continu_line(vpaint_107 /* "Not a good settings file" */);
	return(0);
	}
/* keep a few elements of current state that might confuse things if
   changed */
new.frame_ix = vs.frame_ix;	/* keep frame ix */
new.bframe_ix = vs.bframe_ix; /* and back frame cashe */
new.type = FLIX_MAGIC;
new.zoom_mode = vs.zoom_mode;
if (cur_menu)		/* and menu position */
	new.rmyoff = cur_menu->y;
exchange_words(&new, &vs, sizeof(vs)/sizeof(WORD));
rethink_settings();
return(1);
}

static
save_defaults(name)
char *name;
{
Vsettings old;

copy_structure(&vs, &old, sizeof(old));
old.type = SETTINGS_MAGIC;
return(write_gulp(name, &old, (long)sizeof(old) ));
}


qload_mask()
{
char *title;

unzoom();
if ((title =  get_filename(vpaint_108 /* "Load Mask?" */, ".MSK"))!=NULL)
	{
	load_mask(title);
	}
rezoom();
}

qsave_mask()
{
char *title;

if (mask_plane == NULL)
	return;
unzoom();
if ((title =  get_filename(vpaint_110 /* "Save Mask?" */, ".MSK"))!=NULL)
	{
	if (overwrite_old(title) )
		save_mask(title);
	}
rezoom();
}


qload_cel()
{
char *title;

unzoom();
if ((title =  get_filename(vpaint_112 /* "Load Cel?" */, ".CEL"))!=NULL)
	{
	if (load_cel(title))
		show_cel_a_sec();
	}
rezoom();
}

qsave_cel()
{
char *title;

if (cel == NULL)
	return;
unzoom();
if ((title =  get_filename(vpaint_114 /* "Save Cel?" */, ".CEL"))!=NULL)
	{
	if (overwrite_old(title) )
		save_pic(title,(Vscreen *)cel,1);
	}
rezoom();
}


qload_pic()
{
char *title;

unzoom();
if ((title =  get_filename(vpaint_116 /* "Load a Picture?" */, 
	".GIF"))!=NULL)
	{
	save_undo();
	if (!load_some_pic(title, render_form))
		unundo();
	see_cmap();
	zoom_it();
	dirties();
	}
rezoom();
}

qsave_pic()
{
char *title;

unzoom();
if ((title = get_filename(vpaint_118 /* "Save a Picture?" */, 
	".GIF"))!=NULL)
	{
	if (overwrite_old(title) )
		{
		if (suffix_in(title, ".PIC"))
			save_pic(title,render_form,1);
		else 
			save_gif(title, render_form);
		}
	}
rezoom();
}

qload()
{
char *title;

unzoom();
if ((title = get_filename(vpaint_123 /* "Load a flic?" */, 
	".FLI"))!=NULL)
	{
	if (check_fli(title))
		{
		load_fli(title);
		vs.bframe_ix = 0; /* back frame buffer no good now */
		}
	}
rezoom();
}


qsave()
{
#ifdef NOSAVE
#else /* NOSAVE */
char *title;

unzoom();
if ((title = get_filename(vpaint_125 /* "Save a flic?" */, 
	".FLI"))!=NULL)
	{
	if (overwrite_old(title) )
		save_fli(title);
	}
rezoom();
#endif /* NOSAVE */
}


static char *cel_options[] = {
	vpaint_127 /* "* Clear Key Color" */,
	vpaint_128 /* "* Auto Fit Colors" */,
	vpaint_129 /* "  Exit Menu" */,
	};

check_cel_options()
{
char **pt, *p;

pt = cel_options;
p = *pt++;
p[0] = (vs.zero_clear ? '*' : ' ');
p = *pt++;
p[0] = (vs.fit_colors ? '*' : ' ');
}

static
qcel_options()
{
int choice;

for (;;)
	{
	check_cel_options();
	choice =  qchoice(vpaint_130 /* "Cel Options" */, 
		cel_options, Array_els(cel_options));
	if (!choice)
		break;
	switch (choice)
		{
		case 1:
			vs.zero_clear = !vs.zero_clear;
			break;
		case 2:
			vs.fit_colors = !vs.fit_colors;
			break;
		}
	}
}


static char *macro_options[] = {
	vpaint_131 /* "start record" */,
	vpaint_132 /* "end record" */,
	vpaint_133 /* "use macro" */,
	vpaint_134 /* "repeat macro" */,
	vpaint_135 /* "realtime record" */,
	vpaint_136 /* "files..." */,
	cst_cancel,
	};

static
qmacro()
{
int choice;

choice = qchoice(vpaint_138 /* "Macro Recording" */, 
	macro_options, Array_els(macro_options));
switch (choice)
	{
	case 0:
		break;
	case 1:
		start_macro();
		break;
	case 2:
		close_macro();
		break;
	case 3:
		use_macro();
		break;
	case 4:
		repeat_macro();
		break;
	case 5:
		realtime_macro();
		break;
	case 6:
		go_files(11);
		break;
	}
}




dokeys()
{
unsigned char c = key_in;
int ok;

disables();
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
	case 'z':
		ktoggle_zoom();
		break;
	case 'b':
		tog_pen();
		break;
	case '@':
		palette();
		break;
	case '\r':	
		flip_range();
		break;
	case '5':
		flip5();
		break;
	case 0x1b:	/* escape?? */
		dupe_cel();
		break;
	case '\t': /* Tab */
		clip_cel();
		break;
	case '`':
		paste_cel();
		break;
	case 'm':
		move_cel();
		break;
#ifdef UNUSED
	case 's':
		qsave();
		break;
#endif UNUSED
	case 'l':
		qload();
		break;
	case 'n':
		qkill_seq();
		break;
	case 'q':
		qquit();
		break;
	case '\b':
		undo_pic();
		break;
	case 'x':
		clear_pic();
		break;
	case 'u':
		defrag();
		break;
	case '?':
		status();
		break;
	case ' ':
		toggle_menu();
		break;
	case 'o':
		go_ado();
		break;
	default:
		switch (key_in)
			{
			case 0x3b00:	/* F1 */
				get_color();
				break;
			case INSERTKEY:
				insert_a_frame();
				break;
			case DELKEY:
				kill_a_frame();
				break;
			case LARROW:
				prev_frame();
				break;
			case RARROW:
				next_frame();
				break;
			case 0x3c00:	/* F2 */
			case DARROW:
				playit();
				break;
			case UARROW:
				first_frame();
				break;
#ifdef LATER
			default:
				{
				char buf[80];

				sprintf(buf, "%x", key_in);
				continu_line(buf);
				}
#endif LATER
			}
		break;
	}
draw_mp();
}

static current_bad(int dev, long rroom)
{
if (rroom < 0)
	rroom = dfree(dev+1);
if (rroom < MIN_DFREE)
	{
	char *rbufs[4];
	char b1[50], b2[50], b3[50];

	sprintf(b1, vpaint_158, /* "Drive %c: has %ld free bytes." */
		dev+'A', rroom);
	sprintf(b2, vpaint_159, /* "%ld recommended for error free." */
		MIN_DFREE);
	sprintf(b3,  vpaint_160, /* "operation.  Use %c: anyway?" */
		dev+'A');
	rbufs[0] = b1;
	rbufs[1] = b2;
	rbufs[2] = b3;
	rbufs[3] = NULL;
	return(!yes_no_box(rbufs));
	}
return(FALSE);
}


#define ATATIME 8
config_scratch()
/* Let user choose temporary drive. Encourage them to have MIN_DFREE on it.*/
{
char *buf, *p;
char *bufs[12];
long droom[12];
long rroom;
/* int i; */
int t;
int d;
int choice;
char newdev;
int c1;
int start;
int ok;

if ((buf = begmem((ATATIME+2)*34)) == NULL)
	return(0);
AGAIN:
t = 0;
ok = 1;
for (;;)
	{
	p = buf;
	c1 = 0;
	start = t;
	for (;;)
		{
		if (t >= dev_count || c1 >= ATATIME)
			break;
		d = devices[t++];
		if (d > 1)	/* don't do floppies! */
			{
			droom[c1] = dfree(d+1);
			bufs[c1] = p;
			sprintf(p, vpaint_139 /* " %c: drive %8ld bytes free" */, 
				d+'A', droom[c1] );
			c1++;
			if (d == vconfg.scratch_drive)
				p[0] = '*';
			p += 34;
			}
		}
	if (t < dev_count)
		{
		bufs[c1++] = vpaint_140 /* "MORE" */;
		}
	bufs[c1++] = cst_cancel;
	bufs[c1] = NULL;
	choice = qchoice(vpaint_142 /* "Choose drive for temp. files:" */, 
		bufs, c1);
	switch (choice)
		{
		case 0:
			if (current_bad(vconfg.scratch_drive, -1L))
				goto AGAIN;
			ok = 0;
			goto OUT;
		case ATATIME+1:
			start += ATATIME;
			break;		/* to loop again */
		default:
			newdev = bufs[choice-1][1];
			if (current_bad(newdev-'A',droom[choice-1]))
				goto AGAIN;
			do_copy_overs(vconfg.scratch_drive+'A', newdev);
			d = vconfg.scratch_drive = newdev - 'A';
			path_temps(newdev);
			rewrite_config();
			goto OUT;
		}
	}
OUT:
freemem(buf);
return(ok);
}

check_dfree()
{
long l;
char buf1[40],buf2[40], *bufs[5];

l = dfree(vconfg.scratch_drive+1);
if (l < MIN_DFREE)
	{
	sprintf(buf1, vpaint_143 /* "Scratch drive %c: only has" */, 
		vconfg.scratch_drive+'A');
	sprintf(buf2, vpaint_144 /* "%ld bytes free." */, l);
	bufs[0] = buf1;
	bufs[1] = buf2;
	bufs[2] = NULL;
	continu_box(bufs);
	config_scratch();
	}
}




static char *pixel_options[] = {
	vpaint_145 /* "Shrink x2" */,
	vpaint_146 /* "Expand x2" */,
	vpaint_147 /* "Crop" */,

	vpaint_148 /* "Trails" */,
	vpaint_149 /* "Pixelate" */,
	vpaint_150 /* "Engrave" */,

	vpaint_151 /* "Lace" */,
	vpaint_152 /* "Grays Only" */,
	vpaint_153 /* "Blue Numbers" */,

	cst_cancel,
	};


static
pixel_menu()
{
int choice;

choice =  qchoice(vpaint_155 /* "Special Effects" */, 
	pixel_options, Array_els(pixel_options));
switch (choice)
	{
	case 0:	/* cancel */
		break;

	case 1:
		auto_shrink();
		break;
	case 2:
		auto_expand();
		break;
	case 3:
		crop_video();
		break;

	case 4:
		auto_trails();
		break;
	case 5:
		quantize();
		break;
	case 6:
		auto_engrave();
		break;

	case 7:
		auto_dither();
		break;
	case 8:
		greys_only();
		break;
	case 9:	/* blue numbers */
		auto_blue_nums();
		break;
	}
}

static
view_frame()
{
hide_mouse();
wait_click();
show_mouse();
}

static 
v12()
{
exchange_form(render_form, alt_form);
see_cmap();
zoom_it();
}



static
view_alt()
{
if (alt_form != NULL)
	{
	v12();
	hide_mouse();
	wait_click();
	show_mouse();
	v12();
	}
}

static
qquit()
{
char *bufs[3];

bufs[0] =     vpaint_156 /* "Exit Autodesk Animator?" */;
if (dirty_file)
	{
	bufs[1] = vpaint_157 /* "(You have unsaved changes.)" */;
	bufs[2] = NULL;
	}
else
	bufs[1] = NULL;
if (yes_no_box(bufs))
	{
	outofhere();
	}
}

selit(menu, select)
int menu, select;
{
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
			case 2: /* browse */
				go_browse();
				break;
			case 3: /* Timing... */
				do_time_menu();
				break;
			case 4: /* Optics */
				go_ado();
				break;
			case 5:  /* Palette */
				palette();
				break;
			case 6:  /* tools */
				qtools();
				break;
			case 7: /* inks */
				qinks();
				break;
			case 8: /* titling */
				do_title_menu();
				break;
			case 9:	/* --- */
				break;
			case 10:	/* quit */
				qquit();
				break;
#ifdef MORPH
			case 11:	/* morph */
				go_morf();
				break;
#endif /* MORPH */
			}
		break;
	case 1:
		switch (select)
			{
			case 0:
				qkill_seq();
				break;
			case 1:
				qreset_seq();
				break;
			case 2:
				qload_overlay();
				break;
			case 3:
				qload_splice();
				break;
			case 4:
				qsave_backwards();
				break;
			case 5:	/* Pixels */
				pixel_menu();
				break;
			case 6: /* files */
				go_files(0);
				break;
			}
		break;
	case 2: /* frame */
		switch (select)
			{
			case 0:	/* clear */
				clear_pic();
				break;
			case 1: /* restore */
				restore();
				break;
			case 2: /* set */
				auto_set();
				break;
			case 3:	/* separate */
				separate();
				break;
			case 4:  /* view */
				view_frame();
				break;
			case 5: /* files */
				go_files(1);
				break;
			}
		break;
	case 3:	/* alt */
		switch (select)
			{
			case 0: /* grab alt screen */
				grab_alt();
				break;
			case 1: /* swap alt screen */
				swap_alt();
				break;
			case 2: /* put alt screen */
				auto_put();
				break;
			case 3: /* view alt*/
				view_alt();
				break;
			case 4:
				free_alt();
				break;
			}
		break;
	case 4:	/* cel */
		switch (select)
			{
			case 0:	/* clip */
				clip_cel();
				break;
			case 1:	/* get */
				dupe_cel();
				break;
			case 2: /* translate */
				move_cel();
				break;
			case 3: /* paste */
				paste_cel();
				break;
			case 4:	/* below */
				upaste_cel();
				break;
			case 5:  /* stretch */
				vstretch_cel();
				break;
			case 6:  /* rotate */
				vrotate_cel();
				break;
			case 7:  /* mask */
				vmask_cel();
				break;
			case 8: /* options cel */
				qcel_options();
				break;
			case 9:  /* free */
				free_cel(cel);
				cel = NULL;
				break;
			case 10:  /* files */
				go_files(2);
				break;
			}
		break;
	case 5:	/* blue */
		switch (select)
			{
			case 0: /* blue frame */
				qblue_pic();
				break;
			case 1:	/* unblue frame */
				qunblue_pic();
				break;
			case 2:	/* next blue */
				qnext_blue();
				break;
			case 3:	/* tween guides */
				insert_tween();
				break;
			case 4:	/* erase guides */
				clean_tween();
				break;
			case 5:	/* get changes */
				qget_changes();
				break;
			case 6: /* next changes */
				qnext_changes();
				break;
			case 7: 
				loop_range();
				break;
			case 8: /* flip range */
				flip_range();
				break;
			case 9:	/* flip5 */
				flip5();
				break;
			default:
				break;
			}
		break;
	case 6:		/* other */
		switch (select)
			{
			case 0:		/* stencil */
				qstencil();
				break;
			case 1:		/* grid */
				qgrid();
				break;
			case 2:		/* macro */
				qmacro();
				break;
			case 3: /* settings */
				go_files(9);
				break;
			case 4: /* configure */
				new_config();
				break;
			case 5:
				status();
				break;
			default:
				break;
			}
		break;
	default:
		break;
	}
}




