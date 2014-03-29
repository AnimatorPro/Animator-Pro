/* options.c - module to handle the draw tools menu.  Also by inadvisable
   kludging handles most of the ink types menu as well. */

#include "jimk.h"
#include "broadcas.h"
#include "brush.h"
#include "commonst.h"
#include "errcodes.h"
#include "filemenu.h"
#include "inks.h"
#include "options.h"
#include "palmenu.h"
#include "pentools.h"
#include "poly.h"
#include "rastcurs.h"
#include "rastext.h"
#include "scroller.h"
#include "softmenu.h"
#include "textedit.h"

static Optgroup_data optgroup;

extern Image ctriup, ctridown;

extern void mtween_curve(), mtween_poly(),
	toggle_zoom(), go_zoom_settings(), movefli_tool();

extern Button om_points_group_sel, om_sratio_group_sel, om_osped_group_sel,
	text_group_sel, curve_group_sel;

static void get_justify(Button *b);
static void copy_has_moved(Button *b);
static void zero_slider(Button *b);
static void sample_text(Button *b);
static void mload_titles(Button *m);
static void mfont_text(void);
static void msave_titles(void);
static void go_poly_files(void);
static void show_help(Button *b);

/* block for Text options */

static Button tsample_sel = MB_INIT1(
	NONEXT,
	NOCHILD,
	130,33,4,45,
	NODATA, /* uses global uvfont */
	sample_text,
	mfont_text,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);
static Button tfont_sel = MB_INIT1(
	&tsample_sel,
	NOCHILD,
	63,11,4,32,
	NODATA, /* "Font", */
	ccorner_text,
	mfont_text,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);
static void save_grey(Button *m)
{
set_button_disable(m,!pj_exists(text_name));
ccorner_text(m);
}
static Button tsave_sel = MB_INIT1(
	&tfont_sel,
	NOCHILD,
	63,11,71,18,
	NODATA, /* "Save", */
	save_grey,
	msave_titles,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);
static Button tload_sel = MB_INIT1(
	&tsave_sel,
	NOCHILD,
	63,11,71,4,
	NODATA, /* "Load", */
	ccorner_text,
	mload_titles,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);
static Button tedit_sel = MB_INIT1(
	&tload_sel,
	NOCHILD,
	63,11,4,18,
	NODATA, /* "Edit", */
	ccorner_text,
	qedit_titles,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);
static Button tplace_sel = MB_INIT1(
	&tedit_sel,
	NOCHILD,
	63,11,4,4,
	NODATA, /* "Reuse", */
	ccorner_text,
	qplace_titles,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);
static Button tjust_sel = MB_INIT1(
	&tplace_sel,
	NOCHILD,
	63,11,71,32,
	NODATA, /* "Justify", */
	ccorner_text,
	get_justify,
	get_justify,
	NOGROUP,0,
	NOKEY,
	0
	);
Button text_group_sel = MB_INIT1(
	NONEXT,
	&tjust_sel,
	138,82,0,0,
	NOTEXT,
	NOSEE,
	NOFEEL,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);

static void get_justify(Button *b)
{
int choice;
static char mname[] = "justify";
USHORT mdis[5];
(void)b;

/* set up asterisks and disables */
clear_mem(mdis, sizeof(mdis));
mdis[vs.tit_just] = QCF_ASTERISK;
choice = soft_qchoice(mdis, mname);
if (choice >= 0)
	vs.tit_just = choice;
}

/* -----------------Block for move/copy --------------*/
static Button move_copy_sel = MB_INIT1(
	NONEXT,
	NOCHILD,
	63,11,39,48,
	NODATA, /* "Copy", */
	ccorner_text,
	copy_has_moved,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	MB_B_GHILITE
	);
static Button move_one_color_sel = MB_INIT1(
	&move_copy_sel,
	NOCHILD,
	63,11,39,34,
	NODATA, /* "One Color", */
	ccorner_text,
	toggle_bgroup,
	NOOPT,
	&vs.render_one_color,1,
	'1',
	MB_B_GHILITE
	);
static Button move_under_sel = MB_INIT1(
	&move_one_color_sel,
	NOCHILD,
	63,11,39,20,
	NODATA, /* "Under", */
	ccorner_text,
	toggle_bgroup,
	NOOPT,
	&vs.render_under,1,
	'v',
	MB_B_GHILITE
	);
Button move_group_sel = MB_INIT1(
	NONEXT,
	&move_under_sel,
	138,82,0,0,
	NOTEXT,
	NOSEE,
	NOFEEL,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);
static void copy_has_moved(Button *b)
{
	(void)b;
	soft_continu_box("copy_has_moved");
}
/* -----------------Block for separate options --------------*/

static Qslider osep_rgb_sl = 
	QSL_INIT1( 0, 100, &vs.sep_threshold, 0, NULL, leftright_arrs);


static Button osep_rgb_sl_sel = MB_INIT1(
	NONEXT,
	NOCHILD,
	126,11,6,64,
	&osep_rgb_sl,
	see_abovetext_qslider,
	feel_qslider,
	NOOPT,
	NOGROUP, /* "Near Threshold", */ 0,
	NOKEY,
	0
	);
static Button sep_boxed_sel = MB_INIT1(
	&osep_rgb_sl_sel,
	NOCHILD,
	57,13,75,21,
	NODATA, /* "Boxed", */
	dcorner_text,
	toggle_bgroup,
	NOOPT,
	&vs.sep_box,1,
	NOKEY,
	MB_GHILITE
	);
