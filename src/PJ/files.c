/* Files.c - Let user choose whether to load/save/delete any of our 12
   basic file types. */

#include "jimk.h"
#include "a3d.h"
#include "commonst.h"
#include "filemenu.h"
#include "flicel.h"
#include "mask.h"
#include "menus.h"
#include "options.h"
#include "palmenu.h"
#include "rastcurs.h"
#include "softmenu.h"
#include "textedit.h"

/*** Display Functions ***/

static void fml_save(void);
static void fml_load(void);
static void fml_kill(void);
static void enable_saves(void);
static void config_fml(void);
static void fml_change_mode(Button *m);

static char *file_ends[] =
	{
	".FLC",
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
	".TWE",
	};

static UBYTE file_paths[] = {
	FLI_PATH,
	PIC_PATH,
	CEL_PATH,
	PALETTE_PATH,
	TEXT_PATH,
	-1,
	POLY_PATH,
	OPTPATH_PATH,
	OPTICS_PATH,
	SETTINGS_PATH,
	MASK_PATH,
	MACRO_PATH,
	TWEEN_PATH,
};


/*** Button Data ***/
static Button fml_mac_sel = MB_INIT1(
	NONEXT,
	NOCHILD,
	52+1,8+1,256,(188)-150,
	NODATA,
	dcorner_text,
	fml_change_mode,
	NOOPT,
	&vs.file_type,FTP_MACRO,
	NOKEY,
	MB_B_GHILITE
	);
static Button fml_ste_sel = MB_INIT1(
	&fml_mac_sel,
	NOCHILD,
	52+1,8+1,256,(176)-150,
	NODATA,
	dcorner_text,
	fml_change_mode,
	NOOPT,
	&vs.file_type,FTP_MASK,
	NOKEY,
	MB_B_GHILITE
	);
static Button fml_def_sel = MB_INIT1(
	&fml_ste_sel,
	NOCHILD,
	52+1,8+1,256,(164)-150,
	NODATA,
	dcorner_text,
	fml_change_mode,
	NOOPT,
	&vs.file_type,FTP_SETTINGS,
	NOKEY,
	MB_B_GHILITE
	);
static Button fml_opt_sel = MB_INIT1(
	&fml_def_sel,
	NOCHILD,
	52+1,8+1,256,(153)-150,
	NODATA,
	dcorner_text,
	fml_change_mode,
	NOOPT,
	&vs.file_type,FTP_OPTICS,
	NOKEY,
	MB_B_GHILITE
	);
static Button fml_pat_sel = MB_INIT1(
	&fml_opt_sel,
	NOCHILD,
	52+1,8+1,199,(188)-150,
	NODATA,
	dcorner_text,
	fml_change_mode,
	NOOPT,
	&vs.file_type,FTP_PATH,
	NOKEY,
	MB_B_GHILITE
	);
static Button fml_pol_sel = MB_INIT1(
	&fml_pat_sel,
	NOCHILD,
	52+1,8+1,199,(176)-150,
	NODATA,
	dcorner_text,
	fml_change_mode,
	NOOPT,
	&vs.file_type,FTP_POLY,
	NOKEY,
	MB_B_GHILITE
	);
static Button fml_fon_sel = MB_INIT1(
	&fml_pol_sel,
	NOCHILD,
	52+1,8+1,199,(164)-150,
	NODATA,
	dcorner_text,
	fml_change_mode,
	NOOPT,
	&vs.file_type,FTP_FONT,
	NOKEY,
	MB_B_GHILITE
	);
static Button fml_tex_sel = MB_INIT1(
	&fml_fon_sel,
	NOCHILD,
	52+1,8+1,199,(153)-150,
	NODATA,
	dcorner_text,
	fml_change_mode,
	NOOPT,
	&vs.file_type,FTP_TEXT,
	NOKEY,
	MB_B_GHILITE
	);
static Button fml_twe_sel = MB_INIT1(
	&fml_tex_sel,
	NOCHILD,
	52+1,8+1,142,(200)-150,
	NODATA,
	dcorner_text,
	fml_change_mode,
	NOOPT,
	&vs.file_type,FTP_TWEEN,
	NOKEY,
	MB_B_GHILITE
	);
