
/* Filemenu.c - code to interpret the buttons in filedata.c.  Stuff to
   implement the normal file requestor, and some helper routines for
   the visual browser in browse.c */

#include "jimk.h"
#include "flicmenu.h"
#include "dosstuff.h"
#include "commonst.h"
#include "filemenu.str"
#include <ctype.h>

extern Flicmenu fileq_menu, ftitle_sel, flist_sel, fscroll_sel,
	fwild_sel, menu_save, ffile_sel, fdrawer_sel, fwild_sel,
	fdev2_sel, fdev3_sel;

extern Name_scroller fscroller;

extern Stringq drawer_stringq, file_stringq, wild_stringq;

extern char und_drawer[], wild[];


Vector redisplay_drawer;	/* helps share code with browse.c */
static char *default_suffix;
static char 	path_buf[180];
static char fileq_result;
struct name_list *wild_lst;
extern WORD device;


static
new_drawer()
{
if (!change_dir(vs.drawer))
	return(0);
init_fscroller();
init_stq_string(&drawer_stringq);
draw_sel(&fdrawer_sel);
draw_menus(&fdev2_sel);
redraw_fscroller();
return(1);
}

new_dev(m)
Flicmenu *m;
{
if (m->text)
	{
	change_dev(m->identity);
	unhi_group(cur_menu, m->group);
	make_current_drawer();
	hi_group(cur_menu, m->group);
	(*redisplay_drawer)();
	}
}



go_rootdir(m)
Flicmenu *m;
{
hilight(m);
if (vs.drawer[1] == ':')
	strcpy(vs.drawer+2, cst_fsep);
else
	strcpy(vs.drawer, cst_fsep);
(*redisplay_drawer)();
draw_sel(m);
}

/* move up one directory */
go_updir(m)
Flicmenu *m;
{
int len;
char *d,c;

hilight(m);
d = vs.drawer;
len = strlen(d);
/* move 'd' pointer past device if any */
if (len >= 2)
	{
	if (d[1] == ':')
		{
		d += 2;
		len -= 2;
		}
	}
if (len > 0)
	{
	if (d[0] == '\\')
		{
		d++;
		len--;
		}
	}
while (--len >= 0)
	{
	c = d[len];
	d[len] = 0;
	if (c == '\\')
		break;
	}
draw_sel(m);
(*redisplay_drawer)();
}

type_file_name(m)
Flicmenu *m;
{
feel_string_req(m);
if (key_hit && (key_in&0xff) == '\r')
	fq_ok();
}


static
init_fscroller()
{
build_wild_list();
iscroller(&fscroller,wild_lst,&fscroll_sel,&flist_sel,
	scroll_ycount(&flist_sel),redraw_fscroller);
}



#ifdef SLUFFED
static struct name_scroller *scroll;
#endif SLUFFED

static
redraw_fscroller()
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

dir_too_long(char *drawer, char *name)
{
#define MAX_DIR_LEN 66
if (strlen(drawer)+strlen(name) > MAX_DIR_LEN)
	{
	continu_line(filemenu_100); /* "Directory name too long." */
	return(TRUE);
	}
return(FALSE);
}