static Button sep_close_sel = MB_INIT1(
	&sep_boxed_sel,
	NOCHILD,
	63,11,6,37,
	NODATA, /* "Near", */
	ccorner_text,
	change_mode,
	NOOPT,
	&vs.sep_rgb,1,
	NOKEY,
	MB_B_GHILITE
	);
static Button sep_range_sel = MB_INIT1(
	&sep_close_sel,
	NOCHILD,
	63,11,6,22,
	NODATA, /* "Cluster", */
	ccorner_text,
	change_mode,
	NOOPT,
	&vs.sep_rgb,2,
	NOKEY,
	MB_B_GHILITE
	);
static Button sep_single_sel = MB_INIT1(
	&sep_range_sel,
	NOCHILD,
	63,11,6,7,
	NODATA, /* "Single", */
	ccorner_text,
	change_mode,
	NOOPT,
	&vs.sep_rgb,0,
	NOKEY,
	MB_B_GHILITE
	);
Button sep_group_sel = MB_INIT1(
	NONEXT,
	&sep_single_sel,
	138,82,0,0,
	NOTEXT,
	NOSEE,
	NOFEEL,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);
/* -----------------Block for Spray submenu --------------- */
static Qslider osprd_sl = 
	QSL_INIT1( 1, 320, &vs.air_spread, 0, NULL, leftright_arrs);

static Button os_psens_sel = MB_INIT1(
	NONEXT,
	NOCHILD,
	69,20,35,6,
	NODATA, /* "Pres Sens", */
	ccorner_text,
	NOFEEL,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);


static Button osprd_sel = MB_INIT1(
	&os_psens_sel,
	NOCHILD,
	121,11,8,64,
	&osprd_sl,
	see_abovetext_qslider,
	feel_qslider,
	NOOPT,
	NOGROUP, /* "Spray Width", */ 0,
	NOKEY,
	0
	);

static Qslider osped_sl = QSL_INIT1( 1, 100, &vs.air_speed, 0, NULL, leftright_arrs);

static Button osped_sel = MB_INIT1(
	&osprd_sel,
	NOCHILD,
	121,11,8,39,
	&osped_sl,
	see_abovetext_qslider,
	feel_qslider,
	NOOPT,
	NOGROUP, /* "Air Speed", */ 0,
	NOKEY,
	0
	);
Button om_osped_group_sel = MB_INIT1(
	NONEXT,
	&osped_sel,
	138,82,0,0,
	NOTEXT,
	NOSEE,
	NOFEEL,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);

/* ----------- block for curve options -----------**/
static Qslider otens_sl =
	QSL_INIT1( -20, 20, &vs.sp_tens, 0, NULL, leftright_arrs);
static Qslider ocont_sl =
	QSL_INIT1( -20, 20, &vs.sp_cont, 0, NULL, leftright_arrs);
static Qslider obias_sl =
	QSL_INIT1( -20, 20, &vs.sp_bias, 0, NULL, leftright_arrs);

static Button bias_sl_sel = MB_INIT1(
	NONEXT,
	NOCHILD,
	106,11,29,66,
	&obias_sl,
	see_qslider,
	feel_qslider,
	zero_sl,
	NOGROUP,0,
	NOKEY,
	0
	);
static Button cont_sl_sel = MB_INIT1(
	&bias_sl_sel,
	NOCHILD,
	106,11,29,53,
	&ocont_sl,
	see_qslider,
	feel_qslider,
	zero_sl,
	NOGROUP,0,
	NOKEY,
	0
	);
static Button tens_sl_sel = MB_INIT1(
	&cont_sl_sel,
	NOCHILD,
	106,11,29,40,
	&otens_sl,
	see_qslider,
	feel_qslider,
	zero_sl,
	NOGROUP,0,
	NOKEY,
	0
	);
static Button biastag_sel = MB_INIT1(
	&tens_sl_sel,
	NOCHILD,
	28,11,1,66,
	NODATA, /* "Bias", */
	black_ctext,
	zero_slider,
	NOOPT,
	(void *)&bias_sl_sel,0,
	NOKEY,
	0
	);
static Button conttag_sel = MB_INIT1(
	&biastag_sel,
	NOCHILD,
	28,11,1,53,
	NODATA, /* "Cont", */
	black_ctext,
	zero_slider,
	NOOPT,
	&cont_sl_sel,0,
	NOKEY,
	0
	);
static Button tenstag_sel = MB_INIT1(
	&conttag_sel,
	NOCHILD,
	28,11,1,40,
	NODATA, /* "Tens", */
	black_ctext,
	zero_slider,
	NOOPT,
	&tens_sl_sel,0,
	NOKEY,
	0
	);
static Button c2curve_sel = MB_INIT1(
	&tenstag_sel,
	NOCHILD,
	65,12,4,14,
	NODATA, /* "2 Color", */
	ccorner_text,
	toggle_bgroup,
	NOOPT,
	&vs.color2,1,
	'2',
	MB_B_GHILITE
	);
static Button fcurve_sel = MB_INIT1(
	&c2curve_sel,
	NOCHILD,
	65,12,4,3,
	NODATA, /* "Filled", */
	ccorner_text,
	toggle_bgroup,
	NOOPT,
	&vs.fillp,1,
	NOKEY,
	MB_B_GHILITE
	);
