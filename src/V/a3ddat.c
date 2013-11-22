
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
#include "flicmenu.h"
#include "fli.h"
#include "a3d.h"
#include "a3ddat.str"

extern dcorner_text(), ccorner_text(), gary_menu_back(), mundo_pic(),
	change_mode(), change_spin_mode(), change_size_mode(), change_ado_mode(),
	see_qslider(), feel_qslider(), change_rot_scale(), mgo_path_files(),
	path_info(), set_axis(), mado_preview(), mauto_ado(),
	ado_xyz_slider(), see_seg_size(), move_along(),
	go_multi(), change_time_mode(), mgo_stencil(),
	see_cur_ink(), force_opaque(), minks(), see_size_ratio(),
	mview_path(), edit_path(), zero_sl(), xyz_zero_sl(),
	csame_size(), csame_spin(), clear_track(),
	csize_default(), cspin_default(), mmouser(),
	toggle_pen(), ppalette(), ccolor_box(), ncorner_int(),
	ccorner_cursor(), ncorner_cursor(), ncorner_text(),
	ncorner_number(), dcorner_text(), iscale_theta(),
	hang_child(), move_tab_text(), move_menu(), bottom_menu(),
	feel_rdc_qslider(),  a3d_back(), see_mask_m(), toggle_mask(),
	see_pen(), toggle_group(), greytext(), set_pbrush(); 

extern Flicmenu minitime_sel, tseg_group_sel;

extern struct cursor cdown, cleft, cright, csleft,
	cinsert, cappend, cright2, cleft2, csright, cup, size_cursor,
	default_c;


Vertex rot_theta;	/* where the xyz sliders usually point... */

/* The usual x/y/z optics sliders */
static
struct qslider a3d_xslider =
	{
	-500,
	500,
	NULL,
	0,
	};
static
struct qslider a3d_yslider =
	{
	-500,
	500,
	NULL,
	0,
	};
static
struct qslider a3d_zslider =
	{
	-500,
	500,
	NULL,
	0,
	};

/* reduce/enlarge sliders */
static
struct qslider a3d_rdc_slider =
	{
	0,
	100,
	NULL,
	0,
	};
static
struct qslider a3d_enl_slider =
	{
	1,
	100,
	NULL,
	0,
	};

#define MCOFF (-9)

/* Branch of tree that covers mouse control buttons */

/* Mouse control section for size */
		static Flicmenu a3d_y_szm = {
			NONEXT,
			NOCHILD,
			MCOFF+278,186,19,10,
			a3ddat_128 /* "Y" */,
			dcorner_text,
			change_mode,
			&vs.ado_szmouse, 3,
			NOKEY,
			NOOPT,
			};
		static Flicmenu a3d_x_szm = {
			&a3d_y_szm,
			NOCHILD,
			MCOFF+255,186,19,10,
			a3ddat_129 /* "X" */,
			dcorner_text,
			change_mode,
			&vs.ado_szmouse, 2,
			NOKEY,
			NOOPT,
			};
		static Flicmenu a3d_xy_szm = {
			&a3d_x_szm,
			NOCHILD,
			MCOFF+232,186,19,10,
			a3ddat_110 /* "XY" */,
			dcorner_text,
			change_mode,
			&vs.ado_szmouse, 1,
			NOKEY,
			NOOPT,
			};
		static Flicmenu a3d_prop_sel = {
			&a3d_xy_szm,
			NOCHILD,
			MCOFF+227,173,76,10,
			a3ddat_103 /* "PROPORTIONAL" */,
			dcorner_text,
			change_mode,
			&vs.ado_szmouse, 0,
			NOKEY,
			NOOPT,
			};
		static Flicmenu a3d_szcleart_sel = {
			&a3d_prop_sel,
			NOCHILD,
			MCOFF+229,162,71,8,
			a3ddat_111 /* "MOUSE CONTROL" */,
			greytext,
			NOFEEL,
			NOGROUP, 0,
			NOKEY,
			NOOPT,
			};
	static Flicmenu a3d_szm_group = {
		NONEXT,
		&a3d_szcleart_sel,
		214, 159, 84, 40,
		NOTEXT,
		gary_menu_back,
		NOFEEL,
		NOGROUP, 0,
		NOKEY,
		NOOPT,
		};

