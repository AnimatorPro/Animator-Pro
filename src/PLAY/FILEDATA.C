
/* filedata.c - The data structures for all the buttons on the lo-res
   VGA file selector. */

#include "jimk.h"
#include "flicmenu.h"
#include "prjctor.h"
#include "filedata.str"

extern bwtext(), text_boxp1(), print_list(),
	undo_drawer(), undo_file(), fsel_name(), fq_ok(), new_dev(),
	go_rootdir(), go_updir(), fq_ok_plus(),
	parent_drawer(), feel_string_req(), fq_drawer_stringq(),
	type_file_name(), toggle_group(),
	ccorner_text(), blacktext(),
	ccorner_cursor(), fq_toggle_wild(), fq_wild_stringq(),
	gary_see_title(), dcorner_text(), go_stringq(), fq_file_stringq(), /* ldg */
	see_string_req(), text_box(), gary_menu_back(), close_menu(),
	see_dummy(), ffeelscroll2();

extern wtext(), fincup(), fincdown(), ffeelscroll(), see_scroll();
extern struct cursor ctriup, ctridown;

extern char drawer[];
char file[80];


struct flicmenu fcancel_sel =
	{
	NONEXT,
	NOCHILD,
	316-7*6-5, 181, 7*6, 14,
	filedata_100 /* "Cancel" */,
	dcorner_text,
	close_menu,
	NOGROUP, 0,
	' ',
	NOOPT,
	};
struct flicmenu fok_sel =
	{
	&fcancel_sel,
	NOCHILD,
	230, 181, 30, 14,
	filedata_101 /* "OK" */,
	dcorner_text,
	fq_ok,
	NOGROUP, 0,
	'\r',
	NOOPT,
	};
#ifdef LATER
struct flicmenu fplus_sel =
	{
	&fok_sel,
	NOCHILD,
	192, 181, 18, 14,
	"+",
	ccorner_text,
	fq_ok_plus,
	NOGROUP, 0,
	'\r',
	NOOPT,
	};
#endif LATER
WORD device;
struct flicmenu fdev16_sel =
	{
	&fok_sel,
	NOCHILD,
	105+3*20, 130+3*16, 16, 12,
	"N:",
	ccorner_text,
	new_dev,
	&device, 13,
	'n',
	NOOPT,
	};
struct flicmenu fdev15_sel =
	{
	&fdev16_sel,
	NOCHILD,
	105+2*20, 130+3*16, 16, 12,
	"M:",
	ccorner_text,
	new_dev,
	&device, 12,
	'm',
	NOOPT,
	};
struct flicmenu fdev14_sel =
	{
	&fdev15_sel,
	NOCHILD,
	105+1*20, 130+3*16, 16, 12,
	"L:",
	ccorner_text,
	new_dev,
	&device, 11,
	'l',
	NOOPT,
	};
struct flicmenu fdev13_sel =
	{
	&fdev14_sel,
	NOCHILD,
	105, 130+3*16, 16, 12,
	"K:",
	ccorner_text,
	new_dev,
	&device, 10,
	'k',
	NOOPT,
	};
struct flicmenu fdev12_sel =
	{
	&fdev13_sel,
	NOCHILD,
	105+3*20, 130+2*16, 16, 12,
	"J:",
	ccorner_text,
	new_dev,
	&device, 9,
	'j',
	NOOPT,
	};
struct flicmenu fdev11_sel =
	{
	&fdev12_sel,
	NOCHILD,
	105+2*20, 130+2*16, 16, 12,
	"I:",
	ccorner_text,
	new_dev,
	&device, 8,
	'i',
	NOOPT,
	};
struct flicmenu fdev10_sel =
	{
	&fdev11_sel,
	NOCHILD,
	105+1*20, 130+2*16, 16, 12,
	"H:",
	ccorner_text,
	new_dev,
	&device, 7,
	'h',
	NOOPT,
	};
struct flicmenu fdev9_sel =
	{
	&fdev10_sel,
	NOCHILD,
	105, 130+2*16, 16, 12,
	"G:",
	ccorner_text,
	new_dev,
	&device, 6,
	'g',
	NOOPT,
	};
struct flicmenu fdev8_sel =
	{
	&fdev9_sel,
	NOCHILD,
	105+3*20, 130+1*16, 16, 12,
	"F:",
	ccorner_text,
	new_dev,
	&device, 5,
	'f',
	NOOPT,
	};
struct flicmenu fdev7_sel =
	{
	&fdev8_sel,
	NOCHILD,
	105+2*20, 130+1*16, 16, 12,
	"E:",
	ccorner_text,
	new_dev,
	&device, 4,
	'e',
	NOOPT,
	};
struct flicmenu fdev6_sel =
	{
	&fdev7_sel,
	NOCHILD,
	105+1*20, 130+1*16, 16, 12,
	"D:",
	ccorner_text,
	new_dev,
	&device, 3,
	'd',
	NOOPT,
	};
struct flicmenu fdev5_sel =
	{
	&fdev6_sel,
	NOCHILD,
	105, 130+1*16, 16, 12,
	"C:",
	ccorner_text,
	new_dev,
	&device, 2,
	'c',
	NOOPT,
	};
struct flicmenu fdev4_sel =
	{
	&fdev5_sel,
	NOCHILD,
	105+3*20, 130, 16, 12,
	"B:",
	ccorner_text,
	new_dev,
	&device, 1,
	'b',
	NOOPT,
	};
struct flicmenu fdev3_sel =
	{
	&fdev4_sel,
	NOCHILD,
	105+2*20, 130, 16, 12,
	"A:",
	ccorner_text,
	new_dev,
	&device, 0,
	'a',
	NOOPT,
	};