static Button ccurve_sel = MB_INIT1(
	&fcurve_sel,
	NOCHILD,
	65,12,4,25,
	NODATA, /* "Closed", */
	ccorner_text,
	toggle_bgroup,
	NOOPT,
	&vs.closed_curve,1,
	NOKEY,
	MB_B_GHILITE
	);
static void see_poly_reuse(Button *b)
{
	set_button_disable(b,!pj_exists(poly_name));
	ccorner_text(b);
}
static Button ecurve_sel = MB_INIT1(
	&ccurve_sel,
	NOCHILD,
	64,12,71,7,
	NODATA, /* "Reuse", */
	see_poly_reuse,
	edit_curve,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);
Button curve_group_sel = MB_INIT1(
	NONEXT,
	&ecurve_sel,
	138,82,0,0,
	NOTEXT,
	NOSEE,
	NOFEEL,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);

/* -----------------Block for filled/2color --------------------*/
static Button color2_sel = MB_INIT1(
	NONEXT,
	NOCHILD,
	63,11,39,41,
	NODATA, /* "2 Color", */
	ccorner_text,
	toggle_bgroup,
	NOOPT,
	&vs.color2,1,
	'2',
	MB_B_GHILITE
	);
static Button filledp_sel = MB_INIT1(
	&color2_sel,
	NOCHILD,
	63,11,39,25,
	NODATA, /* "Filled", */
	ccorner_text,
	toggle_bgroup,
	NOOPT,
	&vs.fillp,1,
	NOKEY,
	MB_B_GHILITE
	);
Button fill2c_group_sel = MB_INIT1(
	NONEXT,
	&filledp_sel,
	138,82,0,0,
	NOTEXT,
	NOSEE,
	NOFEEL,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);

/* -----------------Block for boxes --------------------------------*/

static Qslider bevel_sl = 
	QSL_INIT1( 0, 16, &vs.box_bevel, 0, NULL, leftright_arrs);
static Button bevel_slide_sel = MB_INIT1(
	NONEXT,
	NOCHILD,
	110,11,15,61,
	&bevel_sl,
	see_abovetext_qslider,
	feel_qslider,
	NOOPT,
	NOGROUP, /* "Bevel", */ 0,
	NOKEY,
	0
	);

static Button bcolor2_sel = MB_INIT1(
	&bevel_slide_sel,
	NOCHILD,
	63,11,37,26,
	NODATA, /* "2 Color", */
	ccorner_text,
	toggle_bgroup,
	NOOPT,
	&vs.color2,1,
	'2',
	MB_B_GHILITE
	);
static Button filledb_sel = MB_INIT1(
	&bcolor2_sel,
	NOCHILD,
	63,11,37,10,
	NODATA, /* "Filled", */
	ccorner_text,
	toggle_bgroup,
	NOOPT,
	&vs.fillp,1,
	NOKEY,
	MB_B_GHILITE
	);
Button box_group_sel = MB_INIT1(
	NONEXT,
	&filledb_sel,
	138,82,0,0,
	NOTEXT,
	NOSEE,
	NOFEEL,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);

/* -----------------Block for freehand polygons --------------------*/
static Button fpg_files_sel = MB_INIT1(
	NONEXT,
	NOCHILD,
	63,11,71,48,
	NODATA, /* "Files", */
	ccorner_text,
	go_poly_files,
	NOOPT,
	NOGROUP, 0,
	NOKEY,
	0
	);
static Button fpg_o1mpg_editp_sel = MB_INIT1(
	&fpg_files_sel,
	NOCHILD,
	63,11,71,20,
	NODATA, /* "Reuse", */
	see_poly_reuse,
	edit_poly,
	NOOPT,
	NOGROUP, 0,
	NOKEY,
	0
	);
static Button fpg_closed_sel = MB_INIT1(
	&fpg_o1mpg_editp_sel,
	NOCHILD,
	63,11,4,48,
	NODATA, /* "Closed", */
	ccorner_text,
	toggle_bgroup,
	NOOPT,
	&vs.closed_curve, 1,
	NOKEY,
	MB_B_GHILITE
	);
static Button fpg_color2_sel = MB_INIT1(
	&fpg_closed_sel,
	NOCHILD,
	63,11,4,34,
	NODATA, /* "2 Color", */
	ccorner_text,
	toggle_bgroup,
	NOOPT,
	&vs.color2, 1,
	'2',
	MB_B_GHILITE
	);
static Button fpg_filledp_sel = MB_INIT1(
	&fpg_color2_sel,
	NOCHILD,
	63,11,4,20,
	NODATA, /* "Filled", */
	ccorner_text,
	toggle_bgroup,
	NOOPT,
	&vs.fillp, 1,
	NOKEY,
	MB_B_GHILITE
	);
Button freepoly_group_sel = MB_INIT1(
	NONEXT,
	&fpg_filledp_sel,
	138,82,0,0,
	NOTEXT,
	NOSEE,
	NOFEEL,
	NOOPT,
	NOGROUP, 0,
	NOKEY,
	0
	);

/* -----------------Block for Star Point slider --------------- */
static Qslider star_points_sl =
	QSL_INIT1( 3, 32, &vs.star_points, 0, NULL, leftright_arrs);

static Button ompg_color2_sel = MB_INIT1(
	NONEXT,
	NOCHILD,
	63,11,39,27,
	NODATA, /* "2 Color", */
	ccorner_text,
	toggle_bgroup,
	NOOPT,
	&vs.color2,1,
	'2',
	MB_B_GHILITE
	);

