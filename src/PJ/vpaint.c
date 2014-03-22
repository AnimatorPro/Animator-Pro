/* vpaint.c - handle high level user button pushes.  Main pull-down
   interpreter switch and main keyboard interpreter switch.  Implementations
   of many of routines called by above.  The first layer under main(). */

#include "jimk.h"
#include "a3d.h"
#include "alt.h"
#include "auto.h"
#include "broadcas.h"
#include "commonst.h"
#include "errcodes.h"
#include "fli.h"
#include "ftextf.h"
#include "homepul.h"
#include "input.h"
#include "menus.h"
#include "picdrive.h"
#include "softmenu.h"
#include "textedit.h"

#define UNSAVE_BUFSIZ 80

static char *unsaved_string(char *buf);
static void qquit(void);

static void get_color(void)
{
	check_input(MMOVE);
	update_ccolor(pj_get_dot(vb.screen->viscel,icb.cx,icb.cy));
}
static void qreset_seq(void)
{
char buf[UNSAVE_BUFSIZ];
static char *keys[] = { "ask", "y", "size", "n", NULL };

	switch(soft_multi_box(keys, "!%d%d%s", "reset_flic",
							   vb.pencel->width,
							   vb.pencel->height,
							   unsaved_string(buf)))
	{
		case 1:
			return_to_main(RESET_DEFAULT_FLX);
			break;
		case 2:
			return_to_main(RESET_NEW_SIZE);
			break;
		default:
			break;
	}
	return;
}
static void qnew_flx(void)
{
char buf[UNSAVE_BUFSIZ];
static char *keys[] = { "ask", "y", "size", "n" , NULL};

	switch(soft_multi_box(keys, "!%d%d%s", "new_flic",
								 vb.pencel->width,
								 vb.pencel->height,
								 unsaved_string(buf)))
	{
		case 1:
			unzoom();
			flx_clear_olays();
			save_undo();
			init_seq();
			empty_newflx();
			flx_draw_olays();
			rezoom();
			break;
		case 2:
			return_to_main(KILL_NEW_SIZE);
			break;
		default:
			break;
	}
	return;
}

void qload_mask(void)
{
char *title;
char buf[50];

	if ((title = vset_get_filename(stack_string("load_msk",buf),
								  ".MSK", load_str,
								  MASK_PATH,NULL,0))!=NULL)
	{
		unzoom();
		cant_load(load_the_mask(title),title);
		rezoom();
	}
}

void qsave_mask(void)
{
char *title;
char buf[50];

	if (mask_rast == NULL)
		return;
	if ((title = vset_get_filename(stack_string("save_msk",buf),
								  ".MSK", save_str,
								  MASK_PATH,NULL,1))!=NULL)
	{
		unzoom();
		if (overwrite_old(title) )
		{
			save_the_mask(title);
		}
		rezoom();
	}
}
void qload(void)
{
char suffi[PDR_SUFFI_SIZE*2 +10];
char hailing[100];
char ss[50];
char buf[UNSAVE_BUFSIZ];
char *path;
extern char *get_pictype_suffi(), *get_fliload_suffi();

	if (!confirm_dirty_load())
		return;

	get_fliload_suffi(suffi, FALSE);

	sprintf(hailing, "%s  %s", stack_string("load_fli",ss),
							   unsaved_string(buf));

	if((path = vset_get_filename(hailing, suffi, load_str,
								FLI_PATH,NULL,0))!=NULL)
	{
		resize_load_fli(path);
	}
}

Errcode load_the_pic(char *title)
{
	return(load_any_picture(title, vb.pencel));
}

void qload_pic(void)
{
char *title;
char buf[50];
extern char *get_pictype_suffi();

	if ((title = vset_get_filename(stack_string("load_pic", buf),
								  get_pictype_suffi(),
								  load_str, PIC_PATH,NULL,0))!=NULL)
	{
		unzoom();
		save_undo();
		load_the_pic(title);
		see_cmap();
		dirties();
		rezoom();
	}
}

