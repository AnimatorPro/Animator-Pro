
/* Drawmode.c This file contains the data structures and routines for the
   ink types menu.  The ink types menu 'parasites' off of the draw tools menu
   in options.c.   That is I wrote options.c first and then hacked it
   about so most of the code in the draw tools and ink types menus could
   be shared.  */


#include "jimk.h"
#include "flicmenu.h"
#include "gemfont.h"
#include "inks.h"
#include "drawmode.str"

extern Flicmenu dtintgroup_sel,tintgroup_sel, radgroup_sel, dithergroup_sel;
extern UBYTE idmd_lookup[];
extern UBYTE dmd_lookup[];

extern inverse_cursor(), blacktext(), black_block(), see_islidepot(),
	see_qslider(), feel_qslider(), ccorner_text(), close_menu(),
	mrewind(), mfast_forward(),wcursor(), 
	undo_pic(), move_menu(), fill_inkwell(),
	show_sel_mode(), toggle_sel_mode(), see_menu_back(),
	see_range(), see_colors2(), see_ink(),
	mget_color(), ccolor_box(), change_mode(),
	playit(), prev_frame(),next_frame(), first_frame(), last_frame(),
	text_lineunder(), see_pen(), toggle_pen(), set_pspeed(),
	text_boxp1(), toggle_group(),
	see_draw_mode(), toggle_draw_mode(), toggle_zoom(),
	bottom_menu(), palette(), options(),text_boxp1(),
	white_block(), hang_child(), wbtext(), hang_ink_option(),
	tninc_slider(), tndec_slider(), 
	see_number_slider(), right_arrow(), left_arrow(),
	bwtext(), wbtexty1(), wbnumber(), bwnumber(), white_slice();

extern struct cursor cdown, cleft, cright, csleft,
	cinsert, cappend, cright2, cleft2, csright, cup, size_cursor,
	default_c;

extern wtext(), fincup(), fincdown(), ffeelscroll(), see_scroll();
extern struct cursor ctriup, ctridown;
extern int see_colors2(), see_menu_back(),print_list(),cursor_box(),bwtext(),
	wbtext();

extern pick_1dm(), pick_dm(), dm_help(), set_rad_center();

extern WORD x_0, y_0, x_1, y_1;


static Option_list xor_dmd =
	{
	NONEXT,
	drawmode_100 /* "Xor" */,
	drawmode_101 /* "Exclusive or the current color register with image." */,
	NOOOM,
	};
static Option_list vsp_dmd =
	{
	&xor_dmd,
	drawmode_102 /* "V Grad" */,
	drawmode_103 /* "Vertical Gradient: apply cluster colors from top to bottom." */,
	&dithergroup_sel,
	};
static Option_list anti_dmd =
	{
	&vsp_dmd,
	drawmode_104 /* "Unzag" */,
	drawmode_105 /* "Antialias (soften) staircase edges.  Doesn't affect vertical or " */
	drawmode_106 /* "horizontal edges." */,
	&dithergroup_sel,
	};
static Option_list celt_dmd =
	{
	&anti_dmd,
	drawmode_107 /* "Tile" */,
	drawmode_108 /* "Use the cel as a tile pattern." */,
	&dithergroup_sel,
	};
static Option_list swe_dmd =
	{
	&celt_dmd,
	drawmode_109 /* "Sweep" */,
	drawmode_110 /* "Remove isolated pixels." */,
	NOOOM,
	};
static Option_list shat_dmd =
	{
	&swe_dmd,
	drawmode_111 /* "Split" */,
	drawmode_112 /* "Slide every other scan line in opposite direction.  Ink strength " */
	drawmode_113 /* "is pixel offset." */,
	&tintgroup_sel,
	};
static Option_list cry_dmd =
	{
	&shat_dmd,
	drawmode_114 /* "Spark" */,
	drawmode_115 /* "Sparkle: change pixel to the sum of its four neighbors modulo 256." */,
	NOOOM,
	};
static Option_list soft_dmd =
	{
	&cry_dmd,
	drawmode_116 /* "Soften" */,
	drawmode_117 /* "Blur the image.  Repeat for increased effect." */,
	&dithergroup_sel,
	};