static Button ompg_filledp_sel = MB_INIT1(
	&ompg_color2_sel,
	NOCHILD,
	63,11,39,11,
	NODATA, /* "Filled", */
	ccorner_text,
	toggle_bgroup,
	NOOPT,
	&vs.fillp,1,
	NOKEY,
	MB_B_GHILITE
	);

static Button omp_slider_sel = MB_INIT1(
	&ompg_filledp_sel,
	NOCHILD,
	110,11,16,62,
	&star_points_sl,
	see_abovetext_qslider,
	feel_qslider,
	NOOPT,
	NOGROUP, /* "POINTS", */ 0,
	NOKEY,
	0
	);

Button om_points_group_sel = MB_INIT1(
	NONEXT,
	&omp_slider_sel,
	138,82,0,0,
	NOTEXT,
	NOSEE,
	NOFEEL,
	NOOPT,
	NOGROUP, 0,
	NOKEY,
	0
	);

/* -----------------Block for Star Ratio slider --------------- */
static Button o1mpg_color2_sel = MB_INIT1(
	NONEXT,
	NOCHILD,
	63,11,39,20,
	NODATA, /* "2 Color", */
	ccorner_text,
	toggle_bgroup,
	NOOPT,
	&vs.color2, 1,
	'2',
	MB_B_GHILITE
	);
static Button o1mpg_filledp_sel = MB_INIT1(
	&o1mpg_color2_sel,
	NOCHILD,
	63,11,39,4,
	NODATA, /* "Filled", */
	ccorner_text,
	toggle_bgroup,
	NOOPT,
	&vs.fillp, 1,
	NOKEY,
	MB_B_GHILITE
	);
static Button osr_points_sl_sel = MB_INIT1(
	&o1mpg_filledp_sel,
	NOCHILD,
	110,11,14,66,
	&star_points_sl,
	see_abovetext_qslider,
	feel_qslider,
	NOOPT,
	NOGROUP, /* "POINTS", */ 0,
	NOKEY,
	0
	);

static Qslider star_ratio_sl =
	QSL_INIT1( 0, 100, &vs.star_ratio,0,NULL, leftright_arrs);
static Button osr_inrad_sl_sel = MB_INIT1(
	&osr_points_sl_sel,
	NOCHILD,
	110,11,14,45,
	&star_ratio_sl,
	see_abovetext_qslider,
	feel_qslider,
	NOOPT,
	NOGROUP, /* "INNER RADIUS RATIO", */ 0,
	NOKEY,
	0
	);

Button om_sratio_group_sel = MB_INIT1(
	NONEXT,
	&osr_inrad_sl_sel,
	138,82,0,0,
	NOTEXT,
	NOSEE,
	NOFEEL,
	NOOPT,
	NOGROUP, 0,
	NOKEY,
	0
	);

static Smu_button_list otool_smblist[] = {
	{ "tfont",          { /* butn */ &tfont_sel } },
	{ "tsave",          { /* butn */ &tsave_sel } },
	{ "tload",          { /* butn */ &tload_sel } },
	{ "tedit",          { /* butn */ &tedit_sel } },
	{ "tplace",         { /* butn */ &tplace_sel } },
	{ "tjust",          { /* butn */ &tjust_sel } },
	{ "copy",           { /* butn */ &move_copy_sel } },
	{ "one_color",      { /* butn */ &move_one_color_sel } },
	{ "under",          { /* butn */ &move_under_sel } },
	{ "sep_boxed",      { /* butn */ &sep_boxed_sel } },
	{ "sep_close",      { /* butn */ &sep_close_sel } },
	{ "sep_range",      { /* butn */ &sep_range_sel } },
	{ "sep_single",     { /* butn */ &sep_single_sel } },
	{ "os_psens",       { /* butn */ &os_psens_sel } },
	{ "biastag",        { /* butn */ &biastag_sel } },
	{ "conttag",        { /* butn */ &conttag_sel } },
	{ "tenstag",        { /* butn */ &tenstag_sel } },
	{ "c2curve",        { /* butn */ &c2curve_sel } },
	{ "fcurve",         { /* butn */ &fcurve_sel } },
	{ "ccurve",         { /* butn */ &ccurve_sel } },
	{ "ecurve",         { /* butn */ &ecurve_sel } },
	{ "color2",         { /* butn */ &color2_sel } },
	{ "filledp",        { /* butn */ &filledp_sel } },
	{ "bcolor2",        { /* butn */ &bcolor2_sel } },
	{ "filledb",        { /* butn */ &filledb_sel } },
	{ "fpg_files",      { /* butn */ &fpg_files_sel } },
	{ "fpg_o1mpg_editp",{ /* butn */ &fpg_o1mpg_editp_sel } },
	{ "fpg_closed",     { /* butn */ &fpg_closed_sel } },
	{ "fpg_color2",     { /* butn */ &fpg_color2_sel } },
	{ "fpg_filledp",    { /* butn */ &fpg_filledp_sel } },
	{ "ompg_color2",    { /* butn */ &ompg_color2_sel } },
	{ "ompg_filledp",   { /* butn */ &ompg_filledp_sel } },
	{ "o1mpg_color2",   { /* butn */ &o1mpg_color2_sel } },
	{ "o1mpg_filledp",  { /* butn */ &o1mpg_filledp_sel } },

	/* string texts, first char is 'T' */
	{ "Tnthresh",   { /* ps */ &osep_rgb_sl_sel.group } },
	{ "Tswid",      { /* ps */ &osprd_sel.group } },
	{ "Tsair",      { /* ps */ &osped_sel.group } },
	{ "Tbevel",     { /* ps */ &bevel_slide_sel.group } },
	{ "Tomppts",    { /* ps */ &omp_slider_sel.group } },
	{ "Tosrpts",    { /* ps */ &osr_points_sl_sel.group } },
	{ "Tinrad",     { /* ps */ &osr_inrad_sl_sel.group } },
};

