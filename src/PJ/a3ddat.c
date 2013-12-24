
/* a3ddat.c - This file contains the data structures for the optics panel.
   Also some functions for enabling/disabling buttons and rearranging
   the optics panel for example to bring up the reduce/enlarge sliders
   when you press the size button */

/* Gak this is next to completely gnarly.  There's I'm not sure,
   3 or 4 levels to the menu tree.  The main branches are
   			1. Mouse control area on left.
				Size specific controls (x y xy proportional)
				Everyone else's controls (x y z yz zx xy)

			2. The 'major' mode strip - whether it's turn/size/move/path
     			Just 4 radio buttons in the center.

			3. The subpanel area - which changes depending which
			   major mode you are in.

   The subpanel area is where it gets really complicated:

   In the case of 'turn' and 'size' the left part of the subpanel
   contains radio buttons which select a subsubpanel to the right.
   What I did to minimize as best I could the number of buttons
   serving only as hierarchical place holders is too complex for
   me to remember any more.  Lets hope no-one wants us to add
   'just one more feature' that would require re-laying this out.

   In the case of 'move'  you go straight to an x/y/z slider set 
   covering the entire subpanel.

   The path sub panel also isn't too bad.  At least the menus
   aren't alway spontaniously re-arranging themselves.

   Oh well.  Someday I'll figure out a simpler way to do this,
   or maybe just give up doing these paint-box like interfaces
   when they go beyond simple columns of text.
   */


#include "jimk.h"
#include "menus.h"
#include "broadcas.h"
#include "fli.h"
#include "a3d.h"
#include "inks.h"
#include "softmenu.h"

extern Button tseg_group_sel;
extern void grey_ctext();
extern void go_nodraw_cel_menu();

void qinks(), go_multi(), arrange_a3d_menu(), go_cel_menu(),
	ado_xyz_slider(), xyz_zero_sl(), iscale_theta(), mview_path(),
	edit_path(), move_along(), mauto_ado(), mado_loop(), mado_view(), 
	ccolor_box(),
	toggle_pen(), qmask(), zero_sl(), ppalette(), see_pen(),
	set_pbrush();
void go_color_grid(Button *b);
static void a3d_go_color_grid(Button *b);
static void clear_pos();


static void feel_rdc_qslider();
static void change_rot_scale();
static void set_axis();
static void csame_spin();
static void csize_default();
static void csame_size();
static void cspin_default();
static void see_size_ratio();
static void change_size_mode();
static void change_spin_mode();
static void mgo_path_files();
static void change_ado_mode();
static void clear_track();

Short_xyz rot_theta;	/* where the xyz sliders usually point... */

/* The usual x/y/z optics sliders */
static Qslider a3d_xslider = 
	QSL_INIT1( -500, 500, NULL, 0, NULL, leftright_arrs);
static Qslider a3d_yslider = 
	QSL_INIT1( -500, 500, NULL, 0, NULL, updown_arrs);
static Qslider a3d_zslider = 
	QSL_INIT1( -500, 500, NULL, 0, NULL, zoutin_arrs);

/* reduce/enlarge sliders */
static Qslider a3d_rdc_slider = 
	QSL_INIT1(	0, 100, NULL, 0, NULL, leftright_arrs);
static Qslider a3d_enl_slider = 
	QSL_INIT1(  1, 100, NULL, 0, NULL, leftright_arrs);

#define MCOFF (-9)


/* Branch of tree that covers mouse control buttons */

/* Mouse control section for size */
		static Button a3d_y_szm = MB_INIT1(
			NONEXT,
			NOCHILD,
			20,11,MCOFF+278,59,
			"Y",
			dcorner_text,
			change_mode,
			NOOPT,
			&vs.ado_szmouse,3,
			NOKEY,
			MB_B_GHILITE
			);
		static Button a3d_x_szm = MB_INIT1(
			&a3d_y_szm,
			NOCHILD,
			20,11,MCOFF+255,59,
			"X",
			dcorner_text,
			change_mode,
			NOOPT,
			&vs.ado_szmouse,2,
			NOKEY,
			MB_B_GHILITE
			);
		static Button a3d_xy_szm = MB_INIT1(
			&a3d_x_szm,
			NOCHILD,
			20,11,MCOFF+232,59,
			"XY",
			dcorner_text,
			change_mode,
			NOOPT,
			&vs.ado_szmouse,1,
			NOKEY,
			MB_B_GHILITE
			);
		static Button a3d_prop_sel = MB_INIT1(
			&a3d_xy_szm,
			NOCHILD,
			77,11,MCOFF+227,46,
			NODATA, /* "Proportional", */
			dcorner_text,
			change_mode,
			NOOPT,
			&vs.ado_szmouse,0,
			NOKEY,
			MB_B_GHILITE
			);
		static Button a3d_szcleart_sel = MB_INIT1(
			&a3d_prop_sel,
			NOCHILD,
			72,9,MCOFF+229,35,
			NODATA, /* "Mouse control", */
			grey_ctext,
			NOFEEL,
			NOOPT,
			NOGROUP,0,
			NOKEY,
			0
			);
	static Button a3d_szm_group = MB_INIT1(
		NONEXT,
		&a3d_szcleart_sel,
		85,41,214,32,
		NOTEXT,
		wbg_ncorner_back,
		NOFEEL,
		NOOPT,
		NOGROUP,0,
		NOKEY,
		0
		);