void qsave_pic(void)
{
Errcode err;
char title[61];
char *picpath;
char suffi[PDR_SUFFI_SIZE];
int sph_size;
char sph_buf[50];


	stack_string("save_pic", sph_buf);
	sph_size = strlen(sph_buf)+1;
	strcpy(title,sph_buf);
	if((err = get_picsave_info(suffi, (title + (sph_size-1)),
							   sizeof(title) - sph_size)) < Success)
	{
		return;
	}

	if ((picpath = vset_get_filename(title, suffi, save_str,
									PIC_PATH,NULL,1))!=NULL)
	{
		if (overwrite_old(picpath) )
		{
			unzoom();
			soft_put_wait_box("!%s", "wait_save", picpath);
			err = save_current_pictype(picpath,vb.pencel);
			softerr(err,"!%s","cant_save",picpath);
			rezoom();
		}
	}
}


void toggle_cel_opt(int mode)
{
USHORT changes;

	switch(mode)
	{
		case 0:
			vs.zero_clear = !vs.zero_clear;
			changes = RSTAT_ZCLEAR;
			break;
		case 1:
			vs.fit_colors = !vs.fit_colors;
			changes = RSTAT_CFIT;
			break;
		case 2:
			vs.render_under = !vs.render_under;
			changes = RSTAT_UNDER;
			break;
		case 3:
			vs.render_one_color = !vs.render_one_color;
			changes = RSTAT_ONECOL;
			break;
	}
	do_rmode_redraw(changes);
}




#ifdef TESTING
static void tram_dir()
{
int ipos = 0;
char picked[20];
Names *nl;
Errcode err;


if ((err = rget_dir(&nl)) < Success)
	softerr(err, "ram_dir");
else
	{
	picked[0] = 0;
	qscroller(picked, "Ram directory", nl, 10, &ipos);
	}
rfree_dir(&nl);
}
#endif /* TESTING */

extern void flip_range(), flip5(), get_color(), first_frame(),
	next_frame(), prev_frame(), menu_doredo(), cut_out_cel(), clip_cel(),
	paste_the_cel(), move_the_cel(), qsave(), qload(), qnew_flx(), qquit(),
	quse_poco(), test(), insert_a_frame(), kill_a_frame(),
	palette(), ktoggle_zoom();

static void flix_first_frame()
{
	mini_first_frame(&flxtime_data);
}
static void flix_next_frame()
{
	mini_next_frame(&flxtime_data);
}
static void flix_prev_frame()
{
	mini_prev_frame(&flxtime_data);
}
static void flix_playit()
{
	mini_playit(&flxtime_data);
}

#ifdef TESTING
static void tog_debug()
{
	debug = !debug;
	if(debug)
		boxf("Debug flag = TRUE");
	else
		boxf("Debug flag = FALSE");
}
#endif

static void tog_zoom()
{
	if(zoom_disabled())
		return;
	hide_mp();
	ktoggle_zoom();
	show_mp();
}

static void toggle_render_under()
{
	toggle_cel_opt(2);
}

static void toggle_one_color()
{
	toggle_cel_opt(3);
}


static Keyequiv header_keys[] = {
	{ "ztogl", tog_zoom, KE_NOHIDE, 'z' },
#ifdef SLUFFED
	{ "status", status, KE_HIDE, '?' },
#endif /* SLUFFED */
	{ "qpal", palette, KE_HIDE, '@' },
	{ "color", get_color, KE_NOHIDE, FKEY2 },
	{ "prevf", flix_prev_frame, KE_NOHIDE, LARROW },
	{ "nextf", flix_next_frame, KE_NOHIDE, RARROW },
	{ "play", flix_playit, KE_HIDE, DARROW },
	{ "frame1", flix_first_frame, KE_NOHIDE, UARROW },
	{ "over_under", toggle_render_under, KE_HIDE, 'v'},
	{ "one_color", toggle_one_color, KE_HIDE, '1'},
#ifdef TESTING
	{ "debug", tog_debug, KE_NOHIDE, '!' },
#endif
};