static void *toolsmbs;
static Errcode nest_load_toolopt_texts(void)
/** allocate and free texts for tool option sub menu buttons **/
{
Errcode err;
	if((err = nest_alloc_brush_texts()) < Success)
		return(err);
	return(soft_buttons("tool_panels", otool_smblist, Array_els(otool_smblist),
						 &toolsmbs ));
}
static void nest_free_toolopt_texts(void)
{
	smu_free_scatters(&toolsmbs);
	nest_free_brush_texts();
}

/* --------- start of stuff always in this menu -----------------*/
static Name_scroller oscroller;

static Button omu_zpan_sel = MB_INIT1(
	NONEXT, /* next */
	&zpan_cycle_group,
	39, 9, 78, 86, /* w,h,x,y */
	NODATA,
	hang_children,
	NOFEEL,
	NOOPT,
	NULL,0,
	NOKEY,
	0
	);
static Button omu_help_sel = MB_INIT1(
	&omu_zpan_sel,
	NONEXT,
	39, 11, 105, 74, /* w,h,x,y */
	NODATA, /* "Help", */
	ccorner_text,
	show_help,
	NOOPT,
	&optgroup, 0,
	NOKEY,
	0
	);
static Button omu_clus_sel = MB_INIT1(
	&omu_help_sel,
	NOCHILD,
	78, 9, 92, 64, /* w,h,x,y */
	NOTEXT,
	see_crb,
	feel_crb,
	ppalette,
	NULL,0,
	NOKEY,
	0
	);
static Button omu_clusid_sel = MB_INIT1(
	&omu_clus_sel,
	NOCHILD,
	12, 9, 78, 64, /* w,h,x,y */
	NODATA, /* "AB", */
	see_clusid,
	toggle_clusid,
	ppalette,
	&omu_clus_sel,0,
	NOKEY,
	0
	);
static Button omu_minipal_sel = MB_INIT1(
	&omu_clusid_sel,
	&minipal_sel,
	29, 9, 78, 54, /* w,h,x,y */
	NOTEXT,
	hang_children,
	NOFEEL,
	NOOPT,
	NULL,0,
	NOKEY,
	0
	);
static Button omu_sbar_sel = MB_INIT1(
	&omu_minipal_sel, /* next */
	NOCHILD, /* children */
	10, 60, 3, 25, /* w,h,x,y */
	&oscroller, /* datme */
	see_scrollbar,
	rt_feel_scrollbar,
	NOOPT,
	&oscroller,0,
	NOKEY,
	0 /* flags */
	);
static Button omu_slist_sel = MB_INIT1(
	&omu_sbar_sel, /* next */
	NOCHILD, /* children */
	52, 82, 15, 14, /* w,h,x,y */
	&oscroller, /* datme */
	see_scroll_names,
	feel_scroll_cels,
	NOOPT,
	&oscroller,0,
	NOKEY,
	0 /* flags */
	);
static Button omu_sdown_sel = MB_INIT1(
	&omu_slist_sel, /* next */
	NOCHILD, /* children */
	12, 10, 2, 86, /* w,h,x,y */
	&ctridown,  /* datme */
	ncorner_image,
	scroll_incdown,
	NOOPT,
	&oscroller, 0,
	NOKEY,
	0 /* flags */
	);
static Button omu_sup_sel = MB_INIT1(
	&omu_sdown_sel, /* next */
	NOCHILD, /* children */
	12, 10, 2, 14, /* w,h,x,y */
	&ctriup,
	ncorner_image,
	scroll_incup,
	NOOPT,
	&oscroller, 0,
	NOKEY,
	0 /* flags */
	);

static Button omu_subopts_sel = MB_INIT1(
	&omu_sup_sel,
	NOCHILD,
	138, 82, 179, 14, /* w,h,x,y */
	NOTEXT,
	hang_toolopts,
	NOFEEL,
	NOOPT,
	&optgroup, 0,
	NOKEY,
	MB_SCALE_ABS /* flags */
	);
static Button omu_opts_sel = MB_INIT1(
	&omu_subopts_sel,
	NOCHILD,
	108, 39, 69, 14, /* w,h,x,y */
	NOTEXT,
	hang_children,
	NOFEEL,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0 /* flags */
	);

static Sgroup1_data omu_sh1dat = {
	&flxtime_data,
};

static Button omu_std1_sel = MB_INIT1(
	&omu_opts_sel,
	&std_head1_sel,
	0, 0, 129, 3, /* w,h,x,y */
	NOTEXT,
	hang_children,
	NOFEEL,
	NOOPT,
	&omu_sh1dat,0,
	NOKEY,
	0 /* flags */
	);
static Button omu_redo_sel = MB_INIT1(
	&omu_std1_sel, /* next */
	NOCHILD, /* children */
	33, 9, 94, 3, /* w,h,x,y */
	NODATA, /* "Redo", */
	see_redo,
	menu_doredo,
	NOOPT,
	NOGROUP,0,
	'r',
	0 /* flags */
	);