static Button fml_pal_sel = MB_INIT1(
	&fml_twe_sel,
	NOCHILD,
	52+1,8+1,142,(188)-150,
	NODATA,
	dcorner_text,
	fml_change_mode,
	NOOPT,
	&vs.file_type,FTP_PALETTE,
	NOKEY,
	MB_B_GHILITE
	);
static Button fml_cel_sel = MB_INIT1(
	&fml_pal_sel,
	NOCHILD,
	52+1,8+1,142,(176)-150,
	NODATA,
	dcorner_text,
	fml_change_mode,
	NOOPT,
	&vs.file_type,FTP_CEL,
	NOKEY,
	MB_B_GHILITE
	);
static void right_pic(Button *b)
{
	fml_change_mode(b);
	go_pic_pdr_menu();
}
static Button fml_fra_sel = MB_INIT1(
	&fml_cel_sel,
	NOCHILD,
	52+1,8+1,142,(164)-150,
	NODATA,
	dcorner_text,
	fml_change_mode,
	right_pic,
	&vs.file_type,FTP_PIC,
	NOKEY,
	MB_B_GHILITE
	);
static void right_flic(Button *b)
{
	fml_change_mode(b);
	go_flic_pdr_menu();
}
static Button fml_vid_sel = MB_INIT1(
	&fml_fra_sel,
	NOCHILD,
	52+1,8+1,142,(153)-150,
	NODATA,
	dcorner_text,
	fml_change_mode,
	right_flic,
	&vs.file_type,FTP_FLIC,
	NOKEY,
	MB_B_GHILITE
	);
static Button fml_kil_sel = MB_INIT1(
	&fml_vid_sel,
	NOCHILD,
	54+1,10+1,73,(184+6)-150,
	NODATA,
	ccorner_text,
	fml_kill,
	NOOPT,
	NOGROUP,0,
	'd',
	0
	);
static Button fml_sav_sel = MB_INIT1(
	&fml_kil_sel,
	NOCHILD,
	54+1,10+1,12,(184+6)-150,
	NODATA,
	ccorner_text,
	fml_save,
	NOOPT,
	NOGROUP,0,
	's',
	0
	);
static Button fml_loa_sel = MB_INIT1(
	&fml_sav_sel,
	NOCHILD,
	54+1,10+1,12,(169+6)-150,
	NODATA,
	ccorner_text,
	fml_load,
	NOOPT,
	NOGROUP,0,
	'l',
	0
	);
static Button fml_exi_sel = MB_INIT1(
	&fml_loa_sel,
	NOCHILD,
	54+1,10+1,73,(169+6)-150,
	NODATA,
	ccorner_text,
	mb_close_ok,
	NOOPT,
	NOGROUP,0,
	'e',
	0
	);

extern Minitime_data flxtime_data;

static Button fml_mini_sel = MB_INIT1(
	&fml_exi_sel,
	&minitime_sel,
	81+1,8+1,63,(154)-150,
	NOTEXT,
	hang_children,
	NOFEEL,
	NOOPT,
	&flxtime_data,0,
	NOKEY,
	0
	);
static Button fml_fil_sel = MB_INIT1(
	&fml_mini_sel,
	NOCHILD,
	55,10+1,5,(153)-150,
	NODATA,
	see_titlebar,
	feel_titlebar,
	mb_menu_to_bottom,
	&tbg_moveclose,0,
	'q',
	0
	);

static Menuhdr fml_menu = {
	{320,62,0,150},		/* width, height, x, y */
	FILE_MUID,  		/* id */
	PANELMENU,			/* type */
	&fml_fil_sel, 		/* buttons */
	SCREEN_FONT, 		/* font */
	&menu_cursor.hdr,	/* cursor */
	seebg_white, 		/* seebg */
	NULL,				/* dodata */
	NULL,				/* domenu */
	(MBPEN|MBRIGHT|KEYHIT), /* ioflags */
	0,				/* flags */
	NULL,			/* mw */
	NULL,			/* group */
	{ NULL, NULL },	/* node */
	NULL,			/* procmouse */
	NULL,			/* on_showhide */
	NULL,			/* cleanup */
	0, 0, 0, 0		/* scaled width, height, x, y */
};

