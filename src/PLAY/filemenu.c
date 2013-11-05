
/* filemenu.c - The code part of the VGA lo-res file selector. */
/* Basically a function for each type of button unique to the file */
/* selector, and the entry point - get_filename() */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jimk.h"
#include "flicmenu.h"
#include "fs.h"
#include "ptr.h"

static void redraw_fscroller(void);

extern wait_click();

extern Flicmenu fileq_menu, ftitle_sel, flist_sel, fscroll_sel,
	fwild_sel, menu_save, ffile_sel, fdrawer_sel, fwild_sel,
	fdev1_sel, fdev3_sel;

extern Name_scroller fscroller;

extern Stringq drawer_stringq, file_stringq, wild_stringq;

extern char und_drawer[], wild[], drawer[], file[];


WORD fscroller_top;
Vector redisplay_drawer;	/* helps share code with browse.c */
static char *default_suffix;
char 	path_buf[81];
char fileq_result;
char search_all[] = "*.*";
extern WORD device;



new_drawer()
{
if (!change_dir(drawer))
	return(0);
init_fscroller();
init_stq_string(&drawer_stringq);
draw_sel(&fdrawer_sel);
redraw_fscroller();
return(1);
}

new_dev(m)
Flicmenu *m;
{
hilight(m);
if (m->text)
	{
	change_mode(m);
	if (change_dev(device))
		{
		make_current_drawer();
		(*redisplay_drawer)();
		}
	}
draw_sel(m);
}



go_rootdir(m)
Flicmenu *m;
{
hilight(m);
fs_go_rootdir();
(*redisplay_drawer)();
draw_sel(m);
}

/* move up one directory */
go_updir(m)
Flicmenu *m;
{
hilight(m);
fs_go_updir();
draw_sel(m);
(*redisplay_drawer)();
}

type_file_name(m)
Flicmenu *m;
{
feel_string_req(m);
/* */
if (key_hit && (key_in&0xff) == '\r')
	fq_ok();
/**/
}


init_fscroller()
{
build_wild_list();
iscroller(&fscroller,wild_lst,&fscroll_sel,&flist_sel,
	scroll_ycount(&flist_sel),redraw_fscroller);
}

static void
redraw_fscroller(void)
{
redraw_scroller(&fscroll_sel, &flist_sel);
}


undo_file(m)
Flicmenu *m;
{
undo_stringq(m, &ffile_sel);
}

undo_drawer(m)
Flicmenu *m;
{
undo_stringq(m, &fdrawer_sel);
if (!(*redisplay_drawer)())
	undo_stringq(m, &fdrawer_sel);
}


fsel_name(m)
Flicmenu *m;
{
File_list *n;
static long last_time;
static char *last_name;
long time;

time = get80hz();
if ((n = which_sel(m)) != NULL)
	{
	char *name = n->name;
	if (n->type == FILETYPE_DIRECTORY)
		{
		if (drawer[strlen(drawer)-1] == DIR_SEPARATOR_CHAR)
			name++;
		strcpy(und_drawer, drawer);
		strcat(drawer, name);
		init_stq_string(&drawer_stringq);
		new_drawer();
		}
	else
		{
		if (time - last_time < 30)
			{
			if (name == last_name)
				{
				fileq_result = 1;
				close_menu();
				}
			}
		last_name = name;
		last_time = time;
		upd_stringq(name, &file_stringq);
		draw_sel(&ffile_sel);
		}
	}
}


fq_drawer_stringq(m)
Flicmenu *m;
{
char c;
int len;

feel_string_req(m);
if (drawer[0] != 0)
	{
	len = strlen(drawer);
	if (drawer[len-1] == DIR_SEPARATOR_CHAR) /* perhaps extra slash at end? */
		{
		if (len == 1)	/* just say root, ok */
			;
		else if (len == 3 && drawer[1] == ':') /* root of device .. ok */
			;
		else 
			drawer[len-1] = 0;
		}
	}
if (!(*redisplay_drawer)())
	undo_stringq(m, &fdrawer_sel);
}