static Button omu_undo_sel = MB_INIT1(
	&omu_redo_sel, /* next */
	NOCHILD, /* children */
	33, 9, 59, 3, /* w,h,x,y */
	NODATA, /* "Undo", */
	see_undo,
	menu_doundo,
	NOOPT,
	NOGROUP,0,
	'\b',
	0 /* flags */
	);
static Button omu_title_sel = MB_INIT1(
	&omu_undo_sel,
	NOCHILD,
	53,9,3,3,
	NOTEXT,
	see_titlebar,
	feel_titlebar,
	mb_menu_to_bottom,
	&tbg_moveclose,0,
	'q',
	0 /* flags */
	);

Menuhdr options_menu = {
	{320,98,0,120}, /* width, height, x, y */
	OPT_MUID,		/* id */
	PANELMENU,		/* type */
	&omu_title_sel,	/* buttons */
	SCREEN_FONT,	/* font */
	&menu_cursor.hdr, /* cursor */
	seebg_white, 	/* seebg */
	NULL,			/* dodata */
	NULL,			/* domenu */
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

static Smu_button_list omu_smblist[] = {
	{ "title",  { &omu_title_sel } },
	{ "help",   { &omu_help_sel } },
	{ "clusid", { &omu_clusid_sel } },
	{ "redo",   { &omu_redo_sel } },
	{ "undo",   { &omu_undo_sel } },
};

void see_abovetext_qslider(Button *b)
{
	b->y -= b->height;
	mb_centext(b,mc_black(b),b->group);
	b->y += b->height;
	see_qslider(b);
}
void close_option_tools(Option_tool **tool_list)

/* while detaching tools from list calls tool's "closeit" function */
{
Option_tool *next, *tool;

	tool = *tool_list;
	while(tool)
	{
		next = tool->next;
		if(tool->closeit)
			(*(tool->closeit))(tool);
		else
			tool->next = NULL;
		tool = next;
	}
	*tool_list = NULL;
}
void set_curink(Ink *newink)
{
	vl.ink = newink;
	vs.ink_id = newink->ot.id;
	return;
}
Errcode id_set_curink(int id)
{
Ink *ink;

	if(NULL == (ink = id_find_option(ink_list,id)))
		return(Err_not_found);
	set_curink(ink);
	return(Success);
}

static Errcode notool(Pentool *pt, Wndo *w)
{
	(void)pt;
	(void)w;

	return Success;
}

Pentool null_pentool = PTOOLINIT1(
	NONEXT,
	"",
	PTOOL_OPT,
	NULL_PTOOL,
	NULL,
	NO_SUBOPTS,
	NOCLOSE,
 	notool,
	&plain_ptool_cursor.hdr,
	NULL,
	NULL
);

Errcode set_curptool(Pentool *ptool)
{
Errcode err;

	if(vl.ptool)
	{
		if(vl.ptool == ptool)
			return(Success);
		if(vl.ptool->on_remove)
			(*(vl.ptool->on_remove))(vl.ptool);
	}
	if(ptool == NULL)
		ptool = &null_pentool;
	vl.ptool = ptool;

	/* just in case ********************/
	if(ptool->cursor == NULL)
		ptool->cursor =	&plain_ptool_cursor.hdr;

	if(ptool->on_install)
	{
		if((err = (*(ptool->on_install))(ptool)) < 0)
		{
			err = softerr(err,"!%s", "tool_load", ptool->ot.name );
			vl.ptool = &null_pentool;
		}
		return(err);
	}
	return(Success);
}
void *id_find_option(Option_tool *list, SHORT id)
{
	while(list)
	{
		if(list->id == id)
			break;
		list = list->next;
	}
	return(list);
}
static Errcode id_set_curptool(int id)
{
Pentool *ptool;

	if(NULL == (ptool = (Pentool *)id_find_option(ptool_list,id)))
		return(Err_not_found);
	return(set_curptool(ptool));
}
void set_quickptool(Pentool *ptool)

/* sets pen tool for quick menu */
{
	if(set_curptool(ptool) >= Success)
		vs.ptool_id = ptool->ot.id;
}
Errcode restore_pentool(Pentool *pt)
{
	if(pt != NULL 
		&& slist_ix(ptool_list, pt) >= 0 
		&& pt->ot.id != vs.ptool_id)
	{
		return(id_set_curptool(vs.ptool_id));
	}
	else
		return(set_curptool(pt));
}


void zero_sl(Button *b)
{
Qslider *s;

	s = b->datme;
	*((SHORT *)(s->value)) = 0;
	draw_buttontop(b);
}

static void zero_slider(Button *b)
{
b = (Button *)(b->group);
zero_sl(b);
}


static void refresh_options(void)
{
	draw_button(&omu_opts_sel);
	draw_button(&omu_subopts_sel);
}

static void sample_text(Button *b)
{
int w,h;
char *s;
Clipbox cb;
char buf[PATH_SIZE];

	get_uvfont_name(buf);
	mb_make_clip(b,&cb);
	s = pj_get_path_name(buf);
	w = fstring_width(uvfont, s);
	h = tallest_char(uvfont);
	gftext(&cb, uvfont, s, (b->width - w)/2, 
		   (b->height - h)/2,  mc_grey(b), TM_MASK1, 0);

	if(w > b->width || h > b->height)
		mb_centext(b,mc_black(b),s);
}
static void feel_1_opt(Button *m, Raster *rast, int x, int y, Names *entry,
					  int why)
{
Option_tool *o = (Option_tool *)entry;
Button *match, *obuttons;
UBYTE *slots;
(void)m;
(void)rast;
(void)x;
(void)y;
(void)why;

	if(o->type == INK_OPT)
	{
		obuttons = &ink_opts_sel;
		slots = vs.ink_slots;
		set_curink((Ink *)o);
	}
	else /* if(o->type == PTOOL_OPT) */
	{
		obuttons = &pen_opts_sel;
		slots = vs.tool_slots;
		set_quickptool((Pentool *)o);
	}
	/* if another button's got this option swap them */

	slots[slist_ix(obuttons, optgroup.optb)] = o->id;

	if( (NULL != (match = find_button(obuttons, o->id))) 
		&& (match != optgroup.optb))
	{
		match->datme = optgroup.optb->datme;
		match->identity = optgroup.optb->identity;
		slots[slist_ix(obuttons, match)] = optgroup.optb->identity;
	}
	optgroup.optb->identity = o->id;
	optgroup.optb->datme = o;
	*optgroup.optid = o->id;
	refresh_options();
}

#ifdef SLUFFED
void see_toolhelp(Button *m)
{
#define OPTG ((Optgroup_data *)(m->group))
Option_tool *o;
Vfont *f = m->root->font;
int hbord, vbord;

	o = id_find_option(OPTG->tlist, *(OPTG->optid));

	hbord = fchar_spacing(f,"m")/3;
	vbord = tallest_char(f)/2;
	wbg_ncorner_back(m);
	if(o)
	{
		wwtext(m->root, f,
			   o->help, m->x+hbord, m->y+vbord, m->width-hbord, 
			   m->height-vbord, 0,0, mc_grey(m), TM_MASK1, 0);
	}
}
#endif /* SLUFFED */

void change_ink_mode(Button *m)
{
	change_mode(m);
	set_curink(m->datme);
	if (get_button_hdr(m) == &options_menu)
	{
		optgroup.optb = m;
		refresh_options();
	}
}

void change_pen_mode(Button *m)
{
	change_mode(m);
	set_quickptool(m->datme);
	if (get_button_hdr(m) == &options_menu)
	{
		optgroup.optb = m;
		refresh_options();
	}
}
void see_option_name(Button *b)
{
Option_tool *opt = b->datme;

	b->datme = opt->name;
	dcorner_text(b);
	b->datme = opt;
}
void hang_toolopts(Button *b)
{
Option_tool *o;
#define OPTG ((Optgroup_data *)(b->group))

	/* fudge to make brush selector line up right */
	b->width = mb_mscale_x(b,b->orig_rect.width - 1) + 1;

	if((o = id_find_option(OPTG->tlist, *(OPTG->optid))) != NULL)
		b->children = o->options;
	else
		b->children = NULL;
	bg_hang_children(b);

#undef OPTG
}

static void mload_titles(Button *m)
{
(void)m;

hide_mp();
qload_titles();
show_mp();
}

static void mfont_text(void)
{
	hide_mp();
	qfont_text();
	show_mp();
}

void qload_titles(void)
{
char *title;
char sbuf[50];

	unzoom();
	if ((title =  vset_get_filename(stack_string("load_text",sbuf),
								".TXT",load_str,
								TEXT_PATH,NULL,0))!=NULL)
	{
		cant_load(load_titles(title), title);
	}
	rezoom();
}

static void msave_titles(void)
{
hide_mp();
qsave_titles();
show_mp();
}

static Errcode save_titles(char *title)
{
return(pj_copyfile(text_name, title));
}

void qsave_titles(void)
{
char *title;
char sbuf[50];

if (pj_exists(text_name) )
	{
	if ((title =  vset_get_filename(stack_string("save_text",sbuf), 
								".TXT", save_str,
								TEXT_PATH,NULL,0))!=NULL)
		{
		if (overwrite_old(title))
			{
			save_titles(title);
			}
		}
	}
}

static void go_poly_files(void)
{
	hide_mp();
	go_files(FTP_POLY);
	show_mp();
}


static void iopt_scroller(SHORT topname)
{
	oscroller.top_name = topname;
	oscroller.names = (Names *)(optgroup.tlist);
	oscroller.scroll_sel = &omu_sbar_sel;
	oscroller.list_sel = &omu_slist_sel;
	oscroller.font = vb.screen->mufont;
	oscroller.cels_per_row = 1;
	oscroller.feel_1_cel = feel_1_opt;
	init_name_scroller(&oscroller,vb.screen);
}

static void omu_color_redraw(void *mh, USHORT why)
{
	(void)mh;
	(void)why;

	redraw_head1_ccolor(&omu_std1_sel);
	zpan_ccycle_redraw(&omu_zpan_sel);
	draw_button(&omu_minipal_sel);
	draw_button(&omu_clus_sel);
}

static Redraw_node omu_redraw_node = {
	{ NULL, NULL }, /* node */
	omu_color_redraw,
	NULL,
	NEW_CCOLOR };

static void omu_on_showhide(Menuhdr *mh,Boolean showing)
{
	(void)mh;

	if(showing)
		add_color_redraw(&omu_redraw_node);
	else
		rem_color_redraw(&omu_redraw_node);
}
static Boolean do_options_keys(void)
{
 	if(check_pen_abort())
		return(TRUE);
	return(common_header_keys());
}
static Errcode opt_menu(char *title,Button *m,Button *bgroup,
					    Option_tool *tlist,SHORT *poptnum, SHORT *ptopname)
{
Errcode err;
void *ss;

	if((err = soft_buttons("option_panel", omu_smblist,
							Array_els(omu_smblist), &ss)) < Success)
	{
		return(err);
	}
	optgroup.tlist = tlist;
	optgroup.optid = poptnum;
	optgroup.optb = m;
	omu_title_sel.datme = title;
	omu_opts_sel.children = bgroup;
	iopt_scroller(*ptopname);
	options_menu.on_showhide = omu_on_showhide;
	err = do_menuloop(vb.screen,&options_menu,NULL,NULL,do_options_keys);
	*ptopname = oscroller.top_name;
	smu_free_scatters(&ss);
	return(err);
}

/* button used for toggling pen or inks menu */

static Button omu_inkpen_sel = MB_INIT1(
	&omu_opts_sel,
	NOCHILD,
	53,9,238,3,
	NOTEXT,
	see_option_name,
	mb_gclose_identity,
	mb_gclose_identity,
	NOGROUP,0,
	NOKEY,
	0
	);

#define INKTYPE		1
#define DTOOLTYPE 	2

static Errcode go_pen_or_ink(Button *b,int optype,Boolean noswap)
{
Errcode err = optype;

	if(MENU_ISOPEN(&options_menu))
		return(Err_abort);

	hide_mp();
	omu_inkpen_sel.next = omu_title_sel.children;
	omu_title_sel.children = &omu_inkpen_sel;
	set_button_disable(&omu_inkpen_sel,noswap);
	menu_to_quickcent(&options_menu);

	while(err > 0)
	{
		switch(err)
		{
			case INKTYPE:
			{
				if((err = load_inkopt_texts()) < Success)
					goto ink_out;

				id_set_curink(vs.ink_id);
				if(!b)
					b = find_button(&ink_opts_sel, vs.ink_id);
				omu_inkpen_sel.datme = vl.ptool;
				omu_inkpen_sel.identity = DTOOLTYPE;
				err = opt_menu("Inks",b, &ink_opts_sel,
							    ink_list,&vs.ink_id,&vs.top_ink);

			ink_out:
				free_inkopt_texts();
				break;
			}
			case DTOOLTYPE:
			{
				if((err = nest_load_toolopt_texts()) < Success)
					goto tool_out;

				id_set_curptool(vs.ptool_id);
				if(!b)
					b = find_button(&pen_opts_sel, vs.ptool_id);
				omu_inkpen_sel.datme = vl.ink;
				omu_inkpen_sel.identity = INKTYPE;
				err = opt_menu("Tools",b,&pen_opts_sel,
								ptool_list,&vs.ptool_id,&vs.top_tool);

			tool_out:
				nest_free_toolopt_texts();
				break;
			}
			default:
				err = Err_bad_input;
		}
		b = NULL;
		if(noswap)
			break;
	}
	omu_title_sel.children = omu_inkpen_sel.children;
	show_mp();
	return(err);
}
void go_inkopts(Button *b)
{
	change_ink_mode(b);
	go_pen_or_ink(b,INKTYPE,0);
}
void go_dtoolopts(Button *b)
{
	change_pen_mode(b);
	go_pen_or_ink(b,DTOOLTYPE,0);
}
void qtools(void)
{
Pentool *pt;

	pt = vl.ptool;
	go_pen_or_ink(NULL,DTOOLTYPE,1);
	restore_pentool(pt);
}
void qinks(void)
{
	go_pen_or_ink(NULL,INKTYPE,1);
}
static void show_help(Button *b)
{
#define OPTG ((Optgroup_data *)(b->group))
Option_tool *o;
char *ttype;
char *htext;
char *classkey;
char *soft_text;

	if((o = id_find_option(OPTG->tlist, *(OPTG->optid))) == NULL)
		return;

	switch(o->type)
	{
		case INK_OPT:
			classkey = "ink_texts";
			ttype = "ink";
			break;
		default:
			classkey = "tool_texts";
			ttype = "tool";
	}

	if( ((soft_text = rex_key_or_text(o->help,&htext)) != NULL)
		&& (smu_load_name_text(&smu_sm,classkey,
								soft_text,&soft_text) >= Success))
	{
		htext = soft_text;
	}

	hide_mp();
	continu_box("%s %s:\n\n%s", o->name, ttype, htext);
	smu_free_text(&soft_text);
	show_mp();

#undef OPTG
}
Errcode load_option_names(Option_tool *list, char *symname, 
						  void **ptext, int fill_all)

/* loads names from softmenu namestring text from keys in the tools
 * returns Success if loaded and puts text in *ptext to be freed later */
{
unsigned int count;
Smu_name_scats scts[151];
Smu_name_scats *sc = scts;

	/* load array with ones that are keys and adjust names to actual names
	 * if not found in menu file. Since this changes the actual pointers
	 * to names in the list It may only be called once on each list */

	*ptext = NULL;
	count = 0;
	while(list)
	{
		if((sc->name = rex_key_or_text(list->name,&(list->name))) != NULL)
		{
			if(count > 150)
				return(Err_too_big);
			sc->toload.ps = &(list->name);
			++count;
			++sc;
		}
		list = list->next;
	}

	return(soft_name_scatters(symname, scts, count, ptext,
				  (fill_all?0:SCT_OPTIONAL)));
}
