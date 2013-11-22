/* options.c - module to handle the draw tools menu.  Also by inadvisable
   kludging handles most of the ink types menu as well. */

#include "jimk.h"
#include "flicmenu.h"
#include "gemfont.h"
#include "options.str"

extern UBYTE idmd_lookup[];
extern UBYTE dmd_lookup[];

extern int see_colors2(), see_menu_back(),print_list(),cursor_box(),bwtext(),
	wbtext(), ccolor_box(), right_arrow(), left_arrow(),see_number_slider(),
	white_block(), black_block(), text_boxp1(),toggle_group(),
	ncorner_text(), dcorner_text(), ccorner_text(), gary_menu_back(), bcursor(),
	gary_see_title(), ccorner_cursor(), edit_curve(), edit_poly(),
	see_qslider(), feel_qslider(), close_menu(),
	sample_text(), greytext(), blacktext(),change_mode(),
	tween_poly(), tween_curve(), go_poly_files(),
	ccorner_poly_text(), blacktext(), greytext(), grey_block(), toggle_group(),
	inc_slider(), save_text_text(), dec_slider();
extern int move_menu(), bottom_menu(), mget_color(), pick_option(),
	msave_text(), mload_text(), qedit_text(), mfont_text(),
	qplace_text(),
	hang_option(), hang_child(), opt_name(), opt_help();
extern Flicmenu options_menu, dsel1_sel,dsel_group_sel,fill2c_group_sel,
	freepoly_group_sel, sep_group_sel, itgroup_sel, it0_sel;

extern wtext(), fincup(), fincdown(), ffeelscroll(), see_scroll();
extern struct cursor ctriup, ctridown;


extern draw_tool(), streak_tool(), drizl_tool(), flood_tool(), box_tool(),
	spray_tool(), sep_tool(), circle_tool(),  edge_tool(),
	zero_slider(), zero_sl(),
	line_tool(), fill_tool(), polyf_tool(), 
	shapef_tool(), gel_tool(), text_tool(),
	petlf_tool(), spiral_tool(),
	ovalf_tool(), curve_tool(), move_tool(),
	rpolyf_tool(), starf_tool();

Vector pentools[] = {box_tool, circle_tool, 
	draw_tool, drizl_tool, edge_tool,
	fill_tool, flood_tool, gel_tool, line_tool, 
	move_tool, ovalf_tool, petlf_tool, 
	polyf_tool, rpolyf_tool, 
	sep_tool, shapef_tool, spiral_tool, curve_tool, spray_tool, 
	starf_tool, streak_tool, text_tool};


extern Flicmenu om_points_group_sel, om_sratio_group_sel, om_osped_group_sel,
	text_group_sel, curve_group_sel;


static char in_ink_menu;

static Flicmenu *optm;

static Option_list text_option =
	{
	NONEXT,
	options_100 /* "TEXT" */,
	options_101 /* "Edit text in a boxed area of the screen.  Load ascii files or " */
	options_102 /* "fonts below." */,
	&text_group_sel,
	};
static Option_list streak_option =
	{
	&text_option,
	options_103 /* "STREAK" */,
	options_104 /* "Freehand, brush-size line. May be broken depending on mouse speed." */,
	NOOOM,
	};
static Option_list starf_option =
	{
	&streak_option,
	options_105 /* "STAR" */,
	options_106 /* "Create a star shape.  Define center first,  then size and angle.  " */
	options_118 /* "(Tweenable)" */,
	&om_sratio_group_sel,
	};
static Option_list spray_option =
	{
	&starf_option,
	options_108 /* "SPRAY" */,
	options_109 /* "Apply brush-size ink in random, circular pattern.  Set speed and width " */
	options_110 /* "below." */,
	&om_osped_group_sel,
	};
static Option_list curve_option =
	{
	&spray_option,
	options_111 /* "SPLINE" */,
	options_112 /* "CURVED SPLINE.  SET CURVATURE BELOW.  LOAD AND SAVE FROM POLY TOOL.  " */
	options_113 /* "(TWEENABLE)" */,
	&curve_group_sel,
	};
