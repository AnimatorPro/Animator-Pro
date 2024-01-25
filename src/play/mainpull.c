/* generated with makepull */
#include "jimk.h"
#include "flicmenu.h"
#include "mainpull.str"


			struct pull fil_lob_pull = {
				NONEXT,
				1, 17, 69+6*3, 9,
				NOCHILD,
				mainpull_100 /* "Script Load..." */,
				pull_text,
				0,
				};
			struct pull fil_log_pull = {
				&fil_lob_pull,
				1, 9, 69+18, 9,
				NOCHILD,
				mainpull_101 /* "Gif Load..." */,
				pull_text,
				0,
				};
			struct pull fil_loa_pull = {
				&fil_log_pull,
				1, 1, 69+18, 9,
				NOCHILD,
				mainpull_102 /* "Fli Load..." */,
				pull_text,
				0,
				};
		struct pull rfil_pull = {
			NONEXT,
			-1, 8, 71+6*3, 27,
			&fil_loa_pull,
			NODATA,
			pull_oblock,
			0,
			};
	struct pull fil_pull = {
		NONEXT,
		87, -1, 42, 8,  /* was 60 now 87 */
		&rfil_pull,
		mainpull_103 /* "FILE" */,
		pull_text,
		0,
		};
			struct pull sys_qui_pull = {
				NONEXT,
				1, 17, 39, 9,
				NOCHILD,
				mainpull_104 /* "QUIT" */,
				pull_text,
				0,
				};
			struct pull sys_dot_pull = {
				&sys_qui_pull,
				1, 9, 39, 9,
				NOCHILD,
				mainpull_105 /* "------" */,
				pull_text,
				0,
				};
			struct pull sys_abo_pull = {
				&sys_dot_pull,
				1, 1, 39, 9,
				NOCHILD,
				mainpull_106 /* "ABOUT" */,
				pull_text,
				0,
				};
		struct pull rsys_pull = {
			NONEXT,
			-1, 8, 41, 27,
			&sys_abo_pull,
			NODATA,
			pull_oblock,
			0,
			};
	struct pull sys_pull = {
		&fil_pull,
		6, -1, 81, 8,  /* was 54 now 81 -- added 27 */
		&rsys_pull,
		mainpull_107 /* "PLAYER" */,
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