Boolean common_header_keys(void)
{
	return(do_keyequiv(icb.inkey,header_keys,Array_els(header_keys)));
}

#ifdef TESTING
static void eatk()
{
static SHORT kb64 = 1;
static void *mem = NULL;

	pj_gentle_free(mem);
	mem = NULL;
	while(!mem)
	{
		if(qreq_number(&kb64,1,100,"How much mem times 64k to eat?"))
		{
			mem = begmem(((LONG)kb64) * 0x0000FFFFL);
		}
		else
			break;
	}
	return;
}
static void plus_trd()
{
	trd_compact(0L);
}
static void minus_trd()
{
	trd_compact(1024L*1024L*1024L);
}
#endif /* TESTING */
static void tog_pen()
{
extern Button sh1_brush_sel;
	toggle_pen(&sh1_brush_sel);
}

static void home_help()
{
	Errcode err;
	Smu_strings s;
	Names *help_strings;

	if ((err = smu_get_strings(&smu_sm, "home_help", &s)) >= Success)
		{
		if ((help_strings = array_to_names(s.strings, s.count)) != NULL)
			{
			go_driver_scroller(s.strings[0], help_strings->next, NULL
			, NOFUNC, NOFUNC, NULL, NULL);
			free_slist(help_strings);
			}
		smu_free_strings(&s);
		}
}

static void toggle_dither()
{
	vl.ink->dither = !vl.ink->dither;
}

static void toggle_key_clear()
{
	vs.zero_clear = !vs.zero_clear;
	do_rmode_redraw(RSTAT_ZCLEAR);
}

static void toggle_two_color()
{
	vs.color2 = !vs.color2;
}

static Keyequiv home_keys[] = {

#define UNDO_KE &home_keys[0]
	{ "help", home_help,    KE_NOHIDE, FKEY1 },
	{ "undo", menu_doundo,  KE_NOHIDE, '\b' },
	{ "togm", toggle_menu,  KE_NOHIDE, ' ' },
	{ "togb", tog_pen,      KE_NOHIDE, 'b' },
	{ "clrp", clear_pic,    KE_NOHIDE, 'x' },
	{ "redo", menu_doredo,  KE_HIDE, 'r' },
	{ "flp5", flip5,        KE_HIDE, '5' },
	{ "flpr", flip_range,   KE_HIDE, '\r' },
	{ "celcut", cut_out_cel,    KE_HIDE, 'g' },
	{ "celclp", clip_cel,   KE_HIDE, '\t' },
	{ "celpas", paste_the_cel,KE_HIDE, '`' },
	{ "celmov", move_the_cel, KE_HIDE, 'm' },
	{ "qload", qload,       KE_HIDE, 'l' },
	{ "qnew", qnew_flx,     KE_HIDE, 'n' },
	{ "quit", qquit,        KE_HIDE, 'q' },
	{ "quit2", qquit,       KE_HIDE, ESCKEY },
#ifdef WITH_POCO
	{ "pouse", quse_poco,   KE_HIDE, 'u' },
#endif /* WITH_POCO */
	{ "optic", go_ado,      KE_HIDE, 'o' },
	{ "qinsf", insert_a_frame,  KE_HIDE, INSERTKEY },
	{ "qkillf", kill_a_frame,   KE_HIDE, DELKEY },
	{ "dither", toggle_dither, KE_HIDE, 'd'},
	{ "key_color", toggle_key_clear, KE_HIDE, 'k'},
	{ "two_color", toggle_two_color, KE_HIDE, '2'},
#ifdef TESTING
	{ "test", test,         KE_HIDE, '/' },
	{ "eatk", eatk,         KE_NOHIDE, '\'' },
	{ "trdp", plus_trd,     KE_NOHIDE, '+' },
	{ "trdm", minus_trd,    KE_NOHIDE, '-' },
	{ "rdir", tram_dir,     KE_NOHIDE, '=' },
#endif /* TESTING */
};

