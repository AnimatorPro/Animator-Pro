
/* mainpull.c - The data structures for the top level drop-down menus up
   along the top.  Also a routine to disable menu options we cant' deal
   with yet. */

#include "jimk.h"
#include "flicmenu.h"
#include "fli.h"
#include "mainpull.str"


			static struct pull oth_sta_pull = {
				NONEXT,
				1, 1+5*CH_HEIGHT, 75, 9,
				NOCHILD,
				mainpull_100 /* "INFO       ?" */,
				pull_text,
				0,
				};
			static struct pull oth_con_pull = {
				&oth_sta_pull,
				1, 1+4*CH_HEIGHT, 75, 9,
				NOCHILD,
				mainpull_101 /* "CONFIGURE..." */,
				pull_text,
				0,
				};
			static struct pull oth_set_pull = {
				&oth_con_pull,
				1, 1+3*CH_HEIGHT, 75, 9,
				NOCHILD,
				mainpull_102 /* "SETTINGS..." */,
				pull_text,
				0,
				};
			static struct pull oth_mac_pull = {
				&oth_set_pull,
				1, 1+2*CH_HEIGHT, 75, 9,
				NOCHILD,
				mainpull_103 /* "Record..." */,
				pull_text,
				0,
				};
			static struct pull oth_gri_pull = {
				&oth_mac_pull,
				1, 1+1*CH_HEIGHT, 75, 9,
				NOCHILD,
				mainpull_104 /* "GRID..." */,
				pull_text,
				0,
				};
			static struct pull oth_ste_pull = {
				&oth_gri_pull,
				1, 1+0*CH_HEIGHT, 75, 9,
				NOCHILD,
				mainpull_105 /* "MASK..." */,
				pull_text,
				0,
				};
		static struct pull roth_pull = {
			NONEXT,
			-1-36-2, 8, 77, 43+CH_HEIGHT,
			&oth_ste_pull,
			NODATA,
			pull_oblock,
			0,
			};
	static struct pull oth_pull = {
		NONEXT,
		282, -1, 40, 8,
		&roth_pull,
		mainpull_106 /* "EXTRA" */,
		pull_text,
		0,
		};
			static struct pull blu_fl5_pull = {
				NONEXT,
				1, 73, 87+3*CH_WIDTH, 9,
				NOCHILD,
				mainpull_107 /* "FLIP FIVE       5" */,
				pull_text,
				0,
				};
			static struct pull blu_fli_pull = {
				&blu_fl5_pull,
				1, 65, 87+3*CH_WIDTH, 9,
				NOCHILD,
				mainpull_108 /* "SEGMENT FLIP [cr]" */,
				pull_text,
				0,
				};
			static struct pull blu_loo_pull = {
				&blu_fli_pull,
				1, 57, 87+3*CH_WIDTH, 9,
				NOCHILD,
				mainpull_109 /* "LOOP SEGMENT" */,
				pull_text,
				0,
				};
			static struct pull blu_nex_pull = {
				&blu_loo_pull,
				1, 49, 87+3*CH_WIDTH, 9,
				NOCHILD,
				mainpull_110 /* "REPEAT CHANGES" */,
				pull_text,
				0,
				};
			static struct pull blu_get_pull = {
				&blu_nex_pull,
				1, 41, 87+3*CH_WIDTH, 9,
				NOCHILD,
				mainpull_111 /* "CLIP CHANGES" */,
				pull_text,
				0,
				};
			static struct pull blu_unh_pull = {
				&blu_get_pull,
				1, 33, 87+3*CH_WIDTH, 9,
				NOCHILD,
				mainpull_112 /* "ERASE GUIDES" */,
				pull_text,
				0,
				};
			static struct pull blu_his_pull = {
				&blu_unh_pull,
				1, 25, 87+3*CH_WIDTH, 9,
				NOCHILD,
				mainpull_113 /* "INSERT TWEEN" */,
				pull_text,
				0,
				};
			static struct pull blu_nxb_pull = {
				&blu_his_pull,
				1, 17, 87+3*CH_WIDTH, 9,
				NOCHILD,
				mainpull_114 /* "NEXT BLUE" */,
				pull_text,
				0,
				};
			static struct pull blu_unb_pull = {
				&blu_nxb_pull,
				1, 9, 87+3*CH_WIDTH, 9,
				NOCHILD,
				mainpull_115 /* "UNBLUE FRAME" */,
				pull_text,
				0,
				};
			static struct pull blu_blu_pull = {
				&blu_unb_pull,
				1, 1, 87+3*CH_WIDTH, 9,
				NOCHILD,
				mainpull_116 /* "BLUE FRAME" */,
				pull_text,
				0,
				};
		static struct pull rblu_pull = {
			NONEXT,
			-1-24, 8, 89+3*CH_WIDTH, 83,
			&blu_blu_pull,
			NODATA,
			pull_oblock,
			0,
			};
	static struct pull blu_pull = {
		&oth_pull,
		192, -1, 42, 8,
		&rblu_pull,
		mainpull_117 /* "TRACE" */,
		pull_text,
		0,
		};
			static struct pull cel_fil_pull = {
				NONEXT,
				1, 81, 6+63, 9,
				NOCHILD,
				mainpull_118 /* "FILES... CF" */,
				pull_text,
				0,
				};
			static struct pull cel_fre_pull = {
				&cel_fil_pull,
				1, 73, 6+63, 9,
				NOCHILD,
				mainpull_130 /* "RELEASE" */,
				pull_text,
				0,
				};
			static struct pull cel_opt_pull = {
				&cel_fre_pull,
				1, 65, 6+63, 9,
				NOCHILD,
				mainpull_120 /* "OPTIONS..." */,
				pull_text,
				0,
				};
			static struct pull cel_mas_pull = {
				&cel_opt_pull,
				1, 57, 6+63, 9,
				NOCHILD,
				mainpull_121 /* "1 COLOR" */,
				pull_text,
				0,
				};
			static struct pull cel_rot_pull = {
				&cel_mas_pull,
				1, 49, 6+63, 9,
				NOCHILD,
				mainpull_122 /* "TURN" */,
				pull_text,
				0,
				};
			static struct pull cel_str_pull = {
				&cel_rot_pull,
				1, 41, 6+63, 9,
				NOCHILD,
				mainpull_123 /* "STRETCH" */,
				pull_text,
				0,
				};
			static struct pull cel_bel_pull = {
				&cel_str_pull,
				1, 33, 6+63, 9,
				NOCHILD,
				mainpull_124 /* "BELOW" */,
				pull_text,
				0,
				};
			static struct pull cel_pas_pull = {
				&cel_bel_pull,
				1, 25, 6+63, 9,
				NOCHILD,
				mainpull_125 /* "PASTE     `" */,
				pull_text,
				0,
				};
			static struct pull cel_tra_pull = {
				&cel_pas_pull,
				1, 17, 6+63, 9,
				NOCHILD,
				mainpull_126 /* "MOVE      M" */,
				pull_text,
				0,
				};
			static struct pull cel_get_pull = {
				&cel_tra_pull,
				1, 9, 6+63, 9,
				NOCHILD,
				mainpull_127 /* "GET   [esc]" */,
				pull_text,
				0,
				};
			static struct pull cel_cli_pull = {
				&cel_get_pull,
				1, 1, 6+63, 9,
				NOCHILD,
				mainpull_128 /* "CLIP  [tab]" */,
				pull_text,
				0,
				};
		static struct pull rcel_pull = {
			NONEXT,
			-1-18, 8, 6+65, 91,
			&cel_cli_pull,
			NODATA,
			pull_oblock,
			0,
			};
	static struct pull cel_pull = {
		&blu_pull,
		156, -1, 36, 8,
		&rcel_pull,
		mainpull_129 /* "CEL" */,
		pull_text,
		0,
		};
			static struct pull alt_fre_pull = {
				NONEXT,
				1, 33, 33+3*CH_WIDTH, 9,
				NOCHILD,
				mainpull_130 /* "RELEASE" */,
				pull_text,
				0,
				};
			static struct pull alt_vie_pull = {
				&alt_fre_pull,
				1, 25, 33+3*CH_WIDTH, 9,
				NOCHILD,
				mainpull_131 /* "VIEW" */,
				pull_text,
				0,
				};
			static struct pull alt_pas_pull = {
				&alt_vie_pull,
				1, 17, 33+3*CH_WIDTH, 9,
				NOCHILD,
				mainpull_132 /* "PASTE" */,
				pull_text,
				0,
				};
			static struct pull alt_swa_pull = {
				&alt_pas_pull,
				1, 9, 33+3*CH_WIDTH, 9,
				NOCHILD,
				mainpull_133 /* "TRADE" */,
				pull_text,
				0,
				};
			static struct pull alt_cli_pull = {
				&alt_swa_pull,
				1, 1, 33+3*CH_WIDTH, 9,
				NOCHILD,
				mainpull_134 /* "CLIP" */,
				pull_text,
				0,
				};
		static struct pull ralt_pull = {
			NONEXT,
			-1-10, 8, 35+3*CH_WIDTH, 43,
			&alt_cli_pull,
			NODATA,
			pull_oblock,
			0,
			};
	static struct pull alt_pull = {
		&cel_pull,
		240, -1, 38, 8,
		&ralt_pull,
		mainpull_135 /* "SWAP" */,
		pull_text,
		0,
		};
			static struct pull fra_fil_pull = {
				NONEXT,
				1, 33+8, 51+6+6+6, 9,
				NOCHILD,
				mainpull_136 /* "FILES... PF" */,
				pull_text,
				0,
				};
			static struct pull fra_vie_pull = {
				&fra_fil_pull,
				1, 33, 51+6+6+6, 9,
				NOCHILD,
				mainpull_137 /* "View" */,
				pull_text,
				0,
				};
			static struct pull fra_sep_pull = {
				&fra_vie_pull,
				1, 25, 51+6+6+6, 9,
				NOCHILD,
				mainpull_138 /* "SEPARATE" */,
				pull_text,
				0,
				};
			static struct pull fra_set_pull = {
				&fra_sep_pull,
				1, 17, 51+6+6+6, 9,
				NOCHILD,
				mainpull_139 /* "APPLY INK" */,
				pull_text,
				0,
				};
			static struct pull fra_res_pull = {
				&fra_set_pull,
				1, 9, 51+6+6+6, 9,
				NOCHILD,
				mainpull_140 /* "RESTORE" */,
				pull_text,
				0,
				};
			static struct pull fra_cle_pull = {
				&fra_res_pull,
				1, 1, 51+6+6+6, 9,
				NOCHILD,
				mainpull_141 /* "CLEAR     X" */,
				pull_text,
				0,
				};
		static struct pull rfra_pull = {
			NONEXT,
			-1-12, 8, 53+6+6+6, 43+8,
			&fra_cle_pull,
			NODATA,
			pull_oblock,
			0,
			};
	static struct pull fra_pull = {
		&alt_pull,
		114, -1, 40, 8,
		&rfra_pull,
		mainpull_142 /* "PIC" */,
		pull_text,
		0,
		};
			static struct pull vid_fil_pull = {
				NONEXT,
				1, 49, 75, 9,
				NOCHILD,
				mainpull_143 /* "FILES...  FF" */,
				pull_text,
				0,
				};
			static struct pull vid_eff_pull = {
				&vid_fil_pull,
				1, 33, 75, 9,
				NOCHILD,
				mainpull_144 /* "EFFECTS..." */,
				pull_text,
				0,
				};
			static struct pull vid_bak_pull = {
				&vid_eff_pull,
				1, 41, 75, 9,
				NOCHILD,
				mainpull_145 /* "BACKWARDS..." */,
				pull_text,
				0,
				};
			static struct pull vid_spl_pull = {
				&vid_bak_pull,
				1, 25, 75, 9,
				NOCHILD,
				mainpull_146 /* "JOIN..." */,
				pull_text,
				0,
				};
			static struct pull vid_com_pull = {
				&vid_spl_pull,
				1, 17, 75, 9,
				NOCHILD,
				mainpull_147 /* "COMPOSITE..." */,
				pull_text,
				0,
				};
			static struct pull vid_res_pull = {
				&vid_com_pull,
				1, 9, 75, 9,
				NOCHILD,
				mainpull_148 /* "RESET" */,
				pull_text,
				0,
				};
			static struct pull vid_new_pull = {
				&vid_res_pull,
				1, 1, 75, 9,
				NOCHILD,
				mainpull_149 /* "NEW        N" */,
				pull_text,
				0,
				};
		static struct pull rvid_pull = {
			NONEXT,
			-1-6, 8, 77, 3+7*CH_HEIGHT,
			&vid_new_pull,
			NODATA,
			pull_oblock,
			0,
			};
	static struct pull vid_pull = {
		&fra_pull,
		66, -1, 46, 8,
		&rvid_pull,
		mainpull_150 /* "FLIC" */,
		pull_text,
		0,
		};
