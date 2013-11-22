/* Files.c - Let user choose whether to load/save/delete any of our 12
   basic file types.  If you #define NOSAVE this will disable saving. */

/* generated with makemenu */
#include "jimk.h"
#include "flicmenu.h"
#include "dosstuff.h"
#include "poly.h"
#include "files.str"


/*** Display Functions ***/
extern ccorner_text(), ncorner_text(), gary_menu_back(),
	dcorner_text(), draw_fml();

extern toggle_group(), change_mode(),
	close_menu(), move_menu(), bottom_menu(), move_tab_text();

extern fml_load(), fml_save(), fml_info(), fml_kill(),
	fml_change_mode();

extern char *text_buf;

static WORD file_type;

static char *file_ends[] =
	{
	".FLI",
	".GIF",
	".CEL",
	".COL",
	".TXT",
	".FNT",
	".PLY",
	".PLY",
	".OPT",
	".SET",
	".MSK",
	".REC",
	};


/*** Button Data ***/
static Flicmenu fml_mac_sel = {
	NONEXT,
	NOCHILD,
	256, 188, 52, 8,
	files_112 /* "record" */,
	ccorner_text,
	fml_change_mode,
	&file_type,11,
	NOKEY,
	NOOPT,
	};
static Flicmenu fml_ste_sel = {
	&fml_mac_sel,
	NOCHILD,
	256, 176, 52, 8,
	files_113 /* "mask" */,
	ccorner_text,
	fml_change_mode,
	&file_type,10,
	NOKEY,
	NOOPT,
	};
static Flicmenu fml_def_sel = {
	&fml_ste_sel,
	NOCHILD,
	256, 164, 52, 8,
	files_114 /* "settings" */,
	ccorner_text,
	fml_change_mode,
	&file_type,9,
	NOKEY,
	NOOPT,
	};
static Flicmenu fml_opt_sel = {
	&fml_def_sel,
	NOCHILD,
	256, 153, 52, 8,
	files_115 /* "optic" */,
	ccorner_text,
	fml_change_mode,
	&file_type,8,
	NOKEY,
	NOOPT,
	};
static Flicmenu fml_pat_sel = {
	&fml_opt_sel,
	NOCHILD,
	199, 188, 52, 8,
	files_116 /* "path" */,
	ccorner_text,
	fml_change_mode,
	&file_type,7,
	NOKEY,
	NOOPT,
	};
static Flicmenu fml_pol_sel = {
	&fml_pat_sel,
	NOCHILD,
	199, 176, 52, 8,
	files_117 /* "polygon" */,
	ccorner_text,
	fml_change_mode,
	&file_type,6,
	NOKEY,
	NOOPT,
	};
static Flicmenu fml_fon_sel = {
	&fml_pol_sel,
	NOCHILD,
	199, 164, 52, 8,
	files_118 /* "font" */,
	ccorner_text,
	fml_change_mode,
	&file_type,5,
	NOKEY,
	NOOPT,
	};
static Flicmenu fml_tex_sel = {
	&fml_fon_sel,
	NOCHILD,
	199, 153, 52, 8,
	files_119 /* "text" */,
	ccorner_text,
	fml_change_mode,
	&file_type,4,
	NOKEY,
	NOOPT,
	};
static Flicmenu fml_pal_sel = {
	&fml_tex_sel,
	NOCHILD,
	142, 188, 52, 8,
	files_120 /* "palette" */,
	ccorner_text,
	fml_change_mode,
	&file_type,3,
	NOKEY,
	NOOPT,
	};
static Flicmenu fml_cel_sel = {
	&fml_pal_sel,
	NOCHILD,
	142, 176, 52, 8,
	files_121 /* "cel" */,
	ccorner_text,
	fml_change_mode,
	&file_type,2,
	NOKEY,
	NOOPT,
	};
static Flicmenu fml_fra_sel = {
	&fml_cel_sel,
	NOCHILD,
	142, 164, 52, 8,
	files_122 /* "picture" */,
	ccorner_text,
	fml_change_mode,
	&file_type,1,
	NOKEY,
	NOOPT,
	};
static Flicmenu fml_vid_sel = {
	&fml_fra_sel,
	NOCHILD,
	142, 153, 52, 8,
	files_123 /* "flic" */,
	ccorner_text,
	fml_change_mode,
	&file_type,0,
	NOKEY,
	NOOPT,
	};