/* mouse control section for all other states */
		static Button a3d_x_spm = MB_INIT1(
			NONEXT,
			NOCHILD,
			20,11,MCOFF+277,59,
			"X",
			dcorner_text,
			change_mode,
			NOOPT,
			&vs.ado_mouse,5,
			NOKEY,
			MB_B_GHILITE
			);
		static Button a3d_y_spm = MB_INIT1(
			&a3d_x_spm,
			NOCHILD,
			20,11,MCOFF+255,59,
			"Y",
			dcorner_text,
			change_mode,
			NOOPT,
			&vs.ado_mouse,4,
			NOKEY,
			MB_B_GHILITE
			);
		static Button a3d_z_spm = MB_INIT1(
			&a3d_y_spm,
			NOCHILD,
			20,11,MCOFF+233,59,
			"Z",
			dcorner_text,
			change_mode,
			NOOPT,
			&vs.ado_mouse,3,
			NOKEY,
			MB_B_GHILITE
			);
		static Button a3d_yz_spm = MB_INIT1(
			&a3d_z_spm,
			NOCHILD,
			20,11,MCOFF+277,47,
			"ZY",
			dcorner_text,
			change_mode,
			NOOPT,
			&vs.ado_mouse,2,
			NOKEY,
			MB_B_GHILITE
			);
		static Button a3d_zx_spm = MB_INIT1(
			&a3d_yz_spm,
			NOCHILD,
			20,11,MCOFF+255,47,
			"XZ",
			dcorner_text,
			change_mode,
			NOOPT,
			&vs.ado_mouse,1,
			NOKEY,
			MB_B_GHILITE
			);
		static Button a3d_xy_spm = MB_INIT1(
			&a3d_zx_spm,
			NOCHILD,
			20,11,MCOFF+233,47,
			"XY",
			dcorner_text,
			change_mode,
			NOOPT,
			&vs.ado_mouse,0,
			NOKEY,
			MB_B_GHILITE
			);
		static Button a3d_cleart_sel = MB_INIT1(
			&a3d_xy_spm,
			NOCHILD,
			72,9,MCOFF+229,35,
			NODATA, /* "Mouse control", */
			grey_ctext,
			NOFEEL,
			NOOPT,
			NOGROUP,0,
			NOKEY,
			0
			);
	static Button a3d_spm_group = MB_INIT1(
		NONEXT,
		&a3d_cleart_sel,
		85,41,214,32,
		NOTEXT,
		wbg_ncorner_back,
		NOFEEL,
		NOOPT,
		NOGROUP,0,
		NOKEY,
		0
		);
