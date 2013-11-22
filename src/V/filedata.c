
/* filedata.c - data structures for the file requestor.  (The normal one
   not the visual browse.) The code part of this is mostly in filemenu.c */

#include "jimk.h"
#include "flicmenu.h"
#include "commonst.h"
#include "filedata.str"

extern bwtext(), text_boxp1(), print_list(),
	undo_drawer(), undo_file(), fsel_name(), fq_ok(), new_dev(),
	go_rootdir(), go_updir(), fq_ok_plus(),
	parent_drawer(), feel_string_req(), fq_drawer_stringq(),
	type_file_name(), toggle_group(), move_menu(), bottom_menu(),
	ccorner_text(), blacktext(),
	ccorner_cursor(), fq_toggle_wild(), fq_wild_stringq(),
	gary_see_title(), dcorner_text(),
	see_string_req(), text_box(), gary_menu_back(), close_menu();

extern wtext(), fincup(), fincdown(), ffeelscroll(), see_scroll();
extern struct cursor ctriup, ctridown;


static Flicmenu fcancel_sel =
	{
	NONEXT,
	NOCHILD,
	316-7*6-5, 181, 7*6, 14,
	cst_cancel,
	dcorner_text,
	close_menu,
	NOGROUP, 0,
	' ',
	NOOPT,
	};
static Flicmenu fok_sel =
	{
	&fcancel_sel,
	NOCHILD,
	230, 181, 30, 14,
	cst_ok,
	dcorner_text,
	fq_ok,
	NOGROUP, 0,
	'\r',
	NOOPT,
	};
static Flicmenu fplus_sel =
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
WORD device;
#define F8K 1
static Flicmenu fdev16_sel =
	{
	&fplus_sel,
	NOCHILD,
	105+3*20+F8K, 130+3*16, 16, 12,
	"N:",
	ccorner_text,
	new_dev,
	&device, 13,
	'n',
	NOOPT,
	};
static Flicmenu fdev15_sel =
	{
	&fdev16_sel,
	NOCHILD,
	105+2*20+F8K, 130+3*16, 16, 12,
	"M:",
	ccorner_text,
	new_dev,
	&device, 12,
	'm',
	NOOPT,
	};
static Flicmenu fdev14_sel =
	{
	&fdev15_sel,
	NOCHILD,
	105+1*20+F8K, 130+3*16, 16, 12,
	"L:",
	ccorner_text,
	new_dev,
	&device, 11,
	'l',
	NOOPT,
	};
static Flicmenu fdev13_sel =
	{
	&fdev14_sel,
	NOCHILD,
	105+F8K, 130+3*16, 16, 12,
	"K:",
	ccorner_text,
	new_dev,
	&device, 10,
	'k',
	NOOPT,
	};
static Flicmenu fdev12_sel =
	{
	&fdev13_sel,
	NOCHILD,
	105+3*20+F8K, 130+2*16, 16, 12,
	"J:",
	ccorner_text,
	new_dev,
	&device, 9,
	'j',
	NOOPT,
	};
static Flicmenu fdev11_sel =
	{
	&fdev12_sel,
	NOCHILD,
	105+2*20+F8K, 130+2*16, 16, 12,
	"I:",
	ccorner_text,
	new_dev,
	&device, 8,
	'i',
	NOOPT,
	};
static Flicmenu fdev10_sel =
	{
	&fdev11_sel,
	NOCHILD,
	105+1*20+F8K, 130+2*16, 16, 12,
	"H:",
	ccorner_text,
	new_dev,
	&device, 7,
	'h',
	NOOPT,
	};
static Flicmenu fdev9_sel =
	{
	&fdev10_sel,
	NOCHILD,
	105+F8K, 130+2*16, 16, 12,
	"G:",
	ccorner_text,
	new_dev,
	&device, 6,
	'g',
	NOOPT,
	};
static Flicmenu fdev8_sel =
	{
	&fdev9_sel,
	NOCHILD,
	105+3*20+F8K, 130+1*16, 16, 12,
	"F:",
	ccorner_text,
	new_dev,
	&device, 5,
	'f',
	NOOPT,
	};
static Flicmenu fdev7_sel =
	{
	&fdev8_sel,
	NOCHILD,
	105+2*20+F8K, 130+1*16, 16, 12,
	"E:",
	ccorner_text,
	new_dev,
	&device, 4,
	'e',
	NOOPT,
	};
static Flicmenu fdev6_sel =
	{
	&fdev7_sel,
	NOCHILD,
	105+1*20+F8K, 130+1*16, 16, 12,
	"D:",
	ccorner_text,
	new_dev,
	&device, 3,
	'd',
	NOOPT,
	};
