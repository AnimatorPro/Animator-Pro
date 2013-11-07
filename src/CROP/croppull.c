/* croppull.c - The data structures and disables() routine for the
   crop drop-down menus. */

/* generated with makepull */
#include "jimk.h"
#include "flicmenu.h"
#include "crop.h"
#include "croppull.str"


			struct pull pic_sgi_pull = {
				NONEXT,
				1+0*CH_WIDTH, 1+7*CH_HEIGHT, 3+14*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				croppull_100 /* "Save Gif" */,
				pull_text,
				0,
				};
			struct pull pic_dots_pull = {
				&pic_sgi_pull,
				1+0*CH_WIDTH, 1+6*CH_HEIGHT, 3+14*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				croppull_101 /* "--------------" */,
				pull_text,
				1,
				};
			struct pull pic_lgi_pull = {
				&pic_dots_pull,
				1+0*CH_WIDTH, 1+5*CH_HEIGHT, 3+14*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				croppull_102 /* "Load GIF" */,
				pull_text,
				0,
				};
			struct pull pic_lpc_pull = {
				&pic_lgi_pull,
				1+0*CH_WIDTH, 1+4*CH_HEIGHT, 3+14*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				croppull_103 /* "Load PCX" */,
				pull_text,
				0,
				};
			struct pull pic_lma_pull = {
				&pic_lpc_pull,
				1+0*CH_WIDTH, 1+3*CH_HEIGHT, 3+14*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				croppull_104 /* "Load Macintosh" */,
				pull_text,
				0,
				};
			struct pull pic_lst_pull = {
				&pic_lma_pull,
				1+0*CH_WIDTH, 1+2*CH_HEIGHT, 3+14*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				croppull_112 /* "Load ST" */,
				pull_text,
				0,
				};
			struct pull pic_lam_pull = {
				&pic_lst_pull,
				1+0*CH_WIDTH, 1+1*CH_HEIGHT, 3+14*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				croppull_113 /* "Load AMIGA" */,
				pull_text,
				0,
				};
			struct pull pic_lta_pull = {
				&pic_lam_pull,
				1+0*CH_WIDTH, 1+0*CH_HEIGHT, 3+14*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				croppull_107 /* "Load TARGA" */,
				pull_text,
				0,
				};
		struct pull rpic_pull = {
			NONEXT,
			-1+0*CH_WIDTH, 0+1*CH_HEIGHT, 5+14*CH_WIDTH, 3+8*CH_HEIGHT, 
			&pic_lta_pull,
			NODATA,
			pull_oblock,
			0,
			};
	struct pull pic_pull = {
		NONEXT,
		0+21*CH_WIDTH, -1+0*CH_HEIGHT, 0+7*CH_WIDTH, 0+1*CH_HEIGHT, 
		&rpic_pull,
		croppull_108 /* "PIC" */,
		pull_text,
		0,
		};
			struct pull fli_sfl_pull = {
				NONEXT,
				1+0*CH_WIDTH, 1+4*CH_HEIGHT, 3+14*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				croppull_109 /* "Save FLIC" */,
				pull_text,
				0,
				};
			struct pull fli_vfl_pull = {
				&fli_sfl_pull,
				1+0*CH_WIDTH, 1+3*CH_HEIGHT, 3+14*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				croppull_110 /* "View" */,
				pull_text,
				0,
				};
			struct pull fli_lfl_pull = {
				&fli_vfl_pull,
				1+0*CH_WIDTH, 1+2*CH_HEIGHT, 3+14*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				croppull_111 /* "Load FLIC" */,
				pull_text,
				0,
				};
			struct pull fli_lst_pull = {
				&fli_lfl_pull,
				1+0*CH_WIDTH, 1+1*CH_HEIGHT, 3+14*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				croppull_112 /* "Load ST" */,
				pull_text,
				0,
				};
			struct pull fli_lam_pull = {
				&fli_lst_pull,
				1+0*CH_WIDTH, 1+0*CH_HEIGHT, 3+14*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				croppull_113 /* "Load AMIGA" */,
				pull_text,
				0,
				};
		struct pull rfli_pull = {
			NONEXT,
			-1+0*CH_WIDTH, 0+1*CH_HEIGHT, 5+14*CH_WIDTH, 3+5*CH_HEIGHT, 
			&fli_lam_pull,
			NODATA,
			pull_oblock,
			0,
			};
	struct pull fli_pull = {
		&pic_pull,
		0+13*CH_WIDTH, -1+0*CH_HEIGHT, 0+8*CH_WIDTH, 0+1*CH_HEIGHT, 
		&rfli_pull,
		croppull_114 /* "FLIC" */,
		pull_text,
		0,
		};
			struct pull cro_qui_pull = {
				NONEXT,
				1+0*CH_WIDTH, 1+6*CH_HEIGHT, 3+9*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				croppull_115 /* "Quit" */,
				pull_text,
				0,
				};
			struct pull cro_dot_pull = {
				&cro_qui_pull,
				1+0*CH_WIDTH, 1+5*CH_HEIGHT, 3+9*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				croppull_116 /* "---------" */,
				pull_text,
				1,
				};
			struct pull cro_sli_pull = {
				&cro_dot_pull,
				1+0*CH_WIDTH, 1+4*CH_HEIGHT, 3+9*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				croppull_117 /* "Slide" */,
				pull_text,
				0,
				};
			struct pull cro_mov_pull = {
				&cro_sli_pull,
				1+0*CH_WIDTH, 1+3*CH_HEIGHT, 3+9*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				croppull_118 /* "Move" */,
				pull_text,
				0,
				};
			struct pull cro_sca_pull = {
				&cro_mov_pull,
				1+0*CH_WIDTH, 1+2*CH_HEIGHT, 3+9*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				croppull_119 /* "Scale" */,
				pull_text,
				0,
				};
			struct pull cro_info_pull = {
				&cro_sca_pull,
				1+0*CH_WIDTH, 1+1*CH_HEIGHT, 3+9*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				croppull_120 /* "Memory" */,
				pull_text,
				0,
				};
			struct pull cro_abo_pull = {
				&cro_info_pull,
				1+0*CH_WIDTH, 1+0*CH_HEIGHT, 3+9*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				croppull_121 /* "About" */,
				pull_text,
				0,
				};
		struct pull rcro_pull = {
			NONEXT,
			-1+0*CH_WIDTH, 0+1*CH_HEIGHT, 5+9*CH_WIDTH, 3+7*CH_HEIGHT, 
			&cro_abo_pull,
			NODATA,
			pull_oblock,
			0,
			};
	struct pull cro_pull = {
		&fli_pull,
		0+1*CH_WIDTH, -1+0*CH_HEIGHT, 0+11*CH_WIDTH, 0+1*CH_HEIGHT, 
		&rcro_pull,
		croppull_122 /* "CONVERTER" */,
		pull_text,
		0,
		};
struct pull root_pull = {
	NONEXT,
	0+0*CH_WIDTH, 0+0*CH_HEIGHT, 2+53*CH_WIDTH, 0+1*CH_HEIGHT, 
	&cro_pull,
	NODATA,
	pull_block,
	0,
	};

pull_disables()
{
pic_sgi_pull.disabled =  fli_vfl_pull.disabled = fli_sfl_pull.disabled 
	= (intype == NONE);
cro_sli_pull.disabled = 
	cro_mov_pull.disabled = cro_sca_pull.disabled = !is_still(intype);
}