static Button a3d_g4_sel = MB_INIT1(
	NONEXT,
	&a3d_spm_group,
	85,41,4,28,
	NOTEXT,
	hang_children,
	NOFEEL,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);

			static Button a3d_eslide_sel = MB_INIT1(
				NONEXT,
				NOCHILD,
				146,11,69,62,
				&a3d_enl_slider,
				see_qslider,
				feel_rdc_qslider,
				NOOPT,
				NOGROUP,0,
				NOKEY,
				0
				);
				static Button a3d_spt_1 = MB_INIT1(
					NONEXT,
					NOCHILD,
					14,11,201,62,
					"1",
					ncorner_text,
					change_rot_scale,
					NOOPT,
					&vs.ado_turn,1,
					NOKEY,
					MB_GHILITE
					);
				static Button a3d_spt_2 = MB_INIT1(
					&a3d_spt_1,
					NOCHILD,
					25,11,177,62,
					"1/2",
					ncorner_text,
					change_rot_scale,
					NOOPT,
					&vs.ado_turn,2,
					NOKEY,
					MB_GHILITE
					);
				static Button a3d_spt_4 = MB_INIT1(
					&a3d_spt_2,
					NOCHILD,
					25,11,153,62,
					"1/4",
					ncorner_text,
					change_rot_scale,
					NOOPT,
					&vs.ado_turn,4,
					NOKEY,
					MB_GHILITE
					);
				static Button a3d_spt_6 = MB_INIT1(
					&a3d_spt_4,
					NOCHILD,
					24,11,130,62,
					"1/6",
					ncorner_text,
					change_rot_scale,
					NOOPT,
					&vs.ado_turn,6,
					NOKEY,
					MB_GHILITE
					);
				static Button a3d_spt_8 = MB_INIT1(
					&a3d_spt_6,
					NOCHILD,
					25,11,106,62,
					"1/8",
					ncorner_text,
					change_rot_scale,
					NOOPT,
					&vs.ado_turn,8,
					NOKEY,
					MB_GHILITE
					);
				static Button a3d_spt_360 = MB_INIT1(
					&a3d_spt_8,
					NOCHILD,
					38,11,69,62,
					"1/360",
					ncorner_text,
					change_rot_scale,
					NOOPT,
					&vs.ado_turn,360,
					NOKEY,
					MB_GHILITE
					);
			static Button a3d_spt_3group = MB_INIT1(
				NONEXT,
				&a3d_spt_360,
				146,11,69,62,
				NOTEXT,
				NOSEE,
				NOFEEL,
				NOOPT,
				NOGROUP,0,
				NOKEY,
				0
				);
				static Button a3d_spa_z = MB_INIT1(
					NONEXT,
					NOCHILD,
					50,11,165,62,
					"Z",
					ncorner_text,
					set_axis,
					NOOPT,
					NOGROUP,2,
					NOKEY,
					0
					);
				static Button a3d_spa_y = MB_INIT1(
					&a3d_spa_z,
					NOCHILD,
					49,11,117,62,
					"Y",
					ncorner_text,
					set_axis,
					NOOPT,
					NOGROUP,1,
					NOKEY,
					0
					);
				static Button a3d_spa_x = MB_INIT1(
					&a3d_spa_y,
					NOCHILD,
					49,11,69,62,
					"X",
					ncorner_text,
					set_axis,
					NOOPT,
					NOGROUP,0,
					NOKEY,
					0
					);
			static Button a3d_spa_3group = MB_INIT1(
				NONEXT,
				&a3d_spa_x,
				146,11,69,62,
				NOTEXT,
				NOSEE,
				NOFEEL,
				NOOPT,
				NOGROUP,0,
				NOKEY,
				0
				);
				static Button a3d_szc_same = MB_INIT1(
					NONEXT,
					NOCHILD,
					85,11,130,62,
					NODATA, /* "Same as Spin", */
					ncorner_text,
					csame_spin,
					NOOPT,
					NOGROUP,0,
					NOKEY,
					0
					);
				static Button a3d_szc_default = MB_INIT1(
					&a3d_szc_same,
					NOCHILD,
					62,11,69,62,
					NODATA, /* "Default", */
					ncorner_text,
					csize_default,
					NOOPT,
					NOGROUP,0,
					NOKEY,
					0
					);
			static Button a3d_szc_3group = MB_INIT1(
				NONEXT,
				&a3d_szc_default,
				146,11,69,62,
				NOTEXT,
				NOSEE,
				NOFEEL,
				NOOPT,
				NOGROUP,0,
				NOKEY,
				0
				);
				static Button a3d_spc_same = MB_INIT1(
					NONEXT,
					NOCHILD,
					85,11,130,62,
					NODATA, /* "Same as Size", */
					ncorner_text,
					csame_size,
					NOOPT,
					NOGROUP,0,
					NOKEY,
					0
					);
				static Button a3d_spc_default = MB_INIT1(
					&a3d_spc_same,
					NOCHILD,
					62,11,69,62,
					NODATA, /* "Default", */
					ncorner_text,
					cspin_default,
					NOOPT,
					NOGROUP,0,
					NOKEY,
					0
					);
			static Button a3d_spc_3group = MB_INIT1(
				NONEXT,
				&a3d_spc_default,
				146,11,69,62,
				NOTEXT,
				NOSEE,
				NOFEEL,
				NOOPT,
				NOGROUP,0,
				NOKEY,
				0
				);
		static Button a3d_g3_sel = MB_INIT1(
			NONEXT,
			&a3d_spc_3group,
			146,11,69,62,
			NOTEXT,
			hang_children,
			NOFEEL,
			NOOPT,
			NOGROUP,0,
			NOKEY,
			0
			);
					static Button a3d_zslide_sel = MB_INIT1(
						NONEXT,
						NOCHILD,
						146,11,69,52,
						&a3d_zslider,
						see_qslider,
						ado_xyz_slider,
						xyz_zero_sl,
						NOGROUP,2,
						NOKEY,
						0
						);
					static Button a3d_yslide_sel = MB_INIT1(
						&a3d_zslide_sel,
						NOCHILD,
						146,11,69,42,
						&a3d_yslider,
						see_qslider,
						ado_xyz_slider,
						xyz_zero_sl,
						NOGROUP,1,
						NOKEY,
						0
						);
					static Button a3d_xslide_sel = MB_INIT1(
						&a3d_yslide_sel,
						NOCHILD,
						146,11,69,32,
						&a3d_xslider,
						see_qslider,
						ado_xyz_slider,
						xyz_zero_sl,
						NOGROUP,0,
						NOKEY,
						0
						);
				static Button a3d_slider_group = MB_INIT1(
					NONEXT,
					&a3d_xslide_sel,
					146,31,69,32,
					NOTEXT,
					iscale_theta,
					NOFEEL,
					NOOPT,
					NOGROUP,0,
					NOKEY,
					0
					);

					static Button a3d_etag_sel = MB_INIT1(
						NONEXT,
						NOCHILD,
						146,11,69,52,
						NODATA, /* "Enlarge", */
						ncorner_text,
						NOFEEL,
						NOOPT,
						NOGROUP,0,
						NOKEY,
						0
						);
					static Button a3d_rslide_sel = MB_INIT1(
						&a3d_etag_sel,
						NOCHILD,
						146,11,69,42,
						&a3d_rdc_slider,
						see_qslider,
						feel_rdc_qslider,
						NOOPT,
						NOGROUP,0,
						NOKEY,
						0
						);
					static Button a3d_rtag_sel = MB_INIT1(
						&a3d_rslide_sel,
						NOCHILD,
						146,11,69,32,
						NODATA, /* "Reduce", */
						ncorner_text,
						NOFEEL,
						NOOPT,
						NOGROUP,0,
						NOKEY,
						0
						);
				static Button a3d_szslide_group = MB_INIT1(
					NONEXT,
					&a3d_rtag_sel,
					146,41,69,32,
					NOTEXT,
					NOSEE,
					NOFEEL,
					NOOPT,
					NOGROUP,0,
					NOKEY,
					0
					);
		static Button a3d_g2_sel = MB_INIT1(
			&a3d_g3_sel,
			&a3d_slider_group,
			146,31,69,32,
			NOTEXT,
			hang_children,
			NOFEEL,
			NOOPT,
			NOGROUP,0,
			NOKEY,
			0
			);
					static Button a3d_sz_ratio = MB_INIT1(
						NONEXT,
						NOCHILD,
						42,11,28,62,
						NOTEXT,
						see_size_ratio,
						NOFEEL,
						NOOPT,
						NOGROUP,0,
						NOKEY,
						0
						);
					static Button a3d_sz_both = MB_INIT1(
						&a3d_sz_ratio,
						NOCHILD,
						42,11,28,52,
						NODATA, /* "Both", */
						ncorner_text,
						change_size_mode,
						NOOPT,
						&vs.ado_size,3,
						NOKEY,
						MB_B_GHILITE
						);
					static Button a3d_sz_y = MB_INIT1(
						&a3d_sz_both,
						NOCHILD,
						21,11,49,42,
						"Y",
						ncorner_text,
						change_size_mode,
						NOOPT,
						&vs.ado_size,2,
						NOKEY,
						MB_B_GHILITE
						);
					static Button a3d_sz_x = MB_INIT1(
						&a3d_sz_y,
						NOCHILD,
						22,11,28,42,
						"X",
						ncorner_text,
						change_size_mode,
						NOOPT,
						&vs.ado_size,1,
						NOKEY,
						MB_B_GHILITE
						);
					static Button a3d_sz_center = MB_INIT1(
						&a3d_sz_x,
						NOCHILD,
						42,11,28,32,
						NODATA, /* "Center", */
						ncorner_text,
						change_size_mode,
						NOOPT,
						&vs.ado_size,0,
						NOKEY,
						MB_B_GHILITE
						);
				static Button a3d_size_group = MB_INIT1(
					NONEXT,
					&a3d_sz_center,
					42,41,28,32,
					NOTEXT,
					NOSEE,
					NOFEEL,
					NOOPT,
					NOGROUP,0,
					NOKEY,
					0
					);
					static Button a3d_sp_turns = MB_INIT1(
						NONEXT,
						NOCHILD,
						42,14,28,59,
						NODATA, /* "Turns", */
						ncorner_text,
						change_spin_mode,
						NOOPT,
						&vs.ado_spin,2,
						NOKEY,
						MB_B_GHILITE,
						);
					static Button a3d_sp_axis = MB_INIT1(
						&a3d_sp_turns,
						NOCHILD,
						42,14,28,46,
						NODATA, /* "Axis", */
						ncorner_text,
						change_spin_mode,
						NOOPT,
						&vs.ado_spin,1,
						NOKEY,
						MB_B_GHILITE,
						);
					static Button a3d_sp_center = MB_INIT1(
						&a3d_sp_axis,
						NOCHILD,
						42,15,28,32,
						NODATA, /* "Center", */
						ncorner_text,
						change_spin_mode,
						NOOPT,
						&vs.ado_spin,0,
						NOKEY,
						MB_B_GHILITE,
						);
				static Button a3d_spin_group = MB_INIT1(
					NONEXT,
					&a3d_sp_center,
					42,41,28,32,
					NOTEXT,
					NOSEE,
					NOFEEL,
					NOOPT,
					NOGROUP,0,
					NOKEY,
					0
					);
		static Button a3d_g1_sel = MB_INIT1(
			&a3d_g2_sel,
			&a3d_spin_group,
			42,41,28,32,
			NOTEXT,
			hang_children,
			NOFEEL,
			NOOPT,
			NOGROUP,0,
			NOKEY,
			0
			);
	static Button a3d_spsz_group = MB_INIT1(
		NONEXT,
		&a3d_g1_sel,
		187,41,28,32,
		NOTEXT,
		NOSEE,
		NOFEEL,
		NOOPT,
		NOGROUP,0,
		NOKEY,
		0
		);
