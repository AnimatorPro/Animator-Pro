/* playpull.c - main switch for pulldowns, and relatively small routines
 * called from that switch */

#include "player.h"
#include "picdrive.h"
#include "progids.h"
#include "commonst.h"
#include "softmenu.h"

#define GIFTYPE 1
#define PCXTYPE 2

static char pic_path[PATH_SIZE];
static char pcx_pdr_name[] = "pcx.pdr";
static SHORT pic_tname;

enum pull_ids {
	PLA_ABO = 101,
	PLA_MEM = 102,
	PLA_QUI = 107,
	FIL_FLI = 201,
	FIL_OFLI = 202,
	FIL_GIF = 203,
	FIL_PCX = 204,
	FIL_OTH = 205,
	FIL_SCR = 206,
	/* ---- */
	FIL_SCREEN = 208,
};

static void qload_picture(char *pdr,char *path)
{
	softerr(load_picture(pdr,path,TRUE), "!%s", "play_nopic", path );
}
static void about_player(void)
{
static char date[] = __DATE__;
char relnum[16];
char idtext[32];

	get_relvers(relnum);
	get_userid_string(idtext);
	soft_continu_box("!%s%s%.3s%.2s%.4s%.5s","play_about",relnum,idtext,
					 &date[0], &date[4], &date[7], __TIME__ );
}

#ifdef OLD
		/* Note this should go into resource file soon, but we
		 * want to release this without updating the resource at
		 * the moment.  */

static char about_string[]  = 
	"Autodesk Animator Pro Player (tm)\n"
	"             v.1.1\n"
	"      Copyright 1991 by Jim Kent.\n"
	"          Oct. 4, 1991.\n"
	"\n"
	"      This product is freely\n"
	"          distributable.\n";
static void about_player(void)
{
continu_box(about_string);
}
#endif /* OLD */

extern long mem_free;
extern long init_mem_free;

void qplay_memory()
{
	soft_continu_box("!%d%d%d%d", "play_mem", 
					  mem_free, init_mem_free, pj_mem_used, pj_max_mem_used );
}

static Boolean qload_script()
{
char hailing[80];

	stack_string("play_loadscr", hailing );
	if(pj_get_filename(hailing,
					".SCR", "Play", pcb.scr_root, pcb.scr_root,
					FALSE, &pcb.scr_scroll_top, pcb.scr_wild ) == NULL)
	{
		return(FALSE);	
	}

	return(TRUE);	
}
static void qload_fli(Boolean to_ram)
{
Errcode err;
static char fli_wild[WILD_SIZE];
static SHORT fli_tname;
char *path;
char hailing[80];
char ok_button[20];
char *key;
static char nkey[] = "play_flicload";

	key = (to_ram?"to_load":"to_open");
	soft_name_string(nkey,key,hailing, sizeof(hailing));
	soft_name_string(nkey,&key[3], ok_button, sizeof(ok_button));

	path = pj_get_filename( hailing,
						    ".FL?;.FLC;.FLI", 
							ok_button,
						    pcb.fliname, pcb.fliname, FALSE, 
						    &fli_tname, fli_wild );

	if(!path) 
		return;

	free_ramflis();

	err = open_curfli(TRUE,to_ram);
	if(to_ram)
		softerr(err,"!%s", "play_nfli_load", path );
	else
		softerr(err,"!%s", "play_nfli_open", path );
}
static void qload_gif()
{
char *path;
static char pic_wild[WILD_SIZE];
char hailing[80];

	stack_string("play_gifl", hailing );
	*pj_get_path_name(pic_path) = 0;

	path = pj_get_filename(hailing, ".GIF", load_str,
						   pic_path, pic_path, FALSE, &pic_tname, pic_wild );
	if(path)
		qload_picture(gif_pdr_name,path);

}
static void qload_pcx()
{
char *path;
static char pic_wild[WILD_SIZE];
char hailing[80];

	stack_string("play_pcxl", hailing );
	*pj_get_path_name(pic_path) = 0;

	path = pj_get_filename(hailing, ".PCX", load_str,
						   pic_path, pic_path, FALSE, &pic_tname, pic_wild );
	if(path)
		qload_picture(pcx_pdr_name,path);
}
static void qload_other()
{
char suffi[PDR_SUFFI_SIZE];
char pdr_name[PATH_SIZE];
char headtxt[60];
char hailing[80];
char suffix[6];
char *path;

	stack_string("play_fmtl", hailing);
	stack_string("play_ffl", headtxt);

	if((go_pdr_menu(hailing,pdr_name,suffi,NULL,0,FALSE)) < Success)
		return;

	path = suffi;
	parse_to_semi(&path, &suffix,sizeof(suffix));
	snftextf(hailing, sizeof(hailing),"!%s", headtxt, suffix );

	*pj_get_path_name(pic_path) = 0;

	path = pj_get_filename( hailing, suffi, load_str, pic_path, pic_path, 
							FALSE, &pic_tname, NULL );
	if(path)
		qload_picture(pdr_name,path);
}

static void player_selit(Menuhdr *mh, SHORT hitid)
{
	hide_mp();
	switch(hitid)
	{
		case PLA_ABO: /* about */
			about_player();
			break;
		case PLA_MEM: /* memory */
			qplay_memory();
			break;
		case PLA_QUI: /* quit */
			return_to_main(PRET_QUIT);
			break;
		case FIL_FLI:
			qload_fli(TRUE);
			break;
		case FIL_OFLI:
			qload_fli(FALSE);
			break;
		case FIL_GIF:
			qload_gif();
			break;
		case FIL_PCX:
			qload_pcx();
			break;
		case FIL_OTH:
			qload_other();
			break;
		case FIL_SCR:
			if(qload_script())
				return_to_main(PRET_DOSCRIPT);
			break;
		case FIL_SCREEN: /* screen size */
			return_to_main(PRET_RESIZE_SCREEN);
			break;
		default:
		#ifdef DEBUG
			boxf("hit # %d", hitid);
		#endif
			break;
	}
	show_mp();
}

static int player_dopull(Menuhdr *mh)
{
	return(menu_dopull(mh));
}
void set_playpull_disables(Menuhdr *mh)
{
	set_pul_disable(mh, FIL_GIF, !resource_exists(gif_pdr_name));
	set_pul_disable(mh, FIL_PCX, !resource_exists(pcx_pdr_name));
}
Errcode go_player(void)
{
Errcode err;
Menuhdr tpull;
void *ss;

	if((err = load_play_panel(&ss)) < Success)
		goto error;

	if ((err = load_soft_pull(&tpull, 0, "play_pull", 0,
							  player_selit, player_dopull)) >= Success)
	{
		/* note these are clipped when menu is opened, this will force it 
		 * to lower right */
		set_playpull_disables(&tpull);
		player_menu.x = vb.screen->wndo.width;
		player_menu.y = vb.screen->wndo.height;
		err = do_menuloop(vb.screen,&player_menu,NULL,&tpull,player_do_keys);
		smu_free_pull(&tpull);
	}
error:
	smu_free_scatters(&ss);
	return(softerr(err,"play_menu"));	
}