fsel_name(m)
Flicmenu *m;
{
char *name;
static long last_time;
static char *last_name;
long time;

time = get80hz();
if ((name = sel_name(m))!=NULL)
	{
	if (name[0] == '\\')	/* a directory */
		{
		if (vs.drawer[strlen(vs.drawer)-1] == '\\')
			name++;
		if (!dir_too_long(vs.drawer, name))
			{
			strcpy(und_drawer, vs.drawer);
			strcat(vs.drawer, name);
			init_stq_string(&drawer_stringq);
			new_drawer();
			}
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
if (vs.drawer[0] != 0)
	{
	len = strlen(vs.drawer);
	if (vs.drawer[len-1] == '\\')	/* perhaps extra slash at end? */
		{
		if (len == 1)	/* just say root, ok */
			;
		else if (len == 3 && vs.drawer[1] == ':') /* root of device .. ok */
			;
		else 
			vs.drawer[len-1] = 0;
		}
	}
if (!(*redisplay_drawer)())
	{
	undo_stringq(m, &fdrawer_sel);
	(*redisplay_drawer)();
	}
}


fq_toggle_wild(m)
Flicmenu *m;
{
if (suffix_in(wild, default_suffix))
	strcpy(wild, cst_wild_all);
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



fq_ok()
{
fileq_result = 1;
close_menu();
}


static
add_wild(fn, prefix)
struct fndata *fn;
char *prefix;
{
struct name_list *next;
char buf[16];
int dir;
int c2;

if (fn->name[0] == '.')	/* filter out '.' and '..' */
	{
	c2 = fn->name[1];
	if (c2 == '.' || c2 == 0)
		return(1);
	}
if ((next = askmem(sizeof(*next))) == NULL)
	return(0);
sprintf(buf, "%s%s", prefix, fn->name);
if ((next->name = clone_string(buf)) == NULL)
	{
	freemem(next);
	return(0);
	}
next->next = wild_lst;
wild_lst = next;
}



free_wild_list()
{
free_name_list(wild_lst);
wild_lst = NULL;
}

build_wild_list()
{
extern Name_list *sort_name_list();

/* nuke the old wild list... */
free_wild_list();
attr_wild_list(16, cst_wild_all, cst_fsep);	/* get all directories */
attr_wild_list(0, wild, cst_);		/* and other files matching wild */
wild_lst = sort_name_list(wild_lst);
}

static
attr_wild_list(attr, pat, prefix)
int attr;
char *pat;
char *prefix;
{
union regs reg;
int err;
struct fndata *fn;

/* get the 'DTA' area for directory search */
reg.b.ah = 0x2f;
sysint(0x21,&reg,&reg);
fn = make_ptr(reg.w.bx, reg.w.es);

/* now do the find first... */
reg.b.ah = 0x4e;	/* int 21 function # */
reg.w.cx = attr;	/* 'attribute' */
reg.w.dx = ptr_offset(pat);
reg.w.ds = ptr_seg(pat);
if (!(sysint(0x21,&reg,&reg)&1))	/* check 'carry' flag for error... */
	{
	if ((fn->attribute&16) == attr)
		add_wild(fn,prefix);
	for (;;)
		{
		reg.b.ah = 0x4f;
		if (sysint(0x21,&reg,&reg) & 1)
			break;
		if ((fn->attribute&16) == attr)
			add_wild(fn,prefix);
		}
	}
}



make_path_name(drawer, file, path)
char *drawer, *file, *path;
{
int len; 
char c;

if (file[1] == ':')	/* say hey it's got the drive in file string */
	{
	if (!valid_device(file[0] - 'A') )
		return(0);
	strcpy(path, file);
	return(1);
	}
strcpy(path, drawer);
/* if no : or \ at end of drawer better add it */
if ((len = strlen(drawer)) != 0)
	{
	c = drawer[len-1];
	if (c != ':' && c != '\\')
		strcat(path, cst_fsep);
	}
/* add in the drawer */
strcat(path, file);
return(1);
}

static char *
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

got_suffix(f)
char *f;
{
return(fix_suffix(f,0) != NULL );
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


static
inc_file()
{
char *pt;
char suffix[6];
char numbuf[10];
int len, nlen;
char c;
long number;

if ((pt = fix_suffix(vs.file,0))!=NULL)
	{
	strcpy(suffix,pt);
	}
else
	{
	suffix[0] = 0;
	}
remove_suffix(vs.file);
len = strlen(vs.file);
pt = vs.file + len;
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
	pt = vs.file+nlen;
*pt = 0;
strcat(vs.file, numbuf);
strcat(vs.file, suffix);
vs.fscroller_top = fscroller.top_name;
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

static char file_only[16];


char *
get_filename(prompt, suffix)
char *prompt, *suffix;
{
char *name;

unzoom();
hook_devices(&fdev3_sel, 14);
redisplay_drawer = new_drawer;
make_current_drawer();		/* get current directory and device... */
ftitle_sel.text = prompt;	/* display prompt in move area... */
default_suffix = suffix;	/* stash initial suffix... */
sprintf(wild, "*%s", suffix);	/* and make up initial wild search*/
strcat(vs.file,  suffix);	/* add suffix to old filename */

init_stq_string(&drawer_stringq);	/* initialize 3 text input structures */
init_stq_string(&file_stringq);
init_stq_string(&wild_stringq);

fscroller.top_name = vs.fscroller_top;
init_fscroller();
fileq_result = 0;
name = NULL;
if (cur_menu != NULL)
	{
	clip_rmove_menu(&fileq_menu, 0,
		(uzy - (fileq_menu.height>>1)) - fileq_menu.y); 
	}
do_pmenu(&fileq_menu, &ffile_sel);
if (fileq_result)
	{
	if (make_path_name(vs.drawer, vs.file, path_buf))
		name = path_buf;
	}
strncpy(file_only, vs.file, sizeof(file_only)-1);	/* keep copy with suffix */
if (got_suffix(vs.file))
	remove_suffix(vs.file);
else
	strcat(path_buf, suffix);	/* auto-extend */
vs.fscroller_top = fscroller.top_name;
free_wild_list();
rezoom();
return(name);
}