#define PAX (-100)
#define PAY (4)

static Qslider ptens_sl = 
	QSL_INIT1( -20, 20, &vs.pa_tens, 0, NULL, leftright_arrs);
static Qslider pcont_sl = 
	QSL_INIT1( -20, 20, &vs.pa_cont, 0, NULL, leftright_arrs);
static Qslider pbias_sl = 
	QSL_INIT1( -20, 20, &vs.pa_bias, 0, NULL, leftright_arrs);

		static Button a3d_bias_sel = MB_INIT1(
			NONEXT,
			NOCHILD,
			112,11,PAX+203,(PAY+185)-127,
			&pbias_sl,
			see_qslider,
			feel_qslider,
			zero_sl,
			NOGROUP,0,
			NOKEY,
			0
			);
		static Button a3d_cont_sel = MB_INIT1(
			&a3d_bias_sel,
			NOCHILD,
			112,11,PAX+203,(PAY+175)-127,
			&pcont_sl,
			see_qslider,
			feel_qslider,
			zero_sl,
			NOGROUP,0,
			NOKEY,
			0
			);
		static Button a3d_tens_sel = MB_INIT1(
			&a3d_cont_sel,
			NOCHILD,
			112,11,PAX+203,(PAY+165)-127,
			&ptens_sl,
			see_qslider,
			feel_qslider,
			zero_sl,
			NOGROUP,0,
			NOKEY,
			0
			);
		static Button a3d_pa_closed = MB_INIT1(
			&a3d_tens_sel,
			NOCHILD,
			63,11,PAX+252,(PAY+155)-127,
			NODATA, /* "Closed", */
			ncorner_text,
			change_mode,
			NOOPT,
			&vs.pa_closed,1,
			NOKEY,
			MB_GHILITE
			);
		static Button a3d_pa_open = MB_INIT1(
			&a3d_pa_closed,
			NOCHILD,
			50,11,PAX+203,(PAY+155)-127,
			NODATA, /* "Open", */
			ncorner_text,
			change_mode,
			NOOPT,
			&vs.pa_closed,0,
			NOKEY,
			MB_GHILITE
			);
		static Button a3d_pa_save = MB_INIT1(
			&a3d_pa_open,
			NOCHILD,
			31,11,73,62,
			NODATA, /* "Save", */
			ncorner_text,
			mgo_path_files,
			NOOPT,
			NOGROUP,0,
			NOKEY,
			0
			);
		static Button a3d_pa_load = MB_INIT1(
			&a3d_pa_save,
			NOCHILD,
			31,11,73,52,
			NODATA, /* "Load", */
			ncorner_text,
			mgo_path_files,
			NOOPT,
			NOGROUP,0,
			NOKEY,
			0
			);
		static Button a3d_pa_view = MB_INIT1(
			&a3d_pa_load,
			NOCHILD,
			31,11,73,42,
			NODATA, /* "View", */
			ncorner_text,
			mview_path,
			NOOPT,
			NOGROUP,0,
			NOKEY,
			0
			);
		static Button a3d_pa_edit = MB_INIT1(
			&a3d_pa_view,
			NOCHILD,
			31,11,73,32,
			NODATA, /* "Edit", */
			ncorner_text,
			edit_path,
			NOOPT,
			NOGROUP,0,
			NOKEY,
			0
			);
		static Button a3d_pa_clocked = MB_INIT1(
			&a3d_pa_edit,
			NOCHILD,
			46,11,28,62,
			NODATA, /* "Clocked", */
			ncorner_text,
			change_mode,
			NOOPT,
			&vs.ado_path,3,
			NOKEY,
			MB_B_GHILITE
			);
		static Button a3d_pa_sampled = MB_INIT1(
			&a3d_pa_clocked,
			NOCHILD,
			46,11,28,52,
			NODATA, /* "Sampled", */
			ncorner_text,
			change_mode,
			NOOPT,
			&vs.ado_path,2,
			NOKEY,
			MB_B_GHILITE
			);
		static Button a3d_pa_poly = MB_INIT1(
			&a3d_pa_sampled,
			NOCHILD,
			46,11,28,42,
			NODATA, /* "Polygon", */
			ncorner_text,
			change_mode,
			NOOPT,
			&vs.ado_path,1,
			NOKEY,
			MB_B_GHILITE
			);
		static Button a3d_pa_spline = MB_INIT1(
			&a3d_pa_poly,
			NOCHILD,
			46,11,28,32,
			NODATA, /* "Spline", */
			ncorner_text,
			change_mode,
			NOOPT,
			&vs.ado_path,0,
			NOKEY,
			MB_B_GHILITE
			);
	static Button a3d_path_group = MB_INIT1(
		NONEXT,
		&a3d_pa_spline,
		187,41,28,32,
		NOTEXT,
		NOSEE,
		NOFEEL,
		NOOPT,
		NOGROUP,0,
		NOKEY,
		0
		);
		static Button a3d_mzslide_sel = MB_INIT1(
			NONEXT,
			NOCHILD,
			187,14,28,59,
			&a3d_zslider,
			see_qslider,
			feel_qslider,
			zero_sl,
			NOGROUP,2,
			NOKEY,
			0
			);
		static Button a3d_myslide_sel = MB_INIT1(
			&a3d_mzslide_sel,
			NOCHILD,
			187,14,28,(173)-127,
			&a3d_yslider,
			see_qslider,
			feel_qslider,
			zero_sl,
			NOGROUP,1,
			NOKEY,
			0
			);
		static Button a3d_mxslide_sel = MB_INIT1(
			&a3d_myslide_sel,
			NOCHILD,
			187,15,28,32,
			&a3d_xslider,
			see_qslider,
			feel_qslider,
			zero_sl,
			NOGROUP,0,
			NOKEY,
			0
			);
	static Button a3d_move_group = MB_INIT1(
		NONEXT,
		&a3d_mxslide_sel,
		187,41,28,32,
		NOTEXT,
		NOSEE,
		NOFEEL,
		NOOPT,
		NOGROUP,0,
		NOKEY,
		0
		);