static Smu_button_list fml_blist[] = {
	{ "title",      { &fml_fil_sel } },
	{ "record",     { &fml_mac_sel } },
	{ "mask",       { &fml_ste_sel } },
	{ "settings",   { &fml_def_sel } },
	{ "optic",      { &fml_opt_sel } },
	{ "path",       { &fml_pat_sel } },
	{ "poly",       { &fml_pol_sel } },
	{ "font",       { &fml_fon_sel } },
	{ "text",       { &fml_tex_sel } },
	{ "tween",      { &fml_twe_sel } },
	{ "pal",        { &fml_pal_sel } },
	{ "cel",        { &fml_cel_sel } },
	{ "pic",        { &fml_fra_sel } },
	{ "flic",       { &fml_vid_sel } },
	{ "del",        { &fml_kil_sel } },
	{ "save",       { &fml_sav_sel } },
	{ "load",       { &fml_loa_sel } },
	{ "exit",       { &fml_exi_sel } },
};


static Errcode copy_poly_file(char *source, char *dest)
{
Errcode err;

if ((err = check_poly_file(source)) < Success)
	return err;
return pj_copyfile(source, dest);
}

static Errcode load_path(char *name)
{
	return(copy_poly_file(name,ppoly_name));
}

static Errcode save_path(char *name)
{
	return(pj_copyfile(ppoly_name,name));
}

static Errcode load_polygon(char *name)
{
	return(cant_load(copy_poly_file(name,poly_name),name));
}

static Errcode save_polygon(char *name)
{
	return(pj_copyfile(poly_name, name));
}

static void qload_path(void)
{
char *path;
char buf[50];

if ((path = vset_get_filename(stack_string("load_path",buf),
							".PLY",load_str,OPTPATH_PATH,NULL,0)) != NULL)
	{
	cant_load(load_path(path),path);
	a3d_disables();
	}
}

static void qsave_path(void)
{
char *path;
char buf[50];

if ((path = vset_get_filename(stack_string("save_path",buf),
						   ".PLY",save_str,OPTPATH_PATH,NULL,TRUE)) != NULL)
	{
	if (overwrite_old(path))
		{
		save_path(path);
		}
	}
}

static void qload_polygon(void)
{
char *path;
char buf[50];

if ((path = vset_get_filename(stack_string("load_poly",buf),
							".PLY",load_str,POLY_PATH,NULL,0)) != NULL)
	{
	load_polygon(path);
	a3d_disables();
	}
}

static void qsave_polygon(void)
{
char *path;
char buf[50];

if ((path = vset_get_filename(stack_string("save_poly",buf),
							".PLY",save_str,POLY_PATH,NULL,TRUE)) != NULL)
	{
	if (overwrite_old(path))
		{
		save_polygon(path);
		}
	}
}

static Errcode po_save_tween(char *name)
{
return(pj_copyfile(tween_name,name));
}

static Errcode po_load_tween(char *name)
{
Errcode err;

if ((err = test_load_tween(name)) < Success)
	return(cant_load(err,name));
return(cant_load(pj_copyfile(name,tween_name),name));
}

static void qload_tween(void)
{
char *path;
char buf[50];

if ((path = vset_get_filename(stack_string("load_tween",buf),
							".TWE",load_str,TWEEN_PATH,NULL,0)) != NULL)
	{
	po_load_tween(path);
	a3d_disables();
	}
}

static void qsave_tween(void)
{
char *path;
char buf[50];

if ((path = vset_get_filename(stack_string("save_tween",buf), 
							".TWE",save_str,TWEEN_PATH,NULL,TRUE)) != NULL)
	{
	if (overwrite_old(path))
		po_save_tween(path);
	}
}



static void fml_save(void)
{
hide_mp();
switch (vs.file_type)
	{
	case FTP_FLIC:	/* video */
		qsave();
		break;
	case FTP_PIC: /* frame */
		qsave_pic();
		break;
	case FTP_CEL: /* cel */
		qsave_the_cel();
		break;
	case FTP_PALETTE: /* palette */
		qsave_palette();
		break;
	case FTP_TEXT:	/* text */
		qsave_titles();
		break;
	case FTP_FONT: /* font */
		break;
	case FTP_POLY: /* polygon */
		qsave_polygon();
		break;
	case FTP_PATH:	/* path */
		qsave_path();
		break;
	case FTP_OPTICS: /* optic */
		qsave_a3d();
		break;
	case FTP_SETTINGS: /* settings */
		qsave_vsettings();
		break;
	case FTP_MASK: /* stencil */
		qsave_mask();
		break;
	case FTP_MACRO: /* macro */
		qsave_macro();
		break;
	case FTP_TWEEN: /* tween */
		qsave_tween();
		break;
	}
show_mp();
}