static Flicmenu fml_kil_sel = {
	&fml_vid_sel,
	NOCHILD,
	73, 184, 54, 10,
	files_124 /* "delete" */,
	dcorner_text,
	fml_kill,
	NOGROUP,0,
	'd',
	NOOPT,
	};
static Flicmenu fml_sav_sel = {
	&fml_kil_sel,
	NOCHILD,
	12, 184, 54, 10,
	files_125 /* "save" */,
	dcorner_text,
	fml_save,
	NOGROUP,0,
	's',
	NOOPT,
	};
static Flicmenu fml_loa_sel = {
	&fml_sav_sel,
	NOCHILD,
	12, 169, 54, 10,
	files_126 /* "load" */,
	dcorner_text,
	fml_load,
	NOGROUP,0,
	'l',
	NOOPT,
	};
static Flicmenu fml_exi_sel = {
	&fml_loa_sel,
	NOCHILD,
	73, 169, 54, 10,
	files_127 /* "exit" */,
	dcorner_text,
	close_menu,
	NOGROUP,0,
	'e',
	NOOPT,
	};
static Flicmenu fml_mini_sel = {
	&fml_exi_sel,
	&minitime_sel,
	60, 154, 81, 8,
	NOTEXT,
	hang_child,
	NOFEEL,
	NOGROUP,0,
	NOKEY,
	NOOPT,
	};
static Flicmenu fml_fil_sel = {
	&fml_mini_sel,
	NOCHILD,
	5, 153, 44, 10,
	files_128 /* "FILES" */,
	move_tab_text,
	move_menu,
	NOGROUP,0,
	NOKEY,
	bottom_menu,
	};
static Flicmenu fml_menu = {
	NONEXT,
	&fml_fil_sel,
	0, 150, 319, 49,
	NOTEXT,
	draw_fml,
	NOFEEL,
	NOGROUP,0,
	NOKEY,
	NOOPT,
	};

#ifdef SLUFFED
char save_ok, load_ok, info_ok;
#endif SLUFFED

static
draw_fml(m)
Flicmenu *m;
{
gary_menu_back(m);
enable_saves();
}

qcopyfile(char *sname, char *dname)
{
if (!jexists(sname))
	cant_find(sname);
else
	jcopyfile(sname, dname);
}

static verify_poly(char *name)
{
Poly p;

if (read_gulp(name, &p, (long)sizeof(p)))
	{
	if (p.polymagic == POLYMAGIC)
		return(1);
	continu_line(files_138   /* "Not a polygon/path file." */);
	}
return(0);
}

static
load_path(name)
char *name;
{
if (verify_poly(name))
	qcopyfile(name,ppoly_name);
}

static
save_path(name)
char *name;
{
qcopyfile(ppoly_name,name);
}

static
load_polygon(name)
char *name;
{
if (verify_poly(name))
	qcopyfile(name,poly_name);
}

static
save_polygon(name)
char *name;
{
qcopyfile(poly_name, name);
}

static
qload_path()
{
char *title;

if ((title = get_filename(files_129 /* "Load path file?" */, ".PLY")) != NULL)
	{
	load_path(title);
	a3d_disables();
	}
}

static
qsave_path()
{
char *title;

if ((title = get_filename(files_131 /* "Save path file?" */, ".PLY")) != NULL)
	{
	if (overwrite_old(title))
		save_path(title);
	}
}

static
qload_polygon()
{
char *title;

if ((title = get_filename(files_133 /* "Load polygon file?" */, ".PLY")) != NULL)
	{
	load_polygon(title);
	a3d_disables();
	}
}

static
qsave_polygon()
{
char *title;

if ((title = get_filename(files_135 /* "Save polygon file?" */, ".PLY")) != NULL)
	{
	if (overwrite_old(title))
		save_polygon(title);
	}
}


static
fml_save()
{
#ifdef NOSAVE
#else /* NOSAVE */
hide_mp();
switch (file_type)
	{
	case 0:	/* video */
		qsave();
		break;
	case 1: /* frame */
		qsave_pic();
		break;
	case 2: /* cel */
		qsave_cel();
		break;
	case 3: /* palette */
		qsave_palette();
		break;
	case 4:	/* text */
		qsave_text();
		break;
	case 5: /* font */
		break;
	case 6: /* polygon */
		qsave_polygon();
		break;
	case 7:	/* path */
		qsave_path();
		break;
	case 8: /* optic */
		qsave_a3d();
		break;
	case 9: /* defaults */
		qsave_defaults();
		break;
	case 10: /* stencil */
		qsave_mask();
		break;
	case 11: /* macro */
		save_macro();
		break;
	}
check_dfree();
draw_mp();
#endif /* NOSAVE */
}