/* mouse control section for all other states */
		static Flicmenu a3d_x_spm = {
			NONEXT,
			NOCHILD,
			MCOFF+277,186,19,10,
			a3ddat_129 /* "X" */,
			dcorner_text,
			change_mode,
			&vs.ado_mouse, 5,
			NOKEY,
			NOOPT,
			};
		static Flicmenu a3d_y_spm = {
			&a3d_x_spm,
			NOCHILD,
			MCOFF+255,186,19,10,
			a3ddat_128 /* "Y" */,
			dcorner_text,
			change_mode,
			&vs.ado_mouse, 4,
			NOKEY,
			NOOPT,
			};
		static Flicmenu a3d_z_spm = {
			&a3d_y_spm,
			NOCHILD,
			MCOFF+233,186,19,10,
			a3ddat_118 /* "Z" */,
			dcorner_text,
			change_mode,
			&vs.ado_mouse, 3,
			NOKEY,
			NOOPT,
			};
		static Flicmenu a3d_yz_spm = {
			&a3d_z_spm,
			NOCHILD,
			MCOFF+277,174,19,10,
			a3ddat_108 /* "ZY" */,
			dcorner_text,
			change_mode,
			&vs.ado_mouse, 2,
			NOKEY,
			NOOPT,
			};
		static Flicmenu a3d_zx_spm = {
			&a3d_yz_spm,
			NOCHILD,
			MCOFF+255,174,19,10,
			a3ddat_109 /* "XZ" */,
			dcorner_text,
			change_mode,
			&vs.ado_mouse, 1,
			NOKEY,
			NOOPT,
			};
		static Flicmenu a3d_xy_spm = {
			&a3d_zx_spm,
			NOCHILD,
			MCOFF+233,174,19,10,
			a3ddat_110 /* "XY" */,
			dcorner_text,
			change_mode,
			&vs.ado_mouse, 0,
			NOKEY,
			NOOPT,
			};
		static Flicmenu a3d_cleart_sel = {
			&a3d_xy_spm,
			NOCHILD,
			MCOFF+229,162,71,8,
			a3ddat_111 /* "MOUSE CONTROL" */,
			greytext,
			NOFEEL,
			NOGROUP, 0,
			NOKEY,
			NOOPT,
			};
	static Flicmenu a3d_spm_group = {
		NONEXT,
		&a3d_cleart_sel,
		214, 159, 84, 40,
		NOTEXT,
		gary_menu_back,
		NOFEEL,
		NOGROUP, 0,
		NOKEY,
		NOOPT,
		};