static Option_list smea_dmd =
	{
	&soft_dmd,
	drawmode_118 /* "Smear" */,
	drawmode_119 /* "Drag colors gently in direction of mouse motion.  (See pull for a " */
	drawmode_120 /* "strong smear.)" */,
	NOOOM,
	};
static Option_list rvl_dmd =
	{
	&smea_dmd,
	drawmode_121 /* "Scrape" */,
	drawmode_122 /* "Scrape through to reveal Swap Screen." */,
	&dithergroup_sel,
	};
static Option_list rad_dmd =
	{
	&rvl_dmd,
	drawmode_123 /* "R Grad" */,
	drawmode_124 /* "Radial gradiant: apply cluster ink with focus established by " */
	drawmode_125 /* "center below." */,
	&radgroup_sel,
	};
static Option_list pull_dmd =
	{
	&rad_dmd,
	drawmode_126 /* "Pull" */,
	drawmode_127 /* "Drag colors strongly in direction of mouse motion.  (See smear for a " */
	drawmode_128 /* "gentle smear.)" */,
	NOOOM,
	};
static Option_list opq_dmd =
	{
	&pull_dmd,
	drawmode_129 /* "Opaque" */,
	drawmode_130 /* "Use current color with no see-through." */,
	NOOOM,
	};
static Option_list lsp_dmd =
	{
	&opq_dmd,
	drawmode_131 /* "L Grad" */,
	drawmode_132 /* "Line-contour gradient:  Apply cluster colors horizontally contoured to " */
	drawmode_133 /* "edge." */,
	&dithergroup_sel,
	};
static Option_list jmb_dmd =
	{
	&lsp_dmd,
	drawmode_134 /* "Jumble" */,
	drawmode_135 /* "Mix pixels randomly. Best results at low ink strength." */,
	&tintgroup_sel,
	};
static Option_list out_dmd =
	{
	&jmb_dmd,
	drawmode_136 /* "Hollow" */,
	drawmode_137 /* "Reduce solid shapes to outlines.  Reverse of fill." */,
	NOOOM,
	};
static Option_list hsp_dmd =
	{
	&out_dmd,
	drawmode_138 /* "H Grad" */,
	drawmode_139 /* "Horizontal gradient: apply cluster colors from left to right." */,
	&dithergroup_sel,
	};
static Option_list des_dmd = 
	{
	&hsp_dmd,
	drawmode_140 /* "Gray" */,
	drawmode_141 /* "Desaturate. Remove chroma. Push towards gray." */,
	&dtintgroup_sel,
	};
static Option_list glr_dmd =
	{
	&des_dmd,
	drawmode_142 /* "Glow" */,
	drawmode_143 /* "Glow cluster: shift cluster colors one step." */,
	NOOOM,
	};
static Option_list tlc_dmd =
	{
	&glr_dmd,
	drawmode_144 /* "Glaze" */,
	drawmode_145 /* "Make translucent layers of color while mouse button is down. " */
	drawmode_146 /* "Works well with spray." */,
	&dtintgroup_sel,
	};
static Option_list tsp_dmd =
	{
	&tlc_dmd,
	drawmode_147 /* "Glass" */,
	drawmode_148 /* "Make a single transparent layer. Release mouse button or start a new " */
	drawmode_149 /* "shape for a new layer." */,
	&dtintgroup_sel,
	};
static Option_list emb_dmd =
	{
	&tsp_dmd,
	drawmode_150 /* "Emboss" */,
	drawmode_151 /* "Bas-relief look.  Highlight upper-left set of edges.  Shadow the " */
	drawmode_152 /* "opposite set." */,
	&dtintgroup_sel,
	};
static Option_list dar_dmd =
	{
	&emb_dmd,
	drawmode_153 /* "Dark" */,
	drawmode_154 /* "Make colors darker.  Push towards black." */,
	&dtintgroup_sel,
	};
static Option_list clh_dmd =
	{
	&dar_dmd,
	drawmode_155 /* "Close" */,
	drawmode_156 /* "Close single pixel holes in lines of current color." */,
	NOOOM,
	};
static Option_list bri_dmd =
	{
	&clh_dmd,
	drawmode_157 /* "Bright" */,
	drawmode_158 /* "Brighten:  increase apparent lighting." */,
	&dtintgroup_sel,
	};