static Button a3d_g0_sel = MB_INIT1(
	&a3d_g4_sel,
	&a3d_spsz_group,
	187,41,128,28,
	NOTEXT,
	hang_children,
	NOFEEL,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);
static Button a3d_path_sel = MB_INIT1(
	&a3d_g0_sel,
	NOCHILD,
	29,11,94,(185)-127,
	NODATA, /* "Path", */
	ncorner_text,
	change_ado_mode,
	NOOPT,
	&vs.ado_mode,3,
	NOKEY,
	MB_B_GHILITE,
	);
static Button a3d_move_sel = MB_INIT1(
	&a3d_path_sel,
	NOCHILD,
	29,11,94,(175)-127,
	NODATA, /* "Move", */
	ncorner_text,
	change_ado_mode,
	NOOPT,
	&vs.ado_mode,2,
	NOKEY,
	MB_B_GHILITE,
	);
static Button a3d_size_sel = MB_INIT1(
	&a3d_move_sel,
	NOCHILD,
	29,11,94,(165)-127,
	NODATA, /* "Size", */
	ncorner_text,
	change_ado_mode,
	NOOPT,
	&vs.ado_mode,1,
	NOKEY,
	MB_B_GHILITE,
	);
static Button a3d_spin_sel = MB_INIT1(
	&a3d_size_sel,
	NOCHILD,
	29,11,94,28,
	NODATA, /* "Spin", */
	ncorner_text,
	change_ado_mode,
	NOOPT,
	&vs.ado_mode,0,
	NOKEY,
	MB_B_GHILITE,
	);
