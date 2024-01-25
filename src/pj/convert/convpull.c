/* Convpull.c - main switch for pulldowns, and relatively small routines
 * called from that switch */

#include "errcodes.h"
#include "convert.h"
#include "convpull.h"
#include "softmenu.h"
#include "ptrmacro.h"
#include "commonst.h"
#include "reqlib.h"

void about_converter(void)
/*****************************************************************************
 *
 ****************************************************************************/
{
	static char date[] = __DATE__;
	char relnum[16];

	get_relvers(relnum);
	soft_continu_box("!%s%s%.3s%.2s%.4s%.5s","conv_about", relnum,
					"", &date[0], &date[4], &date[7], __TIME__ );
}

extern long largest_frag();
extern long mem_free;
extern long init_mem_free;

void qconvert_memory()
/*****************************************************************************
 *
 ****************************************************************************/
{
	soft_continu_box("!%ld%ld%ld%s%d%d%d", "conv_status",
		mem_free, init_mem_free, largest_frag(),
		(cs.ifi.cel == NULL ? empty_str : cs.in_name),
		cs.ifi.ai.width, cs.ifi.ai.height, cs.ifi.ai.num_frames);
}

static void conv_st_pic()
/*****************************************************************************
 *
 ****************************************************************************/
{
	switch (soft_qchoice(NULL, "conv_st_pics"))
		{
		case 0: 	/* Load Neochrome */
			get_a_flic("neo.pdr", NULL, NULL);
			break;
		case 1: 	/* Load Degas */
			get_a_flic("degas.pdr", NULL, ".PI?");
			break;
		case 2: 	/* Load Degas Elite */
			get_a_flic("degas.pdr", NULL, ".PC?");
			break;
		}
}

void conv_amiga_flic()
/*****************************************************************************
 *
 ****************************************************************************/
{
	switch (soft_qchoice(NULL, "conv_amiga_flic"))
		{
		case 0:
			get_a_flic("rif.pdr", NULL, NULL);
			break;
		case 1:
			get_a_flic("anim.pdr", NULL, NULL);
			break;
		}
}

static void qwindow_size()
/*****************************************************************************
 *
 ****************************************************************************/
{
	SHORT width = vb.pencel->width;
	SHORT height = vb.pencel->height;

	switch (soft_qchoice(NULL, "!%d%d", "conv_window", width, height))
		{
		case 0: 	/* full screen */
			width = vb.screen->wndo.width;
			height = vb.screen->wndo.height;
			break;
		case 1: 	/* image size */
			width = cs.ifi.ai.width;
			height = cs.ifi.ai.height;
			break;
		case 2:
			width = 320;
			height = 200;
			break;
		case 3:
			width = 640;
			height = 400;
			break;
		case 4:
			width = 640;
			height = 480;
			break;
		case 5:
			width = 800;
			height = 600;
			break;
		case 6:
			width = 1024;
			height = 768;
			break;
		case 7: 	/* custom */
			if (clip_soft_qreq_number(&width, 4, 10000, NULL, NULL,
									  "conv_window_width"))
				clip_soft_qreq_number(&height, 4, 10000, NULL, NULL,
									  "conv_window_height");
			break;
		default:
			return;
		}
	conv_set_pencel(width, height);
}

static void qquit_convert()
{
if(soft_yes_no_box("conv_quit"))
	return_to_main(MRET_QUIT);
}