static Flicmenu a3d_g4_sel = {
	NONEXT,
	&a3d_spm_group,
	4, 155, 84, 40,
	NOTEXT,
	hang_child,
	NOFEEL,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};

			static Flicmenu a3d_eslide_sel = {
				NONEXT,
				NOCHILD,
				69, 189, 145, 10,
				&a3d_enl_slider,
				see_qslider,
				feel_rdc_qslider,
				NOGROUP, 0,
				NOKEY,
				NOOPT,
				};
				static Flicmenu a3d_spt_1 = {
					NONEXT,
					NOCHILD,
					201, 189, 13, 10,
					a3ddat_112 /* "1" */,
					ncorner_text,
					change_rot_scale,
					&vs.ado_turn, 1,
					NOKEY,
					NOOPT,
					};
				static Flicmenu a3d_spt_2 = {
					&a3d_spt_1,
					NOCHILD,
					177, 189, 24, 10,
					a3ddat_113 /* "1/2" */,
					ncorner_text,
					change_rot_scale,
					&vs.ado_turn, 2,
					NOKEY,
					NOOPT,
					};
				static Flicmenu a3d_spt_4 = {
					&a3d_spt_2,
					NOCHILD,
					153, 189, 24, 10,
					a3ddat_114 /* "1/4" */,
					ncorner_text,
					change_rot_scale,
					&vs.ado_turn, 4,
					NOKEY,
					NOOPT,
					};
				static Flicmenu a3d_spt_6 = {
					&a3d_spt_4,
					NOCHILD,
					130, 189, 23, 10,
					a3ddat_115 /* "1/6" */,
					ncorner_text,
					change_rot_scale,
					&vs.ado_turn, 6,
					NOKEY,
					NOOPT,
					};
				static Flicmenu a3d_spt_8 = {
					&a3d_spt_6,
					NOCHILD,
					106, 189, 24, 10,
					a3ddat_116 /* "1/8" */,
					ncorner_text,
					change_rot_scale,
					&vs.ado_turn, 8,
					NOKEY,
					NOOPT,
					};
				static Flicmenu a3d_spt_360 = {
					&a3d_spt_8,
					NOCHILD,
					69, 189, 37, 10,
					a3ddat_117 /* "1/360" */,
					ncorner_text,
					change_rot_scale,
					&vs.ado_turn, 360,
					NOKEY,
					NOOPT,
					};
			static Flicmenu a3d_spt_3group = {
				NONEXT,
				&a3d_spt_360,
				69, 189, 145, 10,
				NOTEXT,
				NOSEE,
				NOFEEL,
				NOGROUP, 0,
				NOKEY,
				NOOPT,
				};
				static Flicmenu a3d_spa_z = {
					NONEXT,
					NOCHILD,
					165, 189, 49, 10,
					a3ddat_118 /* "Z" */,
					ncorner_text,
					set_axis,
					NOGROUP, 2,
					NOKEY,
					NOOPT,
					};
				static Flicmenu a3d_spa_y = {
					&a3d_spa_z,
					NOCHILD,
					117, 189, 48, 10,
					a3ddat_128 /* "Y" */,
					ncorner_text,
					set_axis,
					NOGROUP, 1,
					NOKEY,
					NOOPT,
					};
				static Flicmenu a3d_spa_x = {
					&a3d_spa_y,
					NOCHILD,
					69, 189, 48, 10,
					a3ddat_129 /* "X" */,
					ncorner_text,
					set_axis,
					NOGROUP, 0,
					NOKEY,
					NOOPT,
					};
			static Flicmenu a3d_spa_3group = {
				NONEXT,
				&a3d_spa_x,
				69, 189, 145, 10,
				NOTEXT,
				NOSEE,
				NOFEEL,
				NOGROUP, 0,
				NOKEY,
				NOOPT,
				};
				static Flicmenu a3d_szc_same = {
					NONEXT,
					NOCHILD,
					130, 189, 84, 10,
					a3ddat_121 /* "SAME AS SPIN" */,
					ncorner_text,
					csame_spin,
					NOGROUP, 0,
					NOKEY,
					NOOPT,
					};
				static Flicmenu a3d_szc_default = {
					&a3d_szc_same,
					NOCHILD,
					69, 189, 61, 10,
					a3ddat_124 /* "DEFAULT" */,
					ncorner_text,
					csize_default,
					NOGROUP, 0,
					NOKEY,
					NOOPT,
					};
			static Flicmenu a3d_szc_3group = {
				NONEXT,
				&a3d_szc_default,
				69, 189, 145, 10,
				NOTEXT,
				NOSEE,
				NOFEEL,
				NOGROUP, 0,
				NOKEY,
				NOOPT,
				};
				static Flicmenu a3d_spc_same = {
					NONEXT,
					NOCHILD,
					130, 189, 84, 10,
					a3ddat_123 /* "SAME AS SIZE" */,
					ncorner_text,
					csame_size,
					NOGROUP, 0,
					NOKEY,
					NOOPT,
					};
				static Flicmenu a3d_spc_default = {
					&a3d_spc_same,
					NOCHILD,
					69, 189, 61, 10,
					a3ddat_124 /* "DEFAULT" */,
					ncorner_text,
					cspin_default,
					NOGROUP, 0,
					NOKEY,
					NOOPT,
					};
			static Flicmenu a3d_spc_3group = {
				NONEXT,
				&a3d_spc_default,
				69, 189, 145, 10,
				NOTEXT,
				NOSEE,
				NOFEEL,
				NOGROUP, 0,
				NOKEY,
				NOOPT,
				};
		static Flicmenu a3d_g3_sel = {
			NONEXT,
			&a3d_spc_3group,
			69, 189, 145, 10,
			NOTEXT,
			hang_child,
			NOFEEL,
			NOGROUP, 0,
			NOKEY,
			NOOPT,
			};
					static Flicmenu a3d_zslide_sel = {
						NONEXT,
						NOCHILD,
						69, 179, 145, 10,
						&a3d_zslider,
						see_qslider,
						ado_xyz_slider,
						NOGROUP, 2,
						NOKEY,
						xyz_zero_sl,
						};
					static Flicmenu a3d_yslide_sel = {
						&a3d_zslide_sel,
						NOCHILD,
						69, 169, 145, 10,
						&a3d_yslider,
						see_qslider,
						ado_xyz_slider,
						NOGROUP, 1,
						NOKEY,
						xyz_zero_sl,
						};
					static Flicmenu a3d_xslide_sel = {
						&a3d_yslide_sel,
						NOCHILD,
						69, 159, 145, 10,
						&a3d_xslider,
						see_qslider,
						ado_xyz_slider,
						NOGROUP, 0,
						NOKEY,
						xyz_zero_sl,
						};
				static Flicmenu a3d_slider_group = {
					NONEXT,
					&a3d_xslide_sel,
					69, 159, 145, 30,
					NOTEXT,
					iscale_theta,
					NOFEEL,
					NOGROUP, 0,
					NOKEY,
					NOOPT,
					};

					static Flicmenu a3d_etag_sel = {
						NONEXT,
						NOCHILD,
						69, 179, 145, 10,
						a3ddat_125 /* "ENLARGE" */,
						ncorner_text,
						NOFEEL,
						NOGROUP, 0,
						NOKEY,
						NOOPT,
						};
					static Flicmenu a3d_rslide_sel = {
						&a3d_etag_sel,
						NOCHILD,
						69, 169, 145, 10,
						&a3d_rdc_slider,
						see_qslider,
						feel_rdc_qslider,
						NOGROUP, 0,
						NOKEY,
						NOOPT,
						};
					static Flicmenu a3d_rtag_sel = {
						&a3d_rslide_sel,
						NOCHILD,
						69, 159, 145, 10,
						a3ddat_126 /* "REDUCE" */,
						ncorner_text,
						NOFEEL,
						NOGROUP, 0,
						NOKEY,
						NOOPT,
						};
				static Flicmenu a3d_szslide_group = {
					NONEXT,
					&a3d_rtag_sel,
					69, 159, 145, 40,
					NOTEXT,
					NOSEE,
					NOFEEL,
					NOGROUP, 0,
					NOKEY,
					NOOPT,
					};
		static Flicmenu a3d_g2_sel = {
			&a3d_g3_sel,
			&a3d_slider_group,
			69, 159, 145, 30,
			NOTEXT,
			hang_child,
			NOFEEL,
			NOGROUP, 0,
			NOKEY,
			NOOPT,
			};
					static Flicmenu a3d_sz_ratio = {
						NONEXT,
						NOCHILD,
						28, 189, 41, 10,
						NOTEXT,
						see_size_ratio,
						NOFEEL,
						NOGROUP, 0,
						NOKEY,
						NOOPT,
						};
					static Flicmenu a3d_sz_both = {
						&a3d_sz_ratio,
						NOCHILD,
						28, 179, 41, 10,
						a3ddat_127 /* "BOTH" */,
						ncorner_text,
						change_size_mode,
						&vs.ado_size, 3,
						NOKEY,
						NOOPT,
						};
					static Flicmenu a3d_sz_y = {
						&a3d_sz_both,
						NOCHILD,
						49, 169, 20, 10,
						a3ddat_128 /* "Y" */,
						ncorner_text,
						change_size_mode,
						&vs.ado_size, 2,
						NOKEY,
						NOOPT,
						};
					static Flicmenu a3d_sz_x = {
						&a3d_sz_y,
						NOCHILD,
						28, 169, 21, 10,
						a3ddat_129 /* "X" */,
						ncorner_text,
						change_size_mode,
						&vs.ado_size, 1,
						NOKEY,
						NOOPT,
						};
					static Flicmenu a3d_sz_center = {
						&a3d_sz_x,
						NOCHILD,
						28, 159, 41, 10,
						a3ddat_133 /* "CENTER" */,
						ncorner_text,
						change_size_mode,
						&vs.ado_size, 0,
						NOKEY,
						NOOPT,
						};
				static Flicmenu a3d_size_group = {
					NONEXT,
					&a3d_sz_center,
					28, 159, 41, 40,
					NOTEXT,
					NOSEE,
					NOFEEL,
					NOGROUP, 0,
					NOKEY,
					NOOPT,
					};
					static Flicmenu a3d_sp_turns = {
						NONEXT,
						NOCHILD,
						28, 186, 41, 13,
						a3ddat_131 /* "TURNS" */,
						ncorner_text,
						change_spin_mode,
						&vs.ado_spin, 2,
						NOKEY,
						NOOPT,
						};
					static Flicmenu a3d_sp_axis = {
						&a3d_sp_turns,
						NOCHILD,
						28, 173, 41, 13,
						a3ddat_132 /* "AXIS" */,
						ncorner_text,
						change_spin_mode,
						&vs.ado_spin, 1,
						NOKEY,
						NOOPT,
						};
					static Flicmenu a3d_sp_center = {
						&a3d_sp_axis,
						NOCHILD,
						28, 159, 41, 14,
						a3ddat_133 /* "CENTER" */,
						ncorner_text,
						change_spin_mode,
						&vs.ado_spin, 0,
						NOKEY,
						NOOPT,
						};
				static Flicmenu a3d_spin_group = {
					NONEXT,
					&a3d_sp_center,
					28, 159, 41, 40,
					NOTEXT,
					NOSEE,
					NOFEEL,
					NOGROUP, 0,
					NOKEY,
					NOOPT,
					};
		static Flicmenu a3d_g1_sel = {
			&a3d_g2_sel,
			&a3d_spin_group,
			28, 159, 41, 40,
			NOTEXT,
			hang_child,
			NOFEEL,
			NOGROUP, 0,
			NOKEY,
			NOOPT,
			};
	static Flicmenu a3d_spsz_group = {
		NONEXT,
		&a3d_g1_sel,
		28,159,186,40,
		NOSEE,
		NOFEEL,
		NOGROUP, 0,
		NOKEY,
		NOOPT,
		};