struct flicmenu fdev2_sel =
	{
	&fdev3_sel,
	NOCHILD,
	105+1*20, 130, 16, 12,
	"..",
	ccorner_text,
	go_updir,
	NOGROUP, 0,
	'.', 
	NOOPT,
	};
struct flicmenu rootdir_sel =
	{
	&fdev2_sel,
	NOCHILD,
	105, 130, 16, 12,
	"\\",
	ccorner_text,
	go_rootdir,
	NOGROUP, 0,
	'\\',
	NOOPT,
	};
char und_wild[12] = "*.*";
char wild[12];
struct stringq wild_stringq =
	{
	2, 2, 11, 12-1, 0, wild, und_wild, 0, 0,
	};
struct flicmenu fwild_sel =
	{
	&rootdir_sel,
	NOCHILD,
	193+58-8, 128+2*16, 11*6+4, 12,
	(char *)&wild_stringq,
	see_string_req,
	fq_wild_stringq,
	NOGROUP, 0,
	CTRL_W,
	NOOPT,
	};
struct flicmenu fwildp_sel =
	{
	&fwild_sel,
	NOCHILD,
	193-8, 128+2*16, 9*6+4, 12,
	filedata_119 /* "Wildcard:" */,
	blacktext,
	fq_toggle_wild,
	NOGROUP, 0,
	NOKEY,  /* '*', */
	NOOPT,
	};
char und_drawer[70] = "c:";
struct stringq drawer_stringq =
	{
	2, 2, 16, 70-1, 0, drawer, und_drawer, 0, 0,
	};
struct flicmenu fdrawer_sel =
	{
	&fwildp_sel,
	NOCHILD,
	193+28-8, 128+16, 16*6+4, 12,
	(char *)&drawer_stringq,
	see_string_req,
	fq_drawer_stringq,
	NOGROUP, 0,
	CTRL_D,
	NOOPT,
	};

struct flicmenu fdrawerp_sel =
	{
	&fdrawer_sel,
	NOCHILD,
	193-8, 128+16, 4*6+4, 12,
	filedata_121 /* "Dir:" */,
	blacktext,
	undo_drawer,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
char und_file[81] = "zippy";
struct stringq file_stringq =
	{
	2, 2, 15, 81-1, 7, file, und_file, 7, 0,
	};
struct flicmenu ffile_sel =
	{
	&fdrawerp_sel,
	NOCHILD,
	193+34-8, 128, 15*6+4, 12,
	(char *)&file_stringq,
	see_string_req, /* fq_file_stringq,*/   
	type_file_name,
	NOGROUP, 0,
	CTRL_F,
	NOOPT,
	};
struct flicmenu ffilep_sel =
	{
	&ffile_sel,
	NOCHILD,
	193-8, 128, 5*6+4, 12,
	filedata_123 /* "File:" */,
	blacktext,
	undo_file,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
struct name_scroller fscroller;
struct flicmenu flist_sel =
	{
	&ffilep_sel,
	NOCHILD,
	20, 115, 74, 11*7+5,
	(char *)&fscroller,
	print_list,
	fsel_name,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};

struct flicmenu fdown_sel =
	{
	&flist_sel,
	NOCHILD,
	3, 185, 14, 12,
	(char *)&ctridown,
	ccorner_cursor,
	fincdown,
	(WORD *)&fscroller, 0,
	DARROW,
	NOOPT,
	};

struct flicmenu fscroll_sel2 =
	{
	&fdown_sel,
	NOCHILD,
	0, 0, 0, 0,
	NOTEXT,
	see_dummy,
	ffeelscroll2,
	NOGROUP, 0,
	PAGEDN,
	NOOPT,
	};
struct flicmenu fscroll_sel =
	{
	&fscroll_sel2,
	NOCHILD,
	3, 129, 14, 54,
	(char *)&fscroller,
	see_scroll,
	ffeelscroll,
	NOGROUP, 0,
	PAGEUP,
	NOOPT,
	};

struct flicmenu fup_sel =
	{
	&fscroll_sel,
	NOCHILD,
	3, 115, 14, 12,
	(char *)&ctriup,
	ccorner_cursor,
	fincup,
	(WORD *)&fscroller, 0,
	UARROW,
	NOOPT,
	};

struct flicmenu ftitle_sel =
	{
	&fup_sel,
	NOCHILD,
	0, 103, 319, 9,
	NULL, /* gets set to prompt by get_filename(prompt) */
	gary_see_title,
	NOFEEL,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};

struct flicmenu fileq_menu = 
	{
	NONEXT,
	&ftitle_sel,
	0, 103,	319, 96,
	NOTEXT,
	gary_menu_back,
	go_stringq,
	NOGROUP, 0,
	'\t',
	NOOPT,
	};

#define NUM_FILE_STRINGQ 3
int cur_fileq;
Flicmenu *fileq_list[NUM_FILE_STRINGQ]={&ffile_sel, &fdrawer_sel, &fwild_sel};

inc_fileq_list()
{
cur_fileq = ((cur_fileq+1 == NUM_FILE_STRINGQ) ? 0 : cur_fileq+1);
}


go_stringq(m)
Flicmenu *m;
{
( ((fileq_list[cur_fileq]))->feelme )(fileq_list[cur_fileq]);
}


see_dummy(m)
Flicmenu *m;
{
}


ffeelscroll2(m) /* too bad this had hard coded stuff */
Flicmenu *m;
{
fflscr(&fscroll_sel,1);
}