static Option_list spiral_option =
	{
	&curve_option,
	options_114 /* "SPIRAL" */,
	options_115 /* "Spiral-shaped line.  Set center, then angle, then turns.  (Tweenable)" */,
	NOOOM,
	};
static Option_list shapef_option =
	{
	&spiral_option,
	options_116 /* "SHAPE" */,
	options_117 /* "Drag mouse to apply freehand boundary.  Then release to close shape.  " */
	options_118 /* "(Tweenable)" */,
	&fill2c_group_sel,
	};
static Option_list sep_option =
	{
	&shapef_option,
	options_119 /* "SEP." */,
	options_120 /* "Separate colors:  replace selected color(s) with current ink." */,
	&sep_group_sel,
	};
static Option_list rpolyf_option =
	{
	&sep_option,
	options_121 /* "RPOLY" */,
	options_122 /* "REGULAR POLYGON:  ALL SIDES SAME LENGTH.  Set points below.  (Tweenable)" */,
	&om_points_group_sel,
	};
static Option_list polyf_option =
	{
	&rpolyf_option,
	options_123 /* "POLY" */,
	options_124 /* "Irregular polygon:  define one point at a time.  (Tweenable)" */,
	&freepoly_group_sel,
	};
static Option_list petlf_option =
	{
	&polyf_option,
	options_125 /* "PETAL" */,
	options_126 /* "A flower-like shape.  Set radius and points below.  (Tweenable)" */,
	&om_sratio_group_sel,
	};
static Option_list ovlf_option =
	{
	&petlf_option,
	options_127 /* "OVAL" */,
	options_128 /* "Define minor axis, then angle and major axis.  (Tweenable)" */,
	&fill2c_group_sel,
	};
static Option_list move_option =
	{
	&ovlf_option,
	options_129 /* "MOVE" */,
	options_130 /* "Move a boxed area of the screen.  Uses key color but not current ink." */,
	NOOOM,
	};
static Option_list line_option =
	{
	&move_option,
	options_131 /* "LINE" */,
	options_132 /* "Apply ink in a straight line using current brush." */,
	NOOOM,
	};
static Option_list gel_option =
	{
	&line_option,
	options_133 /* "GEL" */,
	options_134 /* "Freehand line with soft tapered edge.  Effect varies with brush size." */,
	NOOOM,
	};
static Option_list flood_option =
	{
	&gel_option,
	options_135 /* "FILLTO" */,
	options_136 /* "Click on boundary color.  Then click anywhere within boundary to fill." */,
	NOOOM,
	};
static Option_list fill_option =
	{
	&flood_option,
	options_137 /* "FILL" */,
	options_138 /* "APPLY INK TO ALL PIXELS UNTIL STOPPED BY A DIFFERENT COLOR." */,
	NOOOM,
	};
static Option_list edge_option =
	{
	&fill_option,
	options_139 /* "EDGE" */,
	options_140 /* "Click on a color.  Edges of that color will be fringed with current ink." */,
	NOOOM,
	};
static Option_list driz_option =
	{
	&edge_option,
	options_141 /* "DRIZ." */,
	options_142 /* "DRIZZLE:  LINE GETS THINNER WITH FASTER MOTION.  BEST WITH A MEDIUM TO " */
	options_143 /* "LARGE BRUSH." */,
	NOOOM,
	};
static Option_list draw_option =
	{
	&driz_option,
	options_144 /* "DRAW" */, 
	options_145 /* "HOLD DOWN LEFT BUTTON TO APPLY UNBROKEN LINE USING CURRENT BRUSH." */,
	NOOOM,
	};
static Option_list circle_option =
	{
	&draw_option,
	options_146 /* "CIRCLE" */,
	options_147 /* "MAKE A CIRCLE.  USES CURRENT BRUSH IF NOT FILLED." */,
	&fill2c_group_sel,
	};