fq_toggle_wild(m)
Flicmenu *m;
{
if (suffix_in(wild, default_suffix))
	strcpy(wild, search_all);
else
	sprintf(wild, "*%s", default_suffix);
init_stq_string(&wild_stringq);
init_fscroller();
redraw_fscroller();
draw_sel(m);
draw_sel(&fwild_sel);
}

fq_wild_stringq(m)
Flicmenu *m;
{
feel_string_req(m);
init_fscroller();
redraw_fscroller();
draw_sel(m);
}


#ifdef IF_NEEDED
/* LEWIS */
fq_file_stringq(m)
Flicmenu *m;
{
feel_string_req(m);
draw_sel(m);
}
#endif

fq_ok()
{
fileq_result = 1;
close_menu();
}

build_wild_list()
{
extern Name_list *sort_name_list(Name_list *list);

/* nuke the old wild list... */
free_name_list(wild_lst);
wild_lst = NULL;
fs_build_wild_list(wild);
wild_lst = (File_list *) sort_name_list((Name_list *) wild_lst);
}

char *
fix_suffix(f,remove)
char *f;
WORD remove;
{
int len;
char *pt;
int noperiod = 0;
int got_period;

len = strlen(f);
pt = f+len;
noperiod = 0;
got_period = 0;
while (--len >= 0)
	{
	if (*(--pt) == '.')
		{
		got_period = 1;
		break;
		}
	noperiod++;
	}
if (got_period && noperiod <= 3)
	{
	if (remove)
		*pt = 0;
	return(pt);
	}
else
	return(NULL);
}


remove_suffix(s)
char *s;
{
fix_suffix(s,1);
}

fq_ok_plus()
{
inc_file();
fq_ok();
}


inc_file()
{
char *pt;
char suffix[6];
char numbuf[10];
int len, nlen;
char c;
long number;

if ((pt = fix_suffix(file,0))!=NULL)
	{
	strcpy(suffix,pt);
	}
else
	{
	suffix[0] = 0;
	}
remove_suffix(file);
len = strlen(file);
pt = file + len;
while (len > 0)
	{
	c = *(pt-1);
	if (!isdigit(c) )
		break;
	--pt;
	--len;
	}
number = atol(pt)+1;
sprintf(numbuf, "%2ld",number);
tr_string(numbuf, ' ', '0');
nlen = 8-strlen(numbuf);
if (len > nlen)
	pt = file+nlen;
*pt = 0;
strcat(file, numbuf);
strcat(file, suffix);
fscroller_top = fscroller.top_name;
}

hook_devices(m, buttons)
Flicmenu *m;
int buttons;
{
extern char devices[];
extern int dev_count;
int i;

for (i=0; i<buttons; i++)
	{
	if (i < dev_count)
		{
		m->identity = devices[i];
		m->key_equiv = devices[i]+'a';
		((char *)(m->text))[0] = devices[i] + 'A';
		}
	else
		{
		m->key_equiv = 0;
		m->seeme = m->feelme = m->optme = NULL;
		}
	m = m->next;
	}
}


char *
get_filename(prompt, suffix)
char *prompt, *suffix;
{
char *name;

hook_devices(&fdev3_sel, 14);
redisplay_drawer = new_drawer;
make_current_drawer();		/* get current directory and device... */
ftitle_sel.text = prompt;	/* display prompt in move area... */
default_suffix = suffix;	/* stash initial suffix... */
sprintf(wild, "*%s", suffix);	/* and make up initial wild search*/

init_stq_string(&drawer_stringq);	/* initialize 3 text input structures */
init_stq_string(&file_stringq);
init_stq_string(&wild_stringq);

fscroller.top_name = fscroller_top;
init_fscroller();
fileq_result = 0;
name = NULL;
interp_menu(&fileq_menu, wait_click, &ffile_sel);
if (fileq_result)
	{
	make_path_name(drawer, file ,suffix, path_buf); /* result in path_buf */
	name = path_buf;
	}
fscroller_top = fscroller.top_name;
free_name_list(wild_lst);
wild_lst = NULL;
return(name);
}