Errcode load_home_keys()
/* note this also loads common header keys, since all menus are sub to home
 *menu this works */
{
Errcode err;

	if((err = load_key_equivs("home_keys", header_keys,
							   Array_els(header_keys))) < Success)
	{
		return(err);
	}
	return(load_key_equivs("home_keys", home_keys, Array_els(home_keys)));
}
Boolean hit_undo_key()
{
	return(hit_keyequiv(UNDO_KE, icb.inkey));
}

Boolean home_dokeys(void)
/* returns 0 if input unused 1 if used */
{
	if(!JSTHIT(KEYHIT))
		return(check_toggle_menu());

	if(common_header_keys())
		return(TRUE);

	return(do_keyequiv(icb.inkey,home_keys,Array_els(home_keys)));
}

static void pixel_menu(void)
{
switch (soft_qchoice(NULL, "effects"))
	{
	case 0:
		auto_shrink();
		break;
	case 1:
		auto_expand();
		break;
	case 2:
		crop_video();
		break;
	case 3:
		auto_trails();
		break;
	case 4:
		quantize();
		break;
	case 5:
		auto_engrave();
		break;
	case 6:
		auto_dither();
		break;
	case 7:
		greys_only();
		break;
	case 8: /* blue numbers */
		auto_blue_nums();
		break;
	default:	/* cancel */
		break;
	}
}


void view_frame(void)
{
hide_mp();
hide_mouse();
wait_click();
show_mouse();
show_mp();
}

static void v12(void)
{
	swap_pencels(vb.pencel, vl.alt_cel);
	see_cmap();
	zoom_it();
}

static void view_alt(void)
{
	if (vl.alt_cel != NULL)
	{
		v12();
		hide_mouse();
		wait_click();
		show_mouse();
		v12();
	}
}

static char *unsaved_string(char *buf)
{
char ss[50];

	if(!dirty_file)
		buf[0] = 0;
	else
	{
		stack_string("dirty_file",ss);
		snftextf(buf, UNSAVE_BUFSIZ, "!%d", stack_string("dirty_file", ss),
				dirty_strokes );
	}
	return(buf);
}

Boolean confirm_dirty_load()
{
char buf[UNSAVE_BUFSIZ];

	if(dirty_file)
		if (!soft_yes_no_box("!%s", "load_new", unsaved_string(buf)))
			return(FALSE);
	return(TRUE);
}


static void qquit(void)
{
char buf[UNSAVE_BUFSIZ];
static char *keys[] = {
	"ask",
	"y",
	"x",
	"n",
	NULL
};

	switch(soft_multi_box(keys, "!%s", "exit_pj", unsaved_string(buf)))
	{
		case 1:
			return_to_main(EXIT_SYSTEM);
			break;
		case 2:
			return_to_main(QUIT_SYSTEM);
		default:
			break;
	}
}