#define PAX (-100)
#define PAY (4)
static struct qslider ptens_sl = { -20, 20, &vs.pa_tens, 0};
static struct qslider pcont_sl = { -20, 20, &vs.pa_cont, 0};
static struct qslider pbias_sl = { -20, 20, &vs.pa_bias, 0};

		static Flicmenu a3d_bias_sel =
			{
			NONEXT,
			NOCHILD,
			PAX+203,PAY+185,111,10,
			&pbias_sl,
			see_qslider,
			feel_qslider,
			NOGROUP, 0,
			NOKEY,
			zero_sl,
			};
		static Flicmenu a3d_cont_sel =
			{
			&a3d_bias_sel,
			NOCHILD,
			PAX+203,PAY+175,111,10,
			&pcont_sl,
			see_qslider,
			feel_qslider,
			NOGROUP, 0,
			NOKEY,
			zero_sl,
			};
		static Flicmenu a3d_tens_sel =
			{
			&a3d_cont_sel,
			NOCHILD,
			PAX+203,PAY+165,111,10,
			&ptens_sl,
			see_qslider,
			feel_qslider,
			NOGROUP, 0,
			NOKEY,
			zero_sl,
			};
		static Flicmenu a3d_pa_closed = {
			&a3d_tens_sel,
			NOCHILD,
			PAX+252,PAY+155,62,10,
			a3ddat_134 /* "CLOSED" */,
			ncorner_text,
			change_mode,
			&vs.pa_closed, 1,
			NOKEY,
			NOOPT,
			};
		static Flicmenu a3d_pa_open = {
			&a3d_pa_closed,
			NOCHILD,
			PAX+203,PAY+155,49,10,
			a3ddat_135 /* "OPEN" */,
			ncorner_text,
			change_mode,
			&vs.pa_closed, 0,
			NOKEY,
			NOOPT,
			};
		static Flicmenu a3d_pa_save = {
			&a3d_pa_open,
			NOCHILD,
			73,189,30,10,
			a3ddat_136 /* "SAVE" */,
			ncorner_text,
			mgo_path_files,
			NOGROUP, 0,
			NOKEY,
			NOOPT,
			};
		static Flicmenu a3d_pa_load = {
			&a3d_pa_save,
			NOCHILD,
			73,179,30,10,
			a3ddat_137 /* "LOAD" */,
			ncorner_text,
			mgo_path_files,
			NOGROUP, 0,
			NOKEY,
			NOOPT,
			};
		static Flicmenu a3d_pa_view = {
			&a3d_pa_load,
			NOCHILD,
			73,169,30,10,
			a3ddat_138 /* "VIEW" */,
			ncorner_text,
			mview_path,
			NOGROUP, 0,
			NOKEY,
			NOOPT,
			};
		static Flicmenu a3d_pa_edit = {
			&a3d_pa_view,
			NOCHILD,
			73,159,30,10,
			a3ddat_139 /* "EDIT" */,
			ncorner_text,
			edit_path,
			NOGROUP, 0,
			NOKEY,
			NOOPT,
			};
		static Flicmenu a3d_pa_clocked = {
			&a3d_pa_edit,
			NOCHILD,
			28,189,45,10,
			a3ddat_140 /* "CLOCKED" */,
			ncorner_text,
			change_mode,
			&vs.ado_path, 3,
			NOKEY,
			NOOPT,
			};
		static Flicmenu a3d_pa_sampled = {
			&a3d_pa_clocked,
			NOCHILD,
			28,179,45,10,
			a3ddat_141 /* "SAMPLED" */,
			ncorner_text,
			change_mode,
			&vs.ado_path, 2,
			NOKEY,
			NOOPT,
			};
		static Flicmenu a3d_pa_poly = {
			&a3d_pa_sampled,
			NOCHILD,
			28,169,45,10,
			a3ddat_142 /* "POLYGON" */,
			ncorner_text,
			change_mode,
			&vs.ado_path, 1,
			NOKEY,
			NOOPT,
			};
		static Flicmenu a3d_pa_spline = {
			&a3d_pa_poly,
			NOCHILD,
			28,159,45,10,
			a3ddat_143 /* "SPLINE" */,
			ncorner_text,
			change_mode,
			&vs.ado_path, 0,
			NOKEY,
			NOOPT,
			};
	static Flicmenu a3d_path_group = {
		NONEXT,
		&a3d_pa_spline,
		28, 159, 186, 40,
		NOTEXT,
		NOSEE,
		NOFEEL,
		NOGROUP, 0,
		NOKEY,
		NOOPT,
		};
		static Flicmenu a3d_mzslide_sel = {
			NONEXT,
			NOCHILD,
			28, 186, 186, 13,
			&a3d_zslider,
			see_qslider,
			feel_qslider,
			NOGROUP, 2,
			NOKEY,
			zero_sl,
			};
		static Flicmenu a3d_myslide_sel = {
			&a3d_mzslide_sel,
			NOCHILD,
			28, 173, 186, 13,
			&a3d_yslider,
			see_qslider,
			feel_qslider,
			NOGROUP, 1,
			NOKEY,
			zero_sl,
			};
		static Flicmenu a3d_mxslide_sel = {
			&a3d_myslide_sel,
			NOCHILD,
			28, 159, 186, 14,
			&a3d_xslider,
			see_qslider,
			feel_qslider,
			NOGROUP, 0,
			NOKEY,
			zero_sl,
			};
	static Flicmenu a3d_move_group = {
		NONEXT,
		&a3d_mxslide_sel,
		28, 159, 186, 40,
		NOTEXT,
		NOSEE,
		NOFEEL,
		NOGROUP, 0,
		NOKEY,
		NOOPT,
		};
