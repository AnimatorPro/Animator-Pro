
/* palpull.c - Data structures for the drop-downs used in the palette editor.
	Also code for disabling the bit of the drop downs that user can't use
	currently.  Has the big switch that decides what to do on palette
	drop-down selection too. */

/* generated with makepull */
#include "jimk.h"
#include "flicmenu.h"
#include "palpull.str"


			static struct pull val_ble_pull = {
				NONEXT,
				1+0*CH_WIDTH, 1+8*CH_HEIGHT, 3+8*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				palpull_100 /* "blend" */,
				pull_text,
				0,
				};
			static struct pull val_pas_pull = {
				&val_ble_pull,
				1+0*CH_WIDTH, 1+7*CH_HEIGHT, 3+8*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				palpull_101 /* "paste" */,
				pull_text,
				0,
				};
			static struct pull val_cut_pull = {
				&val_pas_pull,
				1+0*CH_WIDTH, 1+6*CH_HEIGHT, 3+8*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				palpull_102 /* "cut" */,
				pull_text,
				0,
				};
			static struct pull val_def_pull = {
				&val_cut_pull,
				1+0*CH_WIDTH, 1+5*CH_HEIGHT, 3+8*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				palpull_103 /* "default" */,
				pull_text,
				0,
				};
			static struct pull val_use_pull = {
				&val_def_pull,
				1+0*CH_WIDTH, 1+4*CH_HEIGHT, 3+8*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				palpull_104 /* "use cel" */,
				pull_text,
				0,
				};
			static struct pull val_neg_pull = {
				&val_use_pull,
				1+0*CH_WIDTH, 1+3*CH_HEIGHT, 3+8*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				palpull_105 /* "negative" */,
				pull_text,
				0,
				};
			static struct pull val_tin_pull = {
				&val_neg_pull,
				1+0*CH_WIDTH, 1+2*CH_HEIGHT, 3+8*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				palpull_106 /* "tint" */,
				pull_text,
				0,
				};
			static struct pull val_ram_pull = {
				&val_tin_pull,
				1+0*CH_WIDTH, 1+1*CH_HEIGHT, 3+8*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				palpull_107 /* "ramp" */,
				pull_text,
				0,
				};
			static struct pull val_squ_pull = {
				&val_ram_pull,
				1+0*CH_WIDTH, 1+0*CH_HEIGHT, 3+8*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				palpull_108 /* "squeeze" */,
				pull_text,
				0,
				};
		static struct pull rval_pull = {
			NONEXT,
			-1+0*CH_WIDTH, 0+1*CH_HEIGHT, 5+8*CH_WIDTH, 3+9*CH_HEIGHT, 
			&val_squ_pull,
			NODATA,
			pull_oblock,
			0,
			};
	static struct pull val_pull = {
		NONEXT,
		0+39*CH_WIDTH, -1+0*CH_HEIGHT, 0+8*CH_WIDTH, 0+1*CH_HEIGHT, 
		&rval_pull,
		palpull_109 /* "value" */,
		pull_text,
		0,
		};
			static struct pull mat_tra_pull = {
				NONEXT,
				1+0*CH_WIDTH, 1+4*CH_HEIGHT, 3+14*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				palpull_110 /* "trade clusters" */,
				pull_text,
				0,
				};
			static struct pull mat_cyc_pull = {
				&mat_tra_pull,
				1+0*CH_WIDTH, 1+3*CH_HEIGHT, 3+14*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				palpull_111 /* "cycle" */,
				pull_text,
				0,
				};
			static struct pull mat_thr_pull = {
				&mat_cyc_pull,
				1+0*CH_WIDTH, 1+2*CH_HEIGHT, 3+14*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				palpull_112 /* "gradients" */,
				pull_text,
				0,
				};
			static struct pull mat_spe_pull = {
				&mat_thr_pull,
				1+0*CH_WIDTH, 1+1*CH_HEIGHT, 3+14*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				palpull_113 /* "spectrums" */,
				pull_text,
				0,
				};
			static struct pull mat_lum_pull = {
				&mat_spe_pull,
				1+0*CH_WIDTH, 1+0*CH_HEIGHT, 3+14*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				palpull_114 /* "luma sort" */,
				pull_text,
				0,
				};
		static struct pull rmat_pull = {
			NONEXT,
			-1+0*CH_WIDTH, 0+1*CH_HEIGHT, 5+14*CH_WIDTH, 3+5*CH_HEIGHT, 
			&mat_lum_pull,
			NODATA,
			pull_oblock,
			0,
			};
	static struct pull mat_pull = {
		&val_pull,
		0+29*CH_WIDTH, -1+0*CH_HEIGHT, 0+9*CH_WIDTH, 0+1*CH_HEIGHT, 
		&rmat_pull,
		palpull_115 /* "arrange" */,
		pull_text,
		0,
		};
			static struct pull clu_rev_pull = {
				NONEXT,
				1+0*CH_WIDTH, 1+7*CH_HEIGHT, 3+13*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				palpull_116 /* "reverse" */,
				pull_text,
				0,
				};
			static struct pull clu_pin_pull = {
				&clu_rev_pull,
				1+0*CH_WIDTH, 1+6*CH_HEIGHT, 3+13*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				palpull_117 /* "ping-pong" */,
				pull_text,
				0,
				};
			static struct pull clu_inv_pull = {
				&clu_pin_pull,
				1+0*CH_WIDTH, 1+5*CH_HEIGHT, 3+13*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				palpull_118 /* "invert" */,
				pull_text,
				0,
				};
			static struct pull clu_nea_pull = {
				&clu_inv_pull,
				1+0*CH_WIDTH, 1+4*CH_HEIGHT, 3+13*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				palpull_119 /* "near colors" */,
				pull_text,
				0,
				};
			static struct pull clu_fin_pull = {
				&clu_nea_pull,
				1+0*CH_WIDTH, 1+3*CH_HEIGHT, 3+13*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				palpull_120 /* "find ramp" */,
				pull_text,
				0,
				};
			static struct pull clu_box_pull = {
				&clu_fin_pull,
				1+0*CH_WIDTH, 1+2*CH_HEIGHT, 3+13*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				palpull_121 /* "line cluster" */,
				pull_text,
				0,
				};
			static struct pull clu_unu_pull = {
				&clu_box_pull,
				1+0*CH_WIDTH, 1+1*CH_HEIGHT, 3+13*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				palpull_122 /* "unused colors" */,
				pull_text,
				0,
				};
			static struct pull clu_get_pull = {
				&clu_unu_pull,
				1+0*CH_WIDTH, 1+0*CH_HEIGHT, 3+13*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				palpull_123 /* "get cluster" */,
				pull_text,
				0,
				};
		static struct pull rclu_pull = {
			NONEXT,
			-1+0*CH_WIDTH, 0+1*CH_HEIGHT, 5+13*CH_WIDTH, 3+8*CH_HEIGHT, 
			&clu_get_pull,
			NODATA,
			pull_oblock,
			0,
			};
	static struct pull clu_pull = {
		&mat_pull,
		0+19*CH_WIDTH, -1+0*CH_HEIGHT, 0+10*CH_WIDTH, 0+1*CH_HEIGHT, 
		&rclu_pull,
		palpull_124 /* "cluster" */,
		pull_text,
		0,
		};
			static struct pull pal_fi_pull = {
				NONEXT,
				1+0*CH_WIDTH, 1+4*CH_HEIGHT, 3+12*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				palpull_125 /* " files..." */,
				pull_text,
				0,
				};
			static struct pull pal_me_pull = {
				&pal_fi_pull,
				1+0*CH_WIDTH, 1+3*CH_HEIGHT, 3+12*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				palpull_126 /* " menu colors" */,
				pull_text,
				0,
				};
			static struct pull pal_one_pull = {
				&pal_me_pull,
				1+0*CH_WIDTH, 1+2*CH_HEIGHT, 3+12*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				palpull_127 /* " one palette" */,
				pull_text,
				0,
				};
			static struct pull pal_cy_pull = {
				&pal_one_pull,
				1+0*CH_WIDTH, 1+1*CH_HEIGHT, 3+12*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				palpull_128 /* " cycle draw" */,
				pull_text,
				0,
				};
			static struct pull pal_re_pull = {
				&pal_cy_pull,
				1+0*CH_WIDTH, 1+0*CH_HEIGHT, 3+12*CH_WIDTH, 1+1*CH_HEIGHT, 
				NOCHILD,
				palpull_129 /* " restore" */,
				pull_text,
				0,
				};
		static struct pull rpal_pull = {
			NONEXT,
			-1+0*CH_WIDTH, 0+1*CH_HEIGHT, 5+12*CH_WIDTH, 3+5*CH_HEIGHT, 
			&pal_re_pull,
			NODATA,
			pull_oblock,
			0,
			};
	struct pull pal_pull = {
		&clu_pull,
		0+9*CH_WIDTH, -1+0*CH_HEIGHT, 0+10*CH_WIDTH, 0+1*CH_HEIGHT, 
		&rpal_pull,
		palpull_130 /* "palette" */,
		pull_text,
		0,
		};