static Button a3d_clear_sel = MB_INIT1(
	&a3d_spin_sel,
	NOCHILD,
	71,9,244,15,
	NODATA, /* "Clear Track", */
	ccorner_text,
	clear_track,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	MB_HIONSEL
	);
static Button a3d_clearm_sel = MB_INIT1(
	&a3d_clear_sel,
	NOCHILD,
	63,9,177,15,
	NODATA, /* "Clear Move", */
	ccorner_text,
	clear_pos,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	MB_HIONSEL
	);
static Button a3d_contin_sel = MB_INIT1(
	&a3d_clearm_sel,
	NOCHILD,
	58,9,115,15,
	NODATA, /* "Continue", */
	ccorner_text,
	move_along,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	MB_HIONSEL
	);
static Button a3d_render_sel = MB_INIT1(
	&a3d_contin_sel,
	NOCHILD,
	33,9,77,15,
	NODATA, /* "Use", */
	ccorner_text,
	mauto_ado,
	NOOPT,
	NOGROUP,0,
	'r',
	0
	);
static Button a3d_loop_sel = MB_INIT1(
	&a3d_render_sel,
	NOCHILD,
	33,9,40,15,
	NODATA, /* "Loop", */
	ccorner_text,
	mado_loop,
	NOOPT,
	NOGROUP,0,
	'l',
	0
	);
static Button a3d_view_sel = MB_INIT1(
	&a3d_loop_sel,
	NOCHILD,
	33,9,4,15,
	NODATA, /* "View", */
	ccorner_text,
	mado_view,
	NOOPT,
	NOGROUP,0,
	'v',
	0
	);

static Button a3d_cco_sel = MB_INIT1(
	&a3d_view_sel,
	NOCHILD,
	11,9,306,2,
	NOTEXT,
	ccolor_box,
	a3d_go_color_grid,
	ppalette,
	NOGROUP,0,
	NOKEY,
	0
	);
static Button a3d_bru_sel = MB_INIT1(
	&a3d_cco_sel,
	NOCHILD,
	11,11,293,2,
	NOTEXT,
	see_pen,
	toggle_pen,
	set_pbrush,
	NOGROUP,0,
	NOKEY,
	0
	);
static Button a3d_kmo_sel = MB_INIT1(
	&a3d_bru_sel,
	NOCHILD,
	11,9,220,3,
	NODATA, /* "K", */
	ncorner_text,
	mb_toggle_zclear,
	go_cel_menu,
	&vs.zero_clear,1,
	NOKEY,
	MB_B_GHILITE,
	);
static Button a3d_fmo_sel = MB_INIT1(
	&a3d_kmo_sel,
	NOCHILD,
	11,9,210,3,
	NODATA, /* "F", */
	ncorner_text,
	toggle_bgroup,
	NOOPT,
	&vs.fillp,1,
	NOKEY,
	MB_B_GHILITE,
	);
static Button a3d_ink_sel = MB_INIT1(
	&a3d_fmo_sel,
	NOCHILD,
	53,9,238,3,
	NOTEXT,
	see_cur_ink,
	qinks,
	qinks,
	&vs.ink_id,opq_INKID,
	NOKEY,
	MB_GHILITE,
	);

extern Minitime_data flxtime_data;

static Button a3d_minitime_hanger_sel = MB_INIT1(
	&a3d_ink_sel,
	&minitime_sel,
	79,9,129,3,
	NOTEXT,
	hang_children,
	NOFEEL,
	NOOPT,
	&flxtime_data,0,
	NOKEY,
	0
	);

static Button a3d_a_sel = MB_INIT1(
	&a3d_minitime_hanger_sel,
	NOCHILD,
	11,9,113,3,
	NODATA, /* "A", */
	dcorner_text,
	change_mode,
	NOOPT,
	&vs.time_mode,2,
	NOKEY,
	MB_GHILITE
	);
static Button a3d_s_sel = MB_INIT1(
	&a3d_a_sel,
	NOCHILD,
	11,9,103,3,
	NODATA, /* "S", */
	dcorner_text,
	change_mode,
	NOOPT,
	&vs.time_mode,1,
	NOKEY,
	MB_GHILITE
	);
static Button a3d_frame_sel = MB_INIT1(
	&a3d_s_sel,
	NOCHILD,
	11,9,93,3,
	NODATA, /* "F", */
	dcorner_text,
	change_mode,
	NOOPT,
	&vs.time_mode,0,
	NOKEY,
	MB_GHILITE
	);


static Button a3d_moveq_sel = MB_INIT1(
	&a3d_frame_sel,
	NOCHILD,
	82,9,4,3,
	NODATA, /* "Optics", */
	see_titlebar,
	feel_titlebar,
	mb_menu_to_bottom,
	&tbg_moveclose,0,
	'q',
	0,	/* flags */
	);

void seebg_a3d();


Menuhdr a3d_menu = {
	{320,72,0,127,},		/* width, height, x y */
	OPTIC_MUID,   		/* id */
	PANELMENU,			/* type */
	&a3d_moveq_sel, 	/* buttons */
	SCREEN_FONT,		/* font */
	&menu_cursor,		/* cursor */
	seebg_a3d, 			/* seebg */
	NULL,				/* dodata */
	NULL,				/* domenu */
	(MBPEN|MBRIGHT|KEYHIT), /* ioflags */
	0,				/* flags */
	NULL,			/* procmouse */
	NULL,			/* on_showhide */
	NULL,			/* cleanup */
};