Option_list add_option =
	{
	&bri_dmd,
	drawmode_159 /* "Add" */,
	drawmode_160 /* "Add register number of current color to image modulo 256." */,
	NOOOM,
	};

#define first_option (&add_option)
Option_list *dm_list = first_option;


static struct qslider tint_sl = {0,100,&vs.tint_percent, 0, NULL};

static Flicmenu tinting_sel = {
	NONEXT,
	NOCHILD,
	114, 182, 152, 13,
	&tint_sl,
	see_qslider,
	feel_qslider,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
static Flicmenu tinttag_sel = {
	&tinting_sel,
	NOCHILD,
	114,170,152,11,
	drawmode_162 /* "Ink Strength" */,
	blacktext,
	NOFEEL,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
static Flicmenu tintgroup_sel =
	{
	NONEXT,
	&tinttag_sel,
	58,152,257,45,
	NOTEXT,
	NOSEE,
	NOFEEL,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
/* -------------------------- */	/* for dithered ink strengths */
static Flicmenu dtinting_sel = {
	NONEXT,
	NOCHILD,
	114, 182, 152, 13,
	&tint_sl,
	see_qslider,
	feel_qslider,
	NOGROUP, 0,
	NOKEY,
	feel_qslider,
	};
static Flicmenu dtinttag_sel = {
	&dtinting_sel,
	NOCHILD,
	114,170,152,11,
	drawmode_162 /* "Ink Strength" */,
	blacktext,
	NOFEEL,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
static Flicmenu dtint_sel = {
	&dtinttag_sel,
	NOCHILD,
	161,158,60,11,
	drawmode_165 /* "Dither" */,
	ccorner_text,
	toggle_group,
	&vs.dither, 1,
	NOKEY,
	NOOPT,
	};
static Flicmenu dtintgroup_sel =
	{
	NONEXT,
	&dtint_sel,
	58,152,257,45,
	NOTEXT,
	NOSEE,
	NOFEEL,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
/* ------------ */
static Flicmenu dither_sel =
	{
	NONEXT,
	NOCHILD,
	161, 180, 60, 12,
	drawmode_165 /* "Dither" */,
	ccorner_text,
	toggle_group,
	&vs.dither, 1,
	NOKEY,
	NOOPT,
	};
static Flicmenu dithergroup_sel =
	{
	NONEXT,
	&dither_sel,
	58,152,257,45,
	NOTEXT,
	NOSEE,
	NOFEEL,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
/* -------- */
static Flicmenu rdither_sel =
	{
	NONEXT,
	NOCHILD,
	161, 180, 60, 12,
	drawmode_165 /* "Dither" */,
	ccorner_text,
	toggle_group,
	&vs.dither, 1,
	NOKEY,
	NOOPT,
	};
static Flicmenu setrad_sel =
	{
	&rdither_sel,
	NOCHILD,
	161, 160, 60, 12,
	drawmode_166 /* "Center" */,
	ccorner_text,
	set_rad_center,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
static Flicmenu radgroup_sel =
	{
	NONEXT,
	&setrad_sel,
	58,152,257,45,
	NOTEXT,
	NOSEE,
	NOFEEL,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};


static set_rad_center()
{
hide_mp();
save_undo();
if (cut_cursor())
	{
	if (rub_circle())
		{
		vs.rgr = center_rad();
		if (vs.rgr <= 0)
			vs.rgr = 1;
		vs.rgx = x_0;
		vs.rgy = y_0;
		}
	}
draw_mp();
}


char *
ink_word()
{
Option_list *ink_opt;

ink_opt = list_el((Name_list *)dm_list, idmd_lookup[vs.draw_mode]);
return(ink_opt->name);
}

force_opaque(m)
Flicmenu *m;
{
new_percent(I_OPAQUE);
vs.draw_mode = I_OPAQUE;
draw_sel(m);
}

see_cur_ink(m)
Flicmenu *m;
{
if (vs.draw_mode == I_OPAQUE)
	m->identity = -1;
else
	m->identity = vs.draw_mode;
m->text = ink_word();
dcorner_text(m);
}