static Flicmenu fdev5_sel =
	{
	&fdev6_sel,
	NOCHILD,
	105+F8K, 130+1*16, 16, 12,
	"C:",
	ccorner_text,
	new_dev,
	&device, 2,
	'c',
	NOOPT,
	};
static Flicmenu fdev4_sel =
	{
	&fdev5_sel,
	NOCHILD,
	105+3*20+F8K, 130, 16, 12,
	"B:",
	ccorner_text,
	new_dev,
	&device, 1,
	'b',
	NOOPT,
	};
Flicmenu fdev3_sel =
	{
	&fdev4_sel,
	NOCHILD,
	105+2*20+F8K, 130, 16, 12,
	"A:",
	ccorner_text,
	new_dev,
	&device, 0,
	'a',
	NOOPT,
	};
Flicmenu fdev2_sel =
	{
	&fdev3_sel,
	NOCHILD,
	105+1*20+F8K, 130, 16, 12,
	"..",
	ccorner_text,
	go_updir,
	NOGROUP, 0,
	'.',
	NOOPT,
	};
static Flicmenu rootdir_sel =
	{
	&fdev2_sel,
	NOCHILD,
	105+F8K, 130, 16, 12,
	cst_fsep,
	ccorner_text,
	go_rootdir,
	NOGROUP, 0,
	'\\',
	NOOPT,
	};
char wild[12];
struct stringq wild_stringq =
	{
	2, 2, 11, 12-1, 0, wild, NULL, 0, 0,
	};
Flicmenu fwild_sel =
	{
	&rootdir_sel,
	NOCHILD,
	193+58-8, 128+2*16, 11*6+4, 12,
	(char *)&wild_stringq,
	see_string_req,
	fq_wild_stringq,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
static Flicmenu fwildp_sel =
	{
	&fwild_sel,
	NOCHILD,
	193-8, 128+2*16, 9*6+4, 12,
	filedata_120 /* "Wildcard:" */,
	blacktext,
	fq_toggle_wild,
	NOGROUP, 0,
	'*',
	NOOPT,
	};
char und_drawer[71] = "c:";
struct stringq drawer_stringq =
	{
	2, 2, 16, 71-1, 0, vs.drawer, und_drawer, 0, 0,
	};
Flicmenu fdrawer_sel =
	{
	&fwildp_sel,
	NOCHILD,
	193+28-8, 128+16, 16*6+4, 12,
	(char *)&drawer_stringq,
	see_string_req,
	fq_drawer_stringq,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};

static Flicmenu fdrawerp_sel =
	{
	&fdrawer_sel,
	NOCHILD,
	193-8, 128+16, 4*6+4, 12,
	filedata_122 /* "Dir:" */,
	blacktext,
	undo_drawer,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
static char und_file[81] = "zippy";
struct stringq file_stringq =
	{
	2, 2, 15, 81-5/* leave room for auto-extend */, 7, vs.file, und_file, 7, 0,
	};
Flicmenu ffile_sel =
	{
	&fdrawerp_sel,
	NOCHILD,
	193+34-8, 128, 15*6+4, 12,
	(char *)&file_stringq,
	see_string_req,
	type_file_name,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
static Flicmenu ffilep_sel =
	{
	&ffile_sel,
	NOCHILD,
	193-6, 128, 5*6, 12,
	filedata_124 /* "File:" */,
	blacktext,
	undo_file,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
struct name_scroller fscroller;
Flicmenu flist_sel =
	{
	&ffilep_sel,
	NOCHILD,
	20, 115, 74+6, 11*7+5,
	(char *)&fscroller,
	print_list,
	fsel_name,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};

static Flicmenu fdown_sel =
	{
	&flist_sel,
	NOCHILD,
	3, 185, 14, 12,
	(char *)&ctridown,
	ccorner_cursor,
	fincdown,
	(WORD *)&fscroller, 0,
	NOKEY,
	NOOPT,
	};

Flicmenu fscroll_sel =
	{
	&fdown_sel,
	NOCHILD,
	3, 129, 14, 54,
	(char *)&fscroller,
	see_scroll,
	ffeelscroll,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};

static Flicmenu fup_sel =
	{
	&fscroll_sel,
	NOCHILD,
	3, 115, 14, 12,
	(char *)&ctriup,
	ccorner_cursor,
	fincup,
	(WORD *)&fscroller, 0,
	NOKEY,
	NOOPT,
	};

Flicmenu ftitle_sel =
	{
	&fup_sel,
	NOCHILD,
	0, 103, 319, 9,
	NULL, /* gets set to prompt by get_filename(prompt) */
	gary_see_title,
	move_menu,
	NOGROUP, 0,
	NOKEY,
	bottom_menu,
	};

Flicmenu fileq_menu = 
	{
	NONEXT,
	&ftitle_sel,
	0, 103,	319, 96,
	NOTEXT,
	gary_menu_back,
	NOFEEL,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