static Smu_button_list a3d_smblist[] = {
	{ "title", &a3d_moveq_sel },
	{ "prop", &a3d_prop_sel },
	{ "szby_mouse", &a3d_szcleart_sel }, 
	{ "by_mouse", &a3d_cleart_sel },
	{ "as_spin", &a3d_szc_same },
	{ "szc_dflt", &a3d_szc_default },
	{ "as_size", &a3d_spc_same },
	{ "spc_dflt", &a3d_spc_default },
	{ "enlarge", &a3d_etag_sel },
	{ "reduce", &a3d_rtag_sel },
	{ "sz_both", &a3d_sz_both },
	{ "sz_cent", &a3d_sz_center },
	{ "sp_turns", &a3d_sp_turns },
	{ "sp_axis", &a3d_sp_axis },
	{ "sp_cent", &a3d_sp_center },
	{ "sp_closed", &a3d_pa_closed },
	{ "pa_open", &a3d_pa_open },
	{ "pa_save", &a3d_pa_save },
	{ "pa_load", &a3d_pa_load },
	{ "pa_view", &a3d_pa_view },
	{ "pa_edit", &a3d_pa_edit },
	{ "pa_clock", &a3d_pa_clocked },
	{ "pa_samp", &a3d_pa_sampled },
	{ "pa_poly", &a3d_pa_poly },
	{ "pa_spline", &a3d_pa_spline },
	{ "path", &a3d_path_sel },
	{ "move", &a3d_move_sel },
	{ "size", &a3d_size_sel },
	{ "spin", &a3d_spin_sel },
	{ "clear_tk", &a3d_clear_sel },
	{ "clear_mv", &a3d_clearm_sel },
	{ "continue", &a3d_contin_sel },
	{ "use", &a3d_render_sel },
	{ "loop", &a3d_loop_sel },
	{ "view", &a3d_view_sel },
	{ "kcol", &a3d_kmo_sel },
	{ "fillp", &a3d_fmo_sel },
	{ "t_all", &a3d_a_sel },
	{ "t_seg", &a3d_s_sel },
	{ "t_frm", &a3d_frame_sel },
	{ "pen", &a3d_bru_sel },
};

Errcode load_a3d_panel(void **ss)
{
	return(soft_buttons("optics_panel", a3d_smblist, 
						 Array_els(a3d_smblist), ss));
}

static void seebg_a3d(Menuwndo *mw)
{
	seebg_white(mw);
	arrange_a3d_menu();
}

static void a3d_go_color_grid(Button *b)
{
	go_color_grid(b);
	draw_button(&a3d_cco_sel);
}

SHORT got_path;
char inspin;


void a3d_disables(void)
{
static Button *disabtab[] = {
	&a3d_pa_view,
	&a3d_pa_save,
	&a3d_pa_edit,
	NULL,
};
	got_path = pj_exists(ppoly_name);
	set_mbtab_disables(disabtab,!got_path);
}


static Button *g3_spin_sels[] = { 
	&a3d_spc_3group,
	&a3d_spa_3group,
	&a3d_spt_3group,
	};

static
void sliders_to_point(Short_xyz *pt)
{
	a3d_xslider.value = &pt->x;
	a3d_yslider.value = &pt->y;
	a3d_zslider.value = &pt->z;
}

static
void sliders_500(void)
{
	a3d_xslider.min = a3d_yslider.min = a3d_zslider.min = -500;
	a3d_xslider.max = a3d_yslider.max = a3d_zslider.max = 500;
}

static
void sliders_center_bounds(void)
{
SHORT cent;
	sliders_500();
	cent = vb.pencel->width/2;
	a3d_xslider.min += cent;
	a3d_xslider.max += cent;
	cent = vb.pencel->height/2;
	a3d_yslider.min += cent;
	a3d_yslider.max += cent;
}


static
void change_rot_scale(Button *m)
{
	change_mode(m);
	arrange_a3d_menu();
	draw_button(&a3d_g2_sel);
}

static
void change_size_mode(Button *m)
{
	change_mode(m);
	arrange_a3d_menu();
	draw_button(&a3d_g2_sel);
	draw_button(&a3d_g3_sel);
	draw_button(&a3d_g4_sel);
	draw_buttontop(&a3d_sz_ratio);
}


static
void change_spin_mode(Button *m)
{
	change_mode(m);
	arrange_a3d_menu();
	draw_button(&a3d_g2_sel);
	draw_button(&a3d_g3_sel);
}


static
void change_ado_mode(Button *m)
{
	change_mode(m);
	arrange_a3d_menu();
	a3d_disables();
	draw_button(&a3d_g0_sel);
	draw_button(&a3d_g4_sel);
}

static
void set_axis(Button *m)
{
	vs.move3.spin_axis.x = vs.move3.spin_axis.y = vs.move3.spin_axis.z = 0;
	switch (m->identity)
	{
		case 0:
			vs.move3.spin_axis.x = 100;
			break;
		case 1:
			vs.move3.spin_axis.y = 100;
			break;
		case 2:
			vs.move3.spin_axis.z = 100;
			break;
	}
	draw_button(&a3d_g2_sel);
}

static
void csame_size(Button *m)
{
	pj_copy_structure(&vs.move3.size_center, 
					&vs.move3.spin_center, sizeof(Short_xyz) );
	draw_button(&a3d_g2_sel);
}