static void fml_load(void)
{
hide_mp();
switch (vs.file_type)
	{
	case FTP_FLIC:	/* video */
		qload();
		break;
	case FTP_PIC: /* frame */
		qload_pic();
		break;
	case FTP_CEL: /* cel */
		qload_the_cel();
		break;
	case FTP_PALETTE: /* palette */
		qload_palette();
		break;
	case FTP_TEXT:	/* text */
		qload_titles();
		break;
	case FTP_FONT: /* font */
		qfont_text();
		break;
	case FTP_POLY: /* polygon */
		qload_polygon();
		break;
	case FTP_PATH:	/* path */
		qload_path();
		break;
	case FTP_OPTICS: /* optic */
		qload_a3d();
		break;
	case FTP_SETTINGS: /* settings */
		qload_vsettings();
		break;
	case FTP_MASK: /* stencil */
		qload_mask();
		break;
	case FTP_MACRO: /* macro */
		qload_macro();
		break;
	case FTP_TWEEN: /* tween */
		qload_tween();
		break;
	}
show_mp();
enable_saves();
}
static void fml_kill(void)
{
char *path;
char suffix[WILD_SIZE];
char *ends;
char buf1[50], buf2[16];

	hide_mp();
	ends = file_ends[vs.file_type];
	parse_to_semi(&ends,suffix,sizeof(suffix));
	if ((path = vset_get_filename(stack_string("del_file",buf1),
								suffix,
								stack_string("del_str",buf2),
							    file_paths[vs.file_type],NULL,0)) != NULL)
	{
		if(!pj_exists(path))
			cant_find(path);
		else if(really_delete(path))
			pj_delete(path);
	}
	show_mp();
}


static void enable_saves(void)
{
Boolean disable_save = FALSE;

	switch (vs.file_type)
	{
		default: /* anything out of range defaults to flic */
		case FTP_FLIC:	/* video */
			vs.file_type = FTP_FLIC;
			break;
		case FTP_PIC: /* frame */
			break;
		case FTP_CEL: /* cel */
			disable_save = (thecel == NULL);
			break;
		case FTP_PALETTE: /* palette */
			break;
		case FTP_TEXT:	/* text */
			disable_save = !pj_exists(text_name);
			break;
		case FTP_FONT: /* font */
			disable_save = TRUE;
			break;
		case FTP_POLY: /* polygon */
			disable_save = !pj_exists(poly_name);
			break;
		case FTP_PATH:	/* path */
			disable_save = !pj_exists(ppoly_name);
			break;
		case FTP_OPTICS: /* optic */
			break;
		case FTP_SETTINGS: /* defaults */
			break;
		case FTP_MASK: /* stencil */
			disable_save = (mask_rast == NULL);
			break;
		case FTP_MACRO: /* macro */
			disable_save = !pj_exists(macro_name);
			break;
		case FTP_TWEEN:	/* tween */
			disable_save = !pj_exists(tween_name);
			break;
	}
	draw_button_disable(&fml_sav_sel,disable_save);
}

static void config_fml(void)
{
	enable_saves();
}

static void fml_change_mode(Button *m)
{
	change_mode(m);
	config_fml();
	draw_buttontop(&fml_sav_sel);
}

static void same_old_files(void)
{
void *ss;

	config_fml();
	hide_mp();
	if (soft_buttons("files_panel", fml_blist, 
					 Array_els(fml_blist), &ss) < Success)
	{
		return;
	}
	menu_to_cursor(vb.screen,&fml_menu);
	do_reqloop(vb.screen,&fml_menu,NULL,NULL,NULL);
	smu_free_scatters(&ss);
	show_mp();
}

void go_files(int type)
{
	if(type >= 0)
		vs.file_type = type;
	same_old_files();
}
void mb_go_files(Button *b)
{
	(void)b;
	same_old_files();
}