static void convert_selit(Menuhdr *mh, SHORT hitid)
/*****************************************************************************
 *
 ****************************************************************************/
{
	hide_mp();
	switch(hitid)
	{
		case CON_ABO: /* about */
			about_converter();
			break;
		case CON_MEM: /* memory */
			qconvert_memory();
			break;
		case CON_SCA: /* scale */
			qscale_menu();
			break;
		case CON_MOV: /* move */
			conv_move();
			break;
		case CON_SLI: /* slide */
			qconv_slide();
			break;
		case CON_QUI: /* quit */
			qquit_convert();
			break;
		case FLI_OTH: /* load other */
			load_other();
			break;
		case FLI_AMI: /* load amiga flic */
			conv_amiga_flic();
			break;
		case FLI_ST: /* load ST flic */
			get_a_flic("seq.pdr", NULL, NULL);
			break;
		case FLI_FLI: /* load FLI flic */
			get_a_flic(fli_pdr_name, NULL, NULL);
			break;
		case FLI_VIE: /* view flic */
			view_flic();
			break;
		case FLI_OLD: /* save old flic */
			save_a_flic(flilores_pdr_name, NULL, cs.ifi.ai.num_frames,
						conv_seek);
			break;
		case FLI_SAV: /* save PJ style flic */
			save_a_flic(fli_pdr_name, NULL, cs.ifi.ai.num_frames, conv_seek);
			break;
		case FLI_SOT:	/* save other flic */
			save_other();
			break;
		case PIC_TAR: /* load targa pic */
			get_a_flic("targa.pdr", NULL, NULL);
			break;
		case PIC_AMI: /* load amiga pic */
			get_a_flic("lbm.pdr", NULL, NULL);
			break;
		case PIC_ST: /* load ST pic */
			conv_st_pic();
			break;
		case PIC_MAC: /* load mac */
			get_a_flic("mac.pdr", NULL, NULL);
			break;
		case PIC_PCX: /* load pcx */
			get_a_flic("pcx.pdr", NULL, NULL);
			break;
		case PIC_GIF: /* load gif */
			get_a_flic(gif_pdr_name, NULL, NULL);
			break;
		case PIC_BMP: /* load bmp */
			get_a_flic("bmp.pdr", NULL, NULL);
			break;
		case PIC_TIF: /* load tiff */
			get_a_flic("tiff.pdr", NULL, NULL);
			break;
		case PIC_VIE:	/* view */
			view_pic();
			break;
		case PIC_SAG:	/* save GIF */
			save_a_pic(gif_pdr_name);
			break;
		case PIC_SAP:	/* save PCX */
			save_a_pic("pcx.pdr");
			break;
		case PIC_SAT:	/* save tiff */
			save_a_pic("tiff.pdr");
			break;
		case PIC_STA:	/* save targa */
			save_a_pic("targa.pdr");
			break;
		case PIC_SBM:	/* save bmp */
			save_a_pic("bmp.pdr");
			break;
		case EXT_SCR: /* screen size */
			return_to_main(MRET_RESIZE_SCREEN);
			break;
		case EXT_WIN: /* window size */
			qwindow_size();
			break;
		case EXT_TIL: /* tile */
			cs.no_tile = !cs.no_tile;
			conv_see_cel(cs.ifi.cel);
			break;
	}
	show_mp();
}

static int conv_dopull(Menuhdr *mh)
/*****************************************************************************
 *
 ****************************************************************************/
{
	static SHORT nocel_pulltab[] =
		{
		CON_SCA,
		CON_MOV,
		CON_SLI,
		FLI_OLD,
		FLI_SOT,
		FLI_SAV,
		FLI_VIE,
		PIC_VIE,
		PIC_SAG,
		PIC_SAP,
		PIC_SAT,
		PIC_STA,
		PIC_SBM,
		};
	Boolean nocel = (cs.ifi.cel == NULL);

	pul_xflag(mh, EXT_TIL, !cs.no_tile);
	set_pultab_disable(mh, nocel_pulltab, Array_els(nocel_pulltab),
		nocel);
	return(menu_dopull(mh));
}

Boolean convert_do_keys()
{
	if(!JSTHIT(KEYHIT))
		return(FALSE);
	switch(tolower((UBYTE)icb.inkey))
	{
		case ESCKEY:
		case 'q':
			qquit_convert();
			return(TRUE);
	}
	return(FALSE);
}

Errcode go_converter(void)
/*****************************************************************************
 *
 ****************************************************************************/
{
	Errcode err;
	Menuhdr tpull;

		if ((err = load_soft_pull(&tpull, 0, "conv_pull", 0,
								  convert_selit, conv_dopull)) >= Success)
		{
			err = do_menuloop(vb.screen,NULL,NULL,&tpull,convert_do_keys);
			smu_free_pull(&tpull);
		}
		err = softerr(err,"conv_pull");
		return(err);
}