static
void csame_spin(Button *m)
{
	pj_copy_structure(&vs.move3.spin_center, 
					&vs.move3.size_center, sizeof(Short_xyz) );
	draw_button(&a3d_g2_sel);
}

static
void csize_default(Button *m)
{
	default_center(&vs.move3.size_center);
	draw_button(&a3d_g2_sel);
}

static
void see_size_ratio(Button *m)
{
char buf[12];
int over, under;
Boolean sign = FALSE;

	switch (vs.ado_size)
	{
		case 1: /* x */
			over = vs.move3.xp;
			under = vs.move3.xq;
			break;
		case 2: /* y */
			over = vs.move3.yp;
			under = vs.move3.yq;
			break;
		case 0:	/* center */
		case 3: /* both */
			over = vs.move3.bp;
			under = vs.move3.bq;
			break;
	}
	if (over < 0)	
		{
		sign = TRUE;
		over = -over;
		}
	if (under < 0)
		{
		sign = !sign;
		under = -under;
		}
	/* Convert to unsigned to avoid divide overflow in huge cases. */
	over *= 100;
	over /= under;
	sprintf(buf, "%s%d.%2d", (sign ? "-" : ""), over/100, over%100);
	tr_string(buf, ' ', '0');
	m->datme = buf;
	ncorner_text(m);
}

static
void feel_rdc_qslider(Button *m)
{
	feel_qslider(m);
	draw_buttontop(&a3d_sz_ratio);
}

static
void cspin_default(Button *m)
{
	default_center(&vs.move3.spin_center);
	draw_button(&a3d_g2_sel);
}

static
void clear_track(void)
{
	switch (vs.ado_mode)
	{
		case ADO_SPIN:
			default_center(&vs.move3.spin_center);
			pj_copy_structure(&default_vs.move3.spin_axis, &vs.move3.spin_axis,
				sizeof(Short_xyz) );
			pj_copy_structure(&default_vs.move3.spin_theta, &vs.move3.spin_theta,
				sizeof(Short_xyz) );
			iscale_theta();
			break;
		case ADO_SIZE:
			pj_copy_words(&default_vs.move3.xp, &vs.move3.xp, 6);
			default_center(&vs.move3.size_center);
			break;
		case ADO_MOVE:
			pj_copy_structure(&default_vs.move3.move, &vs.move3.move,
				sizeof(Short_xyz) );
			break;
		case ADO_PATH:
			pj_delete(ppoly_name);
			a3d_disables();
			break;
	}
	draw_button(&a3d_g0_sel);
}

static void clear_pos(void)
{
ado_clear_pos();
a3d_disables();
draw_button(&a3d_g0_sel);
}

static void mgo_path_files(void)
{
	hide_mp();
	go_files(7);
	show_mp();
}


void arrange_a3d_menu(void)
{
	inspin = 0;
	a3d_g4_sel.children = &a3d_spm_group;
	switch (vs.ado_mode)
	{
		case ADO_SPIN:		/* spin */
			a3d_g0_sel.children = &a3d_spsz_group;
			a3d_g1_sel.children = &a3d_spin_group;
			a3d_g2_sel.children = &a3d_slider_group;
			a3d_g3_sel.children = g3_spin_sels[vs.ado_spin];
			switch (vs.ado_spin)
			{
				case SPIN_CENTER:	/* center */
					sliders_to_point(&vs.move3.spin_center);
					sliders_center_bounds();
					break;
				case SPIN_AXIS: /* axis */
					sliders_to_point(&vs.move3.spin_axis);
					sliders_500();
					break;
				case SPIN_TURNS: /* turns */
					inspin = 1;
					sliders_to_point(&rot_theta);
					a3d_xslider.min = a3d_yslider.min = a3d_zslider.min = 
						-vs.ado_turn*10;
					a3d_xslider.max = a3d_yslider.max = a3d_zslider.max = 
						vs.ado_turn*10;
					break;
			}
			break;
		case ADO_SIZE:	/* size */
			a3d_g0_sel.children = &a3d_spsz_group;
			a3d_g1_sel.children = &a3d_size_group;
			a3d_g2_sel.children = &a3d_szslide_group;
			a3d_g3_sel.children = &a3d_eslide_sel;
			a3d_g4_sel.children = &a3d_szm_group;
			switch (vs.ado_size)
			{
				case 0:	/* center */
					a3d_g2_sel.children = &a3d_slider_group;
					a3d_g3_sel.children = &a3d_szc_3group;
					a3d_g4_sel.children = &a3d_spm_group;
					sliders_to_point(&vs.move3.size_center);
					sliders_center_bounds();
					break;
				case 1: /* x */
					a3d_rdc_slider.value = &vs.move3.xp;
					a3d_enl_slider.value = &vs.move3.xq;
					break;
				case 2: /* y */
					a3d_rdc_slider.value = &vs.move3.yp;
					a3d_enl_slider.value = &vs.move3.yq;
					break;
				case 3: /* both */
					a3d_rdc_slider.value = &vs.move3.bp;
					a3d_enl_slider.value = &vs.move3.bq;
					break;
			}
			break;
		case ADO_MOVE:	/* move */
			a3d_g0_sel.children = &a3d_move_group;
			sliders_to_point(&vs.move3.move);
			sliders_500();
			break;
		case ADO_PATH: /* path */
			a3d_g0_sel.children = &a3d_path_group;
			break;
	}
}