static Option_list box_option =
	{
	&circle_option,
	options_148 /* "BOX" */,
	options_149 /* "DRAW A RECTANGLE.  USES CURRENT BRUSH IF NOT FILLED." */,
	&fill2c_group_sel,
	};

#define first_op (&box_option)
Option_list *options_list = first_op;
static Option_list *olist;

/* block for Text options */
static Flicmenu tsample_sel =
	{
	NONEXT,
	NOCHILD,
	102,152,212,44,
	NOTEXT,
	sample_text,
	NOFEEL,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
static Flicmenu tfont_sel =
	{
	&tsample_sel,
	NOCHILD,
	61,186,38,8,
	options_150 /* "FONT" */,
	ccorner_text,
	mfont_text,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
static Flicmenu tsave_sel =
	{
	&tfont_sel,
	NOCHILD,
	61,178,38,8,
	options_151 /* "SAVE" */,
	save_text_text,
	msave_text,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
static Flicmenu tload_sel =
	{
	&tsave_sel,
	NOCHILD,
	61,170,38,8,
	options_152 /* "LOAD" */,
	ccorner_text,
	mload_text,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
static Flicmenu tedit_sel =
	{
	&tload_sel,
	NOCHILD,
	61,162,38,8,
	options_153 /* "EDIT" */,
	ccorner_text,
	qedit_text,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
static Flicmenu tplace_sel =
	{
	&tedit_sel,
	NOCHILD,
	61,154,38,8,
	options_174 /* "REUSE" */,
	ccorner_text,
	qplace_text,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
static Flicmenu text_group_sel =
	{
	NONEXT,
	&tplace_sel,
	58,152,257,45,
	NOTEXT,
	NOSEE,
	NOFEEL,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
/* -----------------Block for separate options --------------*/

static struct qslider osep_rgb_sl = { 0, 100, &vs.sep_threshold, 0, };
static Flicmenu osep_rgb_sl_sel = {
	NONEXT,
	NOCHILD,
	140-3,184,174,10,
	&osep_rgb_sl,
	see_qslider,
	feel_qslider,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
static Flicmenu osep_rgb_tag_sel = {
	&osep_rgb_sl_sel,
	NOCHILD,
	140-3,174,174,8,
	options_155 /* "NEAR THRESHOLD" */,
	greytext,
	NOFEEL,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};

static Flicmenu sep_boxed_sel =
	{
	&osep_rgb_tag_sel,
	NOCHILD,
	140-3,160,56,10,
	options_156 /* "BOXED" */,
	dcorner_text,
	toggle_group,
	&vs.sep_box, 1,
	NOKEY,
	NOOPT,
	};
static Flicmenu sep_close_sel =
	{
	&sep_boxed_sel,
	NOCHILD,
	70-3,180,56,8,
	options_157 /* "NEAR" */,
	ccorner_text,
	change_mode,
	&vs.sep_type, 1,
	NOKEY,
	NOOPT,
	};
static Flicmenu sep_range_sel =
	{
	&sep_close_sel,
	NOCHILD,
	70-3,170,56,8,
	options_158 /* "CLUSTER" */,
	ccorner_text,
	change_mode,
	&vs.sep_type, 2,
	NOKEY,
	NOOPT,
	};
static Flicmenu sep_single_sel =
	{
	&sep_range_sel,
	NOCHILD,
	70-3,160,56,8,
	options_159 /* "SINGLE" */,
	ccorner_text,
	change_mode,
	&vs.sep_type, 0,
	NOKEY,
	NOOPT,
	};
static Flicmenu sep_group_sel =
	{
	NONEXT,
	&sep_single_sel,
	58,152,257,45,
	NOTEXT,
	NOSEE,
	NOFEEL,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
/* -----------------Block for Spray submenu --------------- */
static struct qslider osprd_sl = { 1, 320, &vs.air_spread, 0, };

static Flicmenu osprd_sel = {
	NONEXT,
	NOCHILD,
	140-3,184,174,10,
	&osprd_sl,
	see_qslider,
	feel_qslider,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
static Flicmenu osprdtag_sel = {
	&osprd_sel,
	NOCHILD,
	140-3,174,174,8,
	options_160 /* "SPRAY WIDTH" */,
	greytext,
	NOFEEL,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};

static struct qslider osped_sl = { 1, 100, &vs.air_speed, 0,};

static Flicmenu osped_sel = {
	&osprdtag_sel,
	NOCHILD,
	140-3,162,174,10,
	&osped_sl,
	see_qslider,
	feel_qslider,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
static Flicmenu ospedtag_sel = {
	&osped_sel,
	NOCHILD,
	140-3,152,174,8,
	options_161 /* "AIR SPEED" */,
	greytext,
	NOFEEL,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
static Flicmenu om_osped_group_sel =
	{
	NONEXT,
	&ospedtag_sel,
	58,152,257,45,
	NOTEXT,
	NOSEE,
	NOFEEL,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};

/* ----------- block for curve options */
static struct qslider otens_sl = { -20, 20, &vs.sp_tens, 0};
static struct qslider ocont_sl = { -20, 20, &vs.sp_cont, 0};
static struct qslider obias_sl = { -20, 20, &vs.sp_bias, 0};

static Flicmenu bias_sl_sel =
	{
	NONEXT,
	NOCHILD,
	151,183,158,10,
	&obias_sl,
	see_qslider,
	feel_qslider,
	NOGROUP, 0,
	NOKEY,
	zero_sl,
	};
static Flicmenu cont_sl_sel =
	{
	&bias_sl_sel,
	NOCHILD,
	151,170,158,10,
	&ocont_sl,
	see_qslider,
	feel_qslider,
	NOGROUP, 0,
	NOKEY,
	zero_sl,
	};
static Flicmenu tens_sl_sel =
	{
	&cont_sl_sel,
	NOCHILD,
	151,157,158,10,
	&otens_sl,
	see_qslider,
	feel_qslider,
	NOGROUP, 0,
	NOKEY,
	zero_sl,
	};
static Flicmenu biastag_sel =
	{
	&tens_sl_sel,
	NOCHILD,
	122,185,24,6,
	options_162 /* "BIAS" */,
	blacktext,
	zero_slider,
	(WORD *)&bias_sl_sel, -1000,
	NOKEY,
	NOOPT,
	};
static Flicmenu conttag_sel =
	{
	&biastag_sel,
	NOCHILD,
	122,172,24,6,
	options_163 /* "CONT" */,
	blacktext,
	zero_slider,
	(WORD *)&cont_sl_sel, -1000,
	NOKEY,
	NOOPT,
	};
static Flicmenu tenstag_sel =
	{
	&conttag_sel,
	NOCHILD,
	122,159,24,6,
	options_164 /* "TENS" */,
	blacktext,
	zero_slider,
	(WORD *)&tens_sl_sel, -1000,
	NOKEY,
	NOOPT,
	};
static Flicmenu c2curve_sel =
	{
	&tenstag_sel,
	NOCHILD,
	61,162,56,8,
	options_181 /* "2 COLOR" */,
	ccorner_text,
	toggle_group,
	&vs.color2, 1,
	NOKEY,
	NOOPT,
	};
static Flicmenu fcurve_sel =
	{
	&c2curve_sel,
	NOCHILD,
	61,154,56,8,
	options_182 /* "FILLED" */,
	ccorner_text,
	toggle_group,
	&vs.fillp, 1,
	NOKEY,
	NOOPT,
	};
static Flicmenu ccurve_sel =
	{
	&fcurve_sel,
	NOCHILD,
	61,170,56,8,
	options_175 /* "CLOSED" */,
	ccorner_text,
	toggle_group,
	&vs.closed_curve, 1,
	NOKEY,
	NOOPT,
	};
static Flicmenu ecurve_sel =
	{
	&ccurve_sel,
	NOCHILD,
	61,178,56,8,
	options_174 /* "REUSE" */,
	ccorner_poly_text,
	edit_curve,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
static Flicmenu tcurve_sel =
	{
	&ecurve_sel,
	NOCHILD,
	61,186,56,8,
	options_173 /* "TWEEN" */,
	ccorner_poly_text,
	tween_curve,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
static Flicmenu curve_group_sel =
	{
	NONEXT,
	&tcurve_sel,
	58,152,257,45,
	NOTEXT,
	NOSEE,
	NOFEEL,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};

/* -----------------Block for filled/2color --------------------*/
static Flicmenu color2_sel =
	{
	NONEXT,
	NOCHILD,
	70-3,180,56,8,
	options_181 /* "2 COLOR" */,
	ccorner_text,
	toggle_group,
	&vs.color2, 1,
	NOKEY,
	NOOPT,
	};
static Flicmenu filledp_sel =
	{
	&color2_sel,
	NOCHILD,
	70-3,169,56,8,
	options_182 /* "FILLED" */,
	ccorner_text,
	toggle_group,
	&vs.fillp, 1,
	NOKEY,
	NOOPT,
	};
static Flicmenu fill2c_group_sel =
	{
	NONEXT,
	&filledp_sel,
	58,152,257,45,
	NOTEXT,
	NOSEE,
	NOFEEL,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};


/* -----------------Block for freehand polygons --------------------*/
static Flicmenu fpg_files_sel =
	{
	NONEXT,
	NOCHILD,
	132,182,56,8,
	options_172 /* "FILES" */,
	ccorner_text,
	go_poly_files,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
static Flicmenu fpg_tween_sel =
	{
	&fpg_files_sel,
	NOCHILD,
	132,171,56,8,
	options_173 /* "TWEEN" */,
	ccorner_poly_text,
	tween_poly,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
static Flicmenu fpg_o1mpg_editp_sel =
	{
	&fpg_tween_sel,
	NOCHILD,
	132,160,56,8,
	options_174 /* "REUSE" */,
	ccorner_poly_text,
	edit_poly,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
static Flicmenu fpg_closed_sel =
	{
	&fpg_o1mpg_editp_sel,
	NOCHILD,
	67,182,56,8,
	options_175 /* "CLOSED" */,
	ccorner_text,
	toggle_group,
	&vs.closed_curve, 1,
	NOKEY,
	NOOPT,
	};
static Flicmenu fpg_color2_sel =
	{
	&fpg_closed_sel,
	NOCHILD,
	67,171,56,8,
	options_181 /* "2 COLOR" */,
	ccorner_text,
	toggle_group,
	&vs.color2, 1,
	NOKEY,
	NOOPT,
	};
static Flicmenu fpg_filledp_sel =
	{
	&fpg_color2_sel,
	NOCHILD,
	67,160,56,8,
	options_182 /* "FILLED" */,
	ccorner_text,
	toggle_group,
	&vs.fillp, 1,
	NOKEY,
	NOOPT,
	};
static Flicmenu freepoly_group_sel =
	{
	NONEXT,
	&fpg_filledp_sel,
	58,152,257,45,
	NOTEXT,
	NOSEE,
	NOFEEL,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};

/* -----------------Block for Star Points slider --------------- */
static struct qslider ompg_sl = { 3, 32, &vs.star_points, 0};

static Flicmenu ompg_color2_sel =
	{
	NONEXT,
	NOCHILD,
	70-3,180,56,8,
	options_181 /* "2 COLOR" */,
	ccorner_text,
	toggle_group,
	&vs.color2, 1,
	NOKEY,
	NOOPT,
	};
static Flicmenu ompg_filledp_sel =
	{
	&ompg_color2_sel,
	NOCHILD,
	70-3,169,56,8,
	options_182 /* "FILLED" */,
	ccorner_text,
	toggle_group,
	&vs.fillp, 1,
	NOKEY,
	NOOPT,
	};

static Flicmenu ompgparent_sel = {
	&ompg_filledp_sel,
	NOCHILD,
	140-3,184,174,10,
	&ompg_sl,
	see_qslider,
	feel_qslider,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
static Flicmenu ompgtag_sel = {
	&ompgparent_sel,
	NOCHILD,
	140-3,174,174,8,
	options_183 /* "POINTS:" */,
	greytext,
	NOFEEL,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
static Flicmenu om_points_group_sel =
	{
	NONEXT,
	&ompgtag_sel,
	58,152,257,45,
	NOTEXT,
	NOSEE,
	NOFEEL,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};

/* -----------------Block for Star Ratio slider --------------- */
static Flicmenu o1mpg_color2_sel =
	{
	NONEXT,
	NOCHILD,
	70-3,180,56,8,
	options_181 /* "2 COLOR" */,
	ccorner_text,
	toggle_group,
	&vs.color2, 1,
	NOKEY,
	NOOPT,
	};
static Flicmenu o1mpg_filledp_sel =
	{
	&o1mpg_color2_sel,
	NOCHILD,
	70-3,169,56,8,
	options_182 /* "FILLED" */,
	ccorner_text,
	toggle_group,
	&vs.fillp, 1,
	NOKEY,
	NOOPT,
	};

static Flicmenu o1mpgparent_sel = {
	&o1mpg_filledp_sel,
	NOCHILD,
	140-3,184,174,10,
	&ompg_sl,
	see_qslider,
	feel_qslider,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
static Flicmenu o1mpgtag_sel = {
	&o1mpgparent_sel,
	NOCHILD,
	140-3,174,174,8,
	options_183 /* "POINTS:" */,
	greytext,
	NOFEEL,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
static struct qslider osrt_sl = { 0, 100, &vs.star_ratio,0,};
static Flicmenu osrtparent_sel = {
	&o1mpgtag_sel,
	NOCHILD,
	140-3,162,174,10,
	(char *)&osrt_sl,
	see_qslider,
	feel_qslider,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
static Flicmenu osrttag_sel = {
	&osrtparent_sel,
	NOCHILD,
	140-3,152,174,8,
	options_184 /* "INNER RADIUS RATIO" */,
	greytext,
	NOFEEL,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
static Flicmenu om_sratio_group_sel =
	{
	NONEXT,
	&osrttag_sel,
	58,152,257,45,
	NOTEXT,
	NOSEE,
	NOFEEL,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};


/* --------- start of stuff always in this menu -----------------*/
static Flicmenu om_hanger_sel =
	{
	NONEXT,
	NOCHILD,
	58,152,257,45,
	NOTEXT,
	hang_option,
	NOFEEL,
	&vs.tool_ix, 0,
	NOKEY,
	NOOPT,
	};
static Flicmenu opt_help_sel =
	{
	&om_hanger_sel,
	NOCHILD,
	140, 115, 175, 34,
	NOTEXT,
	opt_help,
	NOFEEL,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
static Flicmenu opt_hanger_sel =
	{
	&opt_help_sel,
	&dsel_group_sel,
	58, 118, 78, 28,
	NOTEXT,
	hang_child,
	NOFEEL,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
static Name_scroller oscroller;
static Flicmenu olist_sel =
	{
	&opt_hanger_sel,
	NOCHILD,
	16, 115, 38, 82,
	(char *)&oscroller,
	print_list,
	pick_option,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
static Flicmenu odown_sel =
	{
	&olist_sel,
	NOCHILD,
	2, 187, 12, 10,
	(char *)&ctridown,
	ccorner_cursor,
	fincdown,
	(WORD *)&oscroller, 0,
	NOKEY,
	NOOPT,
	};

static Flicmenu oscroll_sel =
	{
	&odown_sel,
	NOCHILD,
	3, 127, 10, 58,
	(char *)&oscroller,
	see_scroll,
	ffeelscroll,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};

static Flicmenu oup_sel =
	{
	&oscroll_sel,
	NOCHILD,
	2, 115, 12, 10,
	(char *)&ctriup,
	ccorner_cursor,
	fincup,
	(WORD *)&oscroller, 0,
	NOKEY,
	NOOPT,
	};

static Flicmenu otitle_sel =
	{
	&oup_sel,
	NOCHILD,
	0, 105, 319, 7,
	NOTEXT,
	gary_see_title,
	move_menu,
	NOGROUP, 0,
	NOKEY,
	bottom_menu,
	};


static Flicmenu options_menu =
	{
	NONEXT,
	&otitle_sel,
	0, 105,	319, 94,
	NULL,
	gary_menu_back,
	NOFEEL,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};

save_text_text(m)
Flicmenu *m;
{
m->disabled = !jexists(text_name);
ccorner_text(m);
}

ccorner_poly_text(m)
Flicmenu *m;
{
m->disabled = !jexists(poly_name);
ccorner_text(m);
}

zero_sl(m)
Flicmenu *m;
{
struct qslider *s;

s = m->text;
*(s->value) = 0;
draw_sel(m);
}

static
zero_slider(m)
Flicmenu *m;
{
m = (Flicmenu *)m->group;
zero_sl(m);
}


static
refresh_options()
{
qdraw_a_menu(&opt_hanger_sel);
qdraw_a_menu(&om_hanger_sel);
draw_sel(&opt_help_sel);
}

static
sample_text(m)
Flicmenu *m;
{
int w,h;
char *s;

s = vs.fonts[0];
w = fstring_width(usr_font, s);
h = usr_font->frm_hgt;
if (w < m->width && h < m->height)
	gftext(&vf, usr_font, s, m->x + (m->width - w)/2, 
		m->y + (m->height - h)/2, sgrey, a1blit, 0);
else
	{
	m->text = s;
	greytext(m);
	}

}



#ifdef SLUFFED
static
cliply_move_menu(m, hiy, x, y)
Flicmenu *m;
int hiy;
int x, y;
{
if (y < hiy)
	y=hiy;
if (y + m->height >= vf.h)
	y = vf.h - m->height - 1;
rmove_menu(m, 0, y-m->y); 
}
#endif SLUFFED



static
pick_option(m)
Flicmenu *m;
{
Option_list *o;
int newmode;

if ((o = which_sel(m) ) != NULL)
	{
	vs.tool_ix = el_ix(olist, o);
	if (in_ink_menu)
		{
		newmode = dmd_lookup[vs.tool_ix];
		new_percent(newmode);
		vs.dm_slots[el_ix(&it0_sel, optm)] =  
			optm->identity = vs.draw_mode = newmode;
		}
	else
		{
		vs.tool_slots[el_ix(&dsel1_sel, optm)] = 
			optm->identity = vs.tool_ix;
		}
	optm->text = o->name;
	}
refresh_options();
}

static
opt_help(m)
Flicmenu *m;
{
Option_list *o;

o = list_el(olist, vs.tool_ix);
gary_menu_back(m);
to_upper(o->help);
wwtext(&vf, &sixhi_font,
	o->help, m->x+2, m->y+2, m->width-3, m->height-3, sgrey,
	a1blit, 0, 0);
}

new_percent(newmode)
int newmode;
{
vs.inkstrengths[vs.draw_mode] = vs.tint_percent + 
	(vs.dither ? 0x80 : 0);
vs.tint_percent = vs.inkstrengths[newmode];
if (vs.tint_percent & 0x80)
	{
	vs.dither = 1;
	vs.tint_percent &= 0x7f;
	}
else
	vs.dither = 0;
}

change_ink_mode(m)
Flicmenu *m;
{
new_percent(m->identity);
change_mode(m);
if (cur_menu == &options_menu)
	{
	vs.tool_ix = idmd_lookup[vs.draw_mode];
	optm = m;
	refresh_options();
	}
}

change_pen_mode(m)
Flicmenu *m;
{
change_mode(m);
if (cur_menu == &options_menu)
	{
	optm = m;
	refresh_options();
	}
}

static
hang_option(m)
Flicmenu *m;
{
Option_list *o;

gary_menu_back(m);
o = list_el(m->text,*m->group);
m->children = o->options;
hang_child(m);
}

static
redraw_oscroller()
{
redraw_scroller(&oscroll_sel, &olist_sel);
}

mload_text()
{
hide_mp();
qload_text();
draw_mp();
}

static
mfont_text()
{
hide_mp();
vs.oscroller_top = oscroller.top_name;
qfont_text();
iopt_scroller();
draw_mp();
}


qload_text()
{
char *title;

vs.oscroller_top = oscroller.top_name;
unzoom();
if ((title =  get_filename(options_185 /* "Load Text?" */, ".TXT"))!=NULL)
	{
	if (!jexists(title))
		cant_find(title);
	else
		{
		jcopyfile(title, text_name);
		vs.tcursor_p = vs.tcursor_y = vs.text_yoff = 0;
		}
	}
rezoom();
iopt_scroller();
}

static
msave_text()
{
hide_mp();
qsave_text();
draw_mp();
}

qsave_text()
{
char *title;

vs.oscroller_top = oscroller.top_name;
if (jexists(text_name) )
	{
	unzoom();
	if ((title =  get_filename(options_187 /* "Save Text?" */, ".TXT"))!=NULL)
		{
		if (overwrite_old(title))
			jcopyfile(text_name, title);
		}
	rezoom();
	}
iopt_scroller();
}

static
go_poly_files()
{
hide_mp();
go_files(6);
iopt_scroller();
draw_mp();
}


/* iopt_scroller() - since reusing some of data structures in file
   requestor to implement scroller, must call this routine if
   are part of a button that could directly or indirectly get you
   to files menu.  (Should redesign the )*(& thing, this is fragile
   code.)  */
static
iopt_scroller()
{
oscroller.top_name = vs.oscroller_top;
iscroller(&oscroller,(Name_list *)olist,&oscroll_sel,&olist_sel,
	scroll_ycount(&olist_sel),redraw_oscroller);
}


static
opt_menu(m)
Flicmenu *m;
{
om_hanger_sel.text = olist;
optm = m;
unzoom();
clip_rmove_menu(&options_menu, 
	cur_menu->x - options_menu.x, cur_menu->y-options_menu.y); 
iopt_scroller();
do_menu(&options_menu);
vs.oscroller_top = oscroller.top_name;
rezoom();
}

static Flicmenu *
match_id(m, id)
Flicmenu *m;
{
for (;;)
	{
	if (m->identity == id)
		return(m);
	m = m->next;
	}
}

static
go_tools(m)
Flicmenu *m;
{
grab_usr_font();
opt_hanger_sel.children = &dsel_group_sel;
otitle_sel.text = options_189 /* "DRAWING TOOLS" */;
olist = first_op;
opt_menu(m);
free_cfont();
}

options(m)
Flicmenu *m;
{
if (cur_menu == &options_menu)	/* avoid recursing */
	return;
change_pen_mode(m);
hide_mp();
go_tools(m);
draw_mp();
}

extern Option_list add_option;

static
go_inks(m)
Flicmenu *m;
{
int otool_ix;

otitle_sel.text = options_190 /* "INK TYPES" */;
in_ink_menu = 1;
opt_hanger_sel.children = &itgroup_sel;
otool_ix = vs.tool_ix;	/* screwy way to reuse tool menu code for ink data */
vs.tool_ix = idmd_lookup[vs.draw_mode];
olist = &add_option;
opt_menu(m);
vs.draw_mode = dmd_lookup[vs.tool_ix];
vs.tool_ix = otool_ix;
in_ink_menu = 0;
}

go_dmmenu(m)
Flicmenu *m;
{
if (cur_menu == &options_menu)	/* avoid recursing */
	return;
change_ink_mode(m);
hide_mp();
go_inks(m);
draw_mp();
}

qtools()
{
go_tools(match_id(&dsel1_sel, vs.tool_ix) );
}


qinks()
{
go_inks(match_id(&it0_sel, vs.draw_mode) );
}

minks()
{
hide_mp();
qinks();
draw_mp();
}