static Flicmenu a3d_g0_sel = {
	&a3d_g4_sel,
	&a3d_spsz_group,
	128,155,186,40,
	NOTEXT,
	hang_child,
	NOFEEL,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
static Flicmenu a3d_path_sel = {
	&a3d_g0_sel,
	NOCHILD,
	94, 185, 28, 10,
	a3ddat_144 /* "PATH" */,
	ncorner_text,
	change_ado_mode,
	&vs.ado_mode, 3,
	NOKEY,
	NOOPT,
	};
static Flicmenu a3d_move_sel = {
	&a3d_path_sel,
	NOCHILD,
	94, 175, 28, 10,
	a3ddat_145 /* "MOVE" */,
	ncorner_text,
	change_ado_mode,
	&vs.ado_mode, 2,
	NOKEY,
	NOOPT,
	};
static Flicmenu a3d_size_sel = {
	&a3d_move_sel,
	NOCHILD,
	94, 165, 28, 10,
	a3ddat_146 /* "SIZE" */,
	ncorner_text,
	change_ado_mode,
	&vs.ado_mode, 1,
	NOKEY,
	NOOPT,
	};
static Flicmenu a3d_spin_sel = {
	&a3d_size_sel,
	NOCHILD,
	94, 155, 28, 10,
	a3ddat_147 /* "SPIN" */,
	ncorner_text,
	change_ado_mode,
	&vs.ado_mode, 0,
	NOKEY,
	NOOPT,
	};
static Flicmenu a3d_clear_sel = {
	&a3d_spin_sel,
	NOCHILD,
	238,142,76,8,
	a3ddat_148 /* "CLEAR TRACK" */,
	ccorner_text,
	clear_track,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
static Flicmenu a3d_contin_sel = {
	&a3d_clear_sel,
	NOCHILD,
	143, 142, 86, 8,
	a3ddat_149 /* "CONTINUE MOVE" */,
	ccorner_text,
	move_along,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};
static Flicmenu a3d_render_sel = {
	&a3d_contin_sel,
	NOCHILD,
	83, 142, 51, 8,
	a3ddat_150 /* "USE" */,
	ccorner_text,
	mauto_ado,
	NOGROUP, 0,
	'r',
	NOOPT,
	};
static Flicmenu a3d_preview_sel = {
	&a3d_render_sel,
	NOCHILD,
	4, 142, 70, 8,
	a3ddat_151 /* "WIREFRAME" */,
	ccorner_text,
	mado_preview,
	NOGROUP, 0,
	'w',
	NOOPT,
	};

static Flicmenu a3d_cco_sel = {
	&a3d_preview_sel,
	NOCHILD,
	304, 130, 10, 8,
	NOTEXT,
	ccolor_box,
	ppalette,
	NOGROUP, 0,
	NOKEY,
	ppalette,
	};
static Flicmenu a3d_bru_sel = {
	&a3d_cco_sel,
	NOCHILD,
	287, 129, 10, 10,
	NOTEXT,
	see_pen,
	toggle_pen,
	NOGROUP, 0,
	NOKEY,
	set_pbrush,
	};
static Flicmenu a3d_kmo_sel = {
	&a3d_bru_sel,
	NOCHILD,
	271, 130, 10, 8,
	a3ddat_152 /* "k" */,
	ccorner_text,
	toggle_group,
	&vs.zero_clear, 1,
	NOKEY,
	NOOPT,
	};
static Flicmenu a3d_smo_sel = {
	&a3d_kmo_sel,
	NOCHILD,
	256, 130, 10, 8,
	NULL,
	see_mask_m,
	toggle_mask,
	&vs.use_mask, 1,
	NOKEY,
	mgo_stencil,
	};
static Flicmenu a3d_fmo_sel = {
	&a3d_smo_sel,
	NOCHILD,
	241, 130, 10, 8,
	a3ddat_158 /* "f" */,
	ccorner_text,
	toggle_group,
	&vs.fillp, 1,
	NOKEY,
	NOOPT,
	};
static Flicmenu a3d_mmo_sel = {
	&a3d_fmo_sel,
	NOCHILD,
	226, 130, 10, 8,
	a3ddat_155 /* "T" */,
	ccorner_text,
	toggle_group,
	&vs.multi, 1,
	NOKEY,
	go_multi,
	};
static Flicmenu a3d_ink_sel = {
	&a3d_mmo_sel,
	NOCHILD,
	183, 130, 38, 8,
	NOTEXT,
	see_cur_ink,
	force_opaque,
	&vs.draw_mode,0,
	NOKEY,
	minks,
	};
static Flicmenu a3d_minitime_hanger_sel = {
	&a3d_ink_sel,
	&minitime_sel,
	103, 130, 78, 8,
	NOTEXT,
	hang_child,
	NOFEEL,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};

static Flicmenu a3d_a_sel = {
	&a3d_minitime_hanger_sel,
	NOCHILD,
	83, 130, 10, 8,
	a3ddat_156 /* "a" */,
	ccorner_text,
	change_mode,
	&vs.time_mode,2,
	NOKEY,
	NOOPT,
	};
static Flicmenu a3d_s_sel = {
	&a3d_a_sel,
	NOCHILD,
	73, 130, 10, 8,
	a3ddat_157 /* "s" */,
	ccorner_text,
	change_mode,
	&vs.time_mode,1,
	NOKEY,
	NOOPT,
	};
static Flicmenu a3d_frame_sel = {
	&a3d_s_sel,
	NOCHILD,
	63, 130, 10, 8,
	a3ddat_158 /* "f" */,
	ccorner_text,
	change_mode,
	&vs.time_mode,0,
	NOKEY,
	NOOPT,
	};

static Flicmenu a3d_moveq_sel = {
	&a3d_frame_sel,
	NOCHILD,
	4,130,50,8,
	a3ddat_159 /* "OPTICS" */,
	move_tab_text,
	move_menu,
	NOGROUP, 0,
	NOKEY,
	bottom_menu,
	};
Flicmenu a3d_menu = 
	{
	NONEXT,
	&a3d_moveq_sel,
	0, 127, 319, 72,
	NOTEXT,
	a3d_back,
	NOFEEL,
	NOGROUP, 0,
	NOKEY,
	NOOPT,
	};

static
a3d_back(m)
Flicmenu *m;
{
set_ado_asterisks();
gary_menu_back(m);
}

WORD got_path;
char inspin;


a3d_disables()
{
got_path = jexists(ppoly_name);
a3d_pa_view.disabled = a3d_pa_save.disabled = 
	a3d_pa_edit.disabled = !got_path;
}


static Flicmenu *g3_spin_sels[] = { 
	&a3d_spc_3group,
	&a3d_spa_3group,
	&a3d_spt_3group,
	};

static
sliders_to_point(pt)
Vertex *pt;
{
a3d_xslider.value = &pt->x;
a3d_yslider.value = &pt->y;
a3d_zslider.value = &pt->z;
}

static
sliders_500()
{
a3d_xslider.min = a3d_yslider.min = a3d_zslider.min = -500;
a3d_xslider.max = a3d_yslider.max = a3d_zslider.max = 500;
}

static
sliders_center_bounds()
{
sliders_500();
a3d_xslider.min += XMAX/2;
a3d_xslider.max += XMAX/2;
a3d_yslider.min += YMAX/2;
a3d_yslider.max += YMAX/2;
}


static
change_rot_scale(m)
Flicmenu *m;
{
change_mode(m);
arrange_a3d_menu();
qdraw_a_menu(&a3d_g2_sel);
}

static
change_size_mode(m)
Flicmenu *m;
{
change_mode(m);
arrange_a3d_menu();
qdraw_a_menu(&a3d_g2_sel);
qdraw_a_menu(&a3d_g3_sel);
qdraw_a_menu(&a3d_g4_sel);
draw_sel(&a3d_sz_ratio);
}


static
change_spin_mode(m)
Flicmenu *m;
{
change_mode(m);
arrange_a3d_menu();
qdraw_a_menu(&a3d_g2_sel);
qdraw_a_menu(&a3d_g3_sel);
}


static
change_ado_mode(m)
Flicmenu *m;
{
change_mode(m);
arrange_a3d_menu();
a3d_disables();
qdraw_a_menu(&a3d_g0_sel);
qdraw_a_menu(&a3d_g4_sel);
}

static
set_axis(m)
Flicmenu *m;
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
qdraw_a_menu(&a3d_g2_sel);
}

static
csame_size(m)
Flicmenu *m;
{
copy_structure(&vs.move3.size_center, &vs.move3.spin_center, sizeof(Vertex) );
qdraw_a_menu(&a3d_g2_sel);
}

static
csame_spin(m)
Flicmenu *m;
{
copy_structure(&vs.move3.spin_center, &vs.move3.size_center, sizeof(Vertex) );
qdraw_a_menu(&a3d_g2_sel);
}

static
csize_default(m)
Flicmenu *m;
{
default_center(&vs.move3.size_center);
qdraw_a_menu(&a3d_g2_sel);
}

static
see_size_ratio(m)
Flicmenu *m;
{
char buf[10];
int over, under;

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
over *= 100;
over /= under;
sprintf(buf, "%d.%2d", over/100, over%100);
tr_string(buf, ' ', '0');
m->text = buf;
ncorner_text(m);
}

static
feel_rdc_qslider(m)
Flicmenu *m;
{
feel_qslider(m);
draw_sel(&a3d_sz_ratio);
}

static
cspin_default(m)
Flicmenu *m;
{
default_center(&vs.move3.spin_center);
qdraw_a_menu(&a3d_g2_sel);
}

static
clear_track()
{
switch (vs.ado_mode)
	{
	case ADO_SPIN:
		default_center(&vs.move3.spin_center);
		copy_structure(&default_vs.move3.spin_axis, &vs.move3.spin_axis,
			sizeof(Vertex) );
		copy_structure(&default_vs.move3.spin_theta, &vs.move3.spin_theta,
			sizeof(Vertex) );
		iscale_theta();
		break;
	case ADO_SIZE:
		copy_words(&default_vs.move3.xp, &vs.move3.xp, 6);
		default_center(&vs.move3.size_center);
		break;
	case ADO_MOVE:
		copy_structure(&default_vs.move3.move, &vs.move3.move,
			sizeof(Vertex) );
		break;
	case ADO_PATH:
		jdelete(ppoly_name);
		a3d_disables();
		break;
	}
qdraw_a_menu(&a3d_g0_sel);
}

static
mgo_path_files()
{
hide_mp();
go_files(7);
draw_mp();
}


arrange_a3d_menu()
{
struct ado_setting *op;

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