void main_selit(Menuhdr *mh, SHORT hitid)
{
(void)mh;

hide_mp();
if (hitid > POC_DOT_PUL && hitid <= POC_DOT_PUL+10) /* poco call */
	{
#ifdef WITH_POCO
	run_pull_poco(mh,hitid);
#endif /* WITH_POCO */
	}
else
	{
	switch (hitid)
		{
		case ANI_ABO_PUL:
			about();
			break;
		case ANI_BRO_PUL:
			go_browse();
			break;
		case ANI_FRA_PUL:
			do_time_menu();
			break;
		case ANI_OPT_PUL: /* Optics */
			go_ado();
			break;
		case ANI_PAL_PUL:  /* Palette */
			palette();
			break;
		case ANI_TOO_PUL:  /* tools */
			qtools();
			break;
		case ANI_INK_PUL: /* inks */
			qinks();
			break;
		case ANI_TIT_PUL: /* titling */
			do_title_menu();
			break;
		case ANI_TWE_PUL:
			tween_menu(TRUE);
			break;
		case ANI_DOS_PUL:
			/* shell_out_to_dos(); */
			break;
		case ANI_QUI_PUL:	/* quit */
			qquit();
			break;
		case FLI_NEW_PUL:
			qnew_flx();
			break;
		case FLI_RES_PUL:
			qreset_seq();
			break;
		case FLI_TOT_PUL:
			set_total_frames();
			break;
		case FLI_COM_PUL:
			qload_overlay();
			break;
		case FLI_JOI_PUL:
			qdo_composite();
			break;
		case FLI_EFF_PUL:
			pixel_menu();
			break;
		case FLI_BAC_PUL:
			qsave_backwards();
			break;
		case FLI_SAV_PUL:
			go_save_segment();
			break;
		case FLI_FIL_PUL: /* files */
			go_files(0);
			break;
		case PIC_CLE_PUL:	/* clear */
			clear_pic();
			break;
		case PIC_RES_PUL: /* restore */
			restore();
			break;
		case PIC_APP_PUL: /* apply ink */
			auto_set();
			break;
		case PIC_SEP_PUL:	/* separate */
			separate();
			break;
		case PIC_VIE_PUL:  /* view */
			view_frame();
			break;
		case PIC_FIL_PUL: /* files */
			go_files(1);
			break;
		case CEL_CLI_PUL:	/* clip */
			clip_cel();
			break;
		case CEL_GET_PUL:	/* get */
			cut_out_cel();
			break;
		case CEL_SNI_PUL:  /* lasoo */
			lasso_cel();
			break;
		case CEL_MOV_PUL: /* translate */
			move_the_cel();
			break;
		case CEL_PAS_PUL: /* paste */
			paste_the_cel();
			break;
		case CEL_STR_PUL:  /* stretch */
			vstretch_cel(0);
			break;
		case CEL_TUR_PUL:  /* rotate */
			vrotate_cel(0);
			break;
		case CEL_CEL_PUL: /* Anim Cel */
			go_cel_menu();
			break;
		case CEL_REL_PUL:  /* free */
			delete_the_cel();
			break;
		case CEL_FIL_PUL:  /* files */
			go_files(2);
			break;
		case TRA_BLU_PUL: /* blue frame */
			qblue_pic();
			break;
		case TRA_UNB_PUL:	/* unblue frame */
			qunblue_pic();
			break;
		case TRA_NEX_PUL:	/* next blue */
			qnext_blue();
			break;
		case TRA_INS_PUL:	/* tween guides */
			insert_tween();
			break;
		case TRA_ERA_PUL:	/* erase guides */
			clean_tween();
			break;
		case TRA_CLI_PUL:	/* get changes */
			qget_changes();
			break;
		case TRA_REP_PUL: /* repeat changes */
			qnext_changes();
			break;
		case TRA_LOO_PUL:
			loop_range();
			break;
		case TRA_SEG_PUL: /* flip range */
			flip_range();
			break;
		case TRA_FLI_PUL:	/* flip5 */
			flip5();
			break;
		case SWA_CLI_PUL: /* grab alt screen */
			grab_alt();
			break;
		case SWA_TRA_PUL: /* swap alt screen */
			swap_alt();
			break;
		case SWA_PAS_PUL: /* put alt screen */
			auto_put();
			break;
		case SWA_VIE_PUL: /* view alt*/
			view_alt();
			break;
		case SWA_REL_PUL:
			qfree_alt();
			break;
		case POC_PRO_PUL:
#ifdef WITH_POCO
			go_pgmn();
#endif /* WITH_POCO */
			break;
		case POC_USE_PUL:
#ifdef WITH_POCO
			quse_poco();
#endif /* WITH_POCO */
			break;
		case EXT_MAS_PUL:		/* stencil */
			qmask();
			break;
		case EXT_GRI_PUL:		/* grid */
			qgrid();
			break;
		case EXT_REC_PUL:		/* macro */
			qmacro();
			break;
		case EXT_SET_PUL: /* settings */
			go_files(9);
			break;
		case EXT_CON_PUL: /* configure */
			new_config();
			break;
		case EXT_INF_PUL:
#ifdef SLUFFED
			status();
#endif /* SLUFFED */
			break;
		case EXT_SCR_PUL:
			return_to_main(RESET_SCREEN_SIZE);
			break;
		}
	}
show_mp();
}