#ifdef MORPH
			static struct pull sys_mor_pull = {
				NONEXT,
				1, 1+11*CH_HEIGHT, 87, 9,
				NOCHILD,
				mainpull_163 /* "METAMORPHIC   " */,
				pull_text,
				0,
				};
#define MORPH_PULL &sys_mor_pull
#else /* MORPH */
#define MORPH_PULL NULL
#endif /* MORPH */
			static struct pull sys_qui_pull = {
				MORPH_PULL,
				1, 1+10*CH_HEIGHT, 87, 9,
				NOCHILD,
				mainpull_151 /* "QUIT         Q" */,
				pull_text,
				0,
				};
			static struct pull sys_dots_pull = {
				&sys_qui_pull,
				1, 1+9*CH_HEIGHT, 87, 9,
				NOCHILD,
				mainpull_152,
				pull_text,
				0,
				};
			static struct pull sys_tit_pull = {
				&sys_dots_pull,
				1, 1+8*CH_HEIGHT, 87, 9,
				NOCHILD,
				mainpull_153 /* "TITLING" */,
				pull_text,
				0,
				};
			static struct pull sys_ink_pull = {
				&sys_tit_pull,
				1, 1+7*CH_HEIGHT, 87, 9,
				NOCHILD,
				mainpull_154 /* "INK TYPES" */,
				pull_text,
				0,
				};
			static struct pull sys_dra_pull = {
				&sys_ink_pull,
				1, 1+6*CH_HEIGHT, 87, 9,
				NOCHILD,
				mainpull_155 /* "DRAW TOOLS" */,
				pull_text,
				0,
				};
			static struct pull sys_pal_pull = {
				&sys_dra_pull,
				1, 1+5*CH_HEIGHT, 87, 9,
				NOCHILD,
				mainpull_156 /* "PALETTE      @" */,
				pull_text,
				0,
				};
			static struct pull sys_opt_pull = {
				&sys_pal_pull,
				1, 1+4*CH_HEIGHT, 87, 9,
				NOCHILD,
				mainpull_157 /* "OPTICS       O" */,
				pull_text,
				0,
				};
			static struct pull sys_tim_pull = {
				&sys_opt_pull,
				1, 1+3*CH_HEIGHT, 87, 9,
				NOCHILD,
				mainpull_158 /* "FRAMES" */,
				pull_text,
				0,
				};
			static struct pull vid_bro_pull = {
				&sys_tim_pull,
				1, 1+2*CH_HEIGHT, 87, 9,
				NOCHILD,
				mainpull_159 /* "BROWSE FLICS" */,
				pull_text,
				0,
				};
			static struct pull sys_dotx_pull = {
				&vid_bro_pull,
				1, 1+1*CH_HEIGHT, 87, 9,
				NOCHILD,
				mainpull_160,
				pull_text,
				0,
				};
			static struct pull sys_abo_pull = {
				&sys_dotx_pull,
				1, 1+0*CH_HEIGHT, 87, 9,
				NOCHILD,
				mainpull_161 /* "ABOUT ANIMATOR" */,
				pull_text,
				0,
				};
		static struct pull rsys_pull = {
			NONEXT,
#ifdef MORPH
			0, 8, 87+2, 3+12*CH_HEIGHT,
#else /* MORPH */
			0, 8, 87+2, 3+11*CH_HEIGHT,
#endif /* MORPH */
			&sys_abo_pull,
			NODATA,
			pull_oblock,
			0,
			};
	static struct pull sys_pull = {
		&vid_pull,
		0, -1, 50, 8,
		&rsys_pull,
		mainpull_162 /* "ANIMATOR" */,
		pull_text,
		0,
		};
struct pull root_pull = {
	NONEXT,
	0, 0, 320, 8,
	&sys_pull,
	NODATA,
	pull_block,
	0,
	};


disables()
{
extern PLANEPTR mask_plane;

enable_pulls(&root_pull);
if (cel == NULL)
	{
	cel_fre_pull.disabled = 1;
	cel_mas_pull.disabled = 1;
	cel_rot_pull.disabled = 1;
	cel_str_pull.disabled = 1;
	cel_tra_pull.disabled = 1;
	cel_bel_pull.disabled = 1;
	cel_pas_pull.disabled = 1;
	}
if (fhead.frame_count < 2)
	{
	blu_fl5_pull.disabled = 1;
	blu_fli_pull.disabled = 1;
	blu_nex_pull.disabled = 1;
	blu_nxb_pull.disabled = 1;
	}
if (alt_form == NULL)
	{
	alt_fre_pull.disabled = 1;
	alt_vie_pull.disabled = 1;
	alt_pas_pull.disabled = 1;
	alt_swa_pull.disabled = 1;
	}
}