pal_disables()
{
pal_me_pull.disabled = sys5;
val_use_pull.disabled = (cel == NULL);
val_ble_pull.disabled = val_pas_pull.disabled = !jexists(cclip_name);
}

pal_checks()
{
xonflag(&pal_cy_pull, vs.cycle_draw);
}

static char *op_lines[] = {
	palpull_131 /* "Make all frames use one color map?" */,
	palpull_132 /* "Takes a while...." */,
	NULL,
	};


static
qone_palette()
{
if (yes_no_box(op_lines))
	one_palette();
}

palette_selit(menu, sel)
WORD menu, sel;
{
switch (menu)
	{
	case 0:	/* COLORS */
		switch (sel)
			{
			case 0:
				crestore();
				break;
			case 1:  /* Cycle Draw */
				vs.cycle_draw = !vs.cycle_draw;
				vs.cdraw_ix = 0;
				pal_checks();
				break;
			case 2:	/* one palette */
				hide_mp();
				qone_palette();
				draw_mp();
				break;
			case 3:	/* menu colors */
				get_menu_colors();
				break;
			case 4:
				hide_mp();
				go_files(3);
				draw_mp();
				break;
			}
		break;
	case 1:		/* CLUSTER */
		switch (sel)
			{
			case 0:	/* get range */
				qselect_bundle();
				break;
			case 1:	/* unused colors */
				cunused();
				break;
			case 2: /* line cluster */
				cluster_line();
				break;
			case 3:	/* find ramp */
				find_ramp();
				break;
			case 4:	/* near colors */
				cclose();
				break;
			case 5: /* invert cluster */
				cluster_invert();
				break;
			case 6: /* ping-pong */
				ping_cluster();
				break;
			case 7: /* reverse cluster */
				cluster_reverse();
				break;
			}
		break;
	case 2:  /* Matrix */
		switch (sel)
			{
			case 0:
				csort();
				break;
			case 1:
				cspec();
				break;
			case 2:
				cthread();
				break;
			case 3: /* Cycle Palette */
				ccycle();
				break;
			case 4:  /* swap cluster */
				cl_swap();
				break;
			}
		break;
	case 3:		/* VALUE */
		switch (sel)
			{
			case 0:
				cpack();
				break;
			case 1: /* force ramp */
				force_ramp();
				break;
			case 2:
				ctint();
				break;
			case 3:	/* negative */
				cneg();
				break;
			case 4:
				cuse_cel();
				break;
			case 5:
				cdefault();
				break;
			case 6: /* cut */
				cl_cut();
				break;
			case 7: /* paste */
				cl_paste();
				break;
			case 8: /* blend */
				cl_blend();
				break;
			}
		break;
	}
pal_disables();
}