static
fml_load()
{
hide_mp();
switch (file_type)
	{
	case 0:	/* video */
		qload();
		break;
	case 1: /* frame */
		qload_pic();
		break;
	case 2: /* cel */
		qload_cel();
		break;
	case 3: /* palette */
		qload_palette();
		break;
	case 4:	/* text */
		qload_text();
		break;
	case 5: /* font */
		qfont_text();
		break;
	case 6: /* polygon */
		qload_polygon();
		break;
	case 7:	/* path */
		qload_path();
		break;
	case 8: /* optic */
		qload_a3d();
		break;
	case 9: /* defaults */
		qload_defaults();
		break;
	case 10: /* stencil */
		qload_mask();
		break;
	case 11: /* macro */
		load_macro();
		break;
	}
draw_mp();
}




#ifdef LATER
fml_info()
{
struct fndata *fn;
extern char file_only[];
char *title;
char b1[20],b2[40],*bs[6];
unsigned d,t;
unsigned year,month,day,hour,minute,second;
char *afternoon;

hide_mp();
if ((title = get_filename("File information", file_ends[file_type])) != NULL)
	{
	fn = find_dta();
	if (find_first(file_only, 0))
		{
		bs[0] = title;
		bs[1] = cst_;
		sprintf(b1, "%ld bytes", fn->size);
		bs[2] = b1;

		day = fn->date;
		year = (day>>9)+1980;
		day &= 0x01ff;
		month = (day>>5) + 1;
		day &= 0x1f;

		second = fn->time;
		hour = (second>>11);
		second &= 0x07ff;
		minute = (second>>5);
		second &= 0x1f;
		if (hour >= 12)
			{
			hour -= 12;
			afternoon = "pm";
			}
		else
			afternoon = "am";
		if (hour == 0)
			hour = 12;
		sprintf(b2, "%d/%d/%d %d:%d:%d %s", month, day+1, year,
			hour, minute, 2*second, afternoon);
		bs[3] = b2;
		bs[4] = NULL;
		continu_box(bs);
		}
	}
draw_mp();
}
#endif LATER

static
fml_kill()
{
char *title;

hide_mp();
if ((title = get_filename(files_137 /* "Delete a file?" */, file_ends[file_type]))!= NULL)
	{
	if (really_delete(title))
		{
		jdelete_rerr(title);
		}
	}
draw_mp();
}


static
enable_saves()
{
#ifdef NOSAVE
fml_sav_sel.disabled = 1;
#else /* NOSAVE */
fml_sav_sel.disabled = 0;
switch (file_type)
	{
	case 0:	/* video */
		break;
	case 1: /* frame */
		break;
	case 2: /* cel */
		fml_sav_sel.disabled = (cel == NULL);
		break;
	case 3: /* palette */
		break;
	case 4:	/* text */
		fml_sav_sel.disabled = !jexists(text_name);
		break;
	case 5: /* font */
		fml_sav_sel.disabled = 1;
		break;
	case 6: /* polygon */
		fml_sav_sel.disabled = !jexists(poly_name);
		break;
	case 7:	/* path */
		fml_sav_sel.disabled = !jexists(ppoly_name);
		break;
	case 8: /* optic */
		break;
	case 9: /* defaults */
		break;
	case 10: /* stencil */
		fml_sav_sel.disabled = (mask_plane == NULL);
		break;
	case 11: /* macro */
		fml_sav_sel.disabled = !jexists(tmacro_name);
		break;
	}
#endif /* NOSAVE */
}

static
fml_change_mode(m)
Flicmenu *m;
{
change_mode(m);
enable_saves();
draw_sel(&fml_sav_sel);
}


go_files(type)
int type;
{
file_type = type;
clip_rmove_menu(&fml_menu, 
	quick_menu.x - fml_menu.x, quick_menu.y-fml_menu.y); 
do_menu(&fml_menu);
}

