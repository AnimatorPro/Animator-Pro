/* Player.c - plays back fli's  from a file containing a list of
 * fli's to play, and various  other commands. */

#include "player.h"
#include "argparse.h"

USHORT program_version, program_id;

Playcb pcb;


Errcode init_after_screen()
/*****************************************************************************
 * Initialization passed to go_resize_screen().   If this were PJ this
 * would be more complicated....
 ****************************************************************************/
{
	show_mouse();
	return(Success);
}
void hide_mp(void)
{
	stack_hide_cgroup(vb.screen);
}

void show_mp(void)
{
	stack_show_cgroup(vb.screen);
}
void exit_player(Errcode err)
/*****************************************************************************
 * Free up all our resources and exit program.
 ****************************************************************************/
{
	err = softerr(err,"play_fatal");
	close_curfli();
	cleanup_pstack();
	cleanup_screen(); 
	cleanup_startup();
	print_alloclist();
	exit((err >= Success)?0:1);
}
static Errcode resize_screen()
/*****************************************************************************
 * Bring up driver requestor to let user set which screen resolution and
 * which driver.
 ****************************************************************************/
{
Errcode err;

	close_curfli();
	pj_clear_rast(vb.pencel);
	if((err = go_resize_screen(init_after_screen,NULL)) < Success
		&& err != Err_abort)
	{
		return(err); /* screen init failed, fatal */
	}
	return(Success);
}
static Boolean auto_on = FALSE;

static Errcode set_auto_on(struct argparse_list *ap, int argc,
	char **argv, int position)
/* Called by arg-parser to set a flag. */
{
auto_on = TRUE;
return(0);
}

static Errcode get_script_arg(Argparse_list *ap, int argc,char **argv,
				 			  int position)
/* Append .scr to any arguments not otherwise matched. */
{
	if(pcb.scr_root[0])
		return(-1);
	copy_insure_suffix(argv[0], ".scr", pcb.scr_root, sizeof(pcb.scr_root));
	return(0);
}


main(int argc, char **argv)
{
Errcode err, oldconfig;
static Argparse_list apl[] = {
	ARGP(apl,0,"-a",set_auto_on),
	ARGP(apl,APLAST,"-auto",set_auto_on),
};

	if((err = init_pj_startup(apl,get_script_arg,argc,argv,
						   "play_help","aa.mu")) < Success)
	{
		goto error;
	}
	oldconfig = err;

	if((err = open_pj_startup_screen(init_after_screen)) < Success)
		goto error;

	if (!auto_on)
		{
		soft_continu_box("aniplay_free");
		}
	if(!oldconfig)
		soft_continu_box("newconfig");

	if(pcb.scr_root[0] != 0)
	{
		if((err = play_script(FALSE)) != Err_abort)
			goto error;
	}
	close_curfli(); /* this will initialize things even though it is closed */

	for(;;)
	{
		err = go_player();
		switch(err)
		{
			case PRET_DOSCRIPT:
				play_script(TRUE);
				break;
			case PRET_QUIT:	
				if(soft_yes_no_box("play_quit"))
					goto done;
				break;
			case PRET_RESIZE_SCREEN:
				if((err = resize_screen()) >= Success);
					break;
			default:
				goto error;
		}
	}

done:
error:
	exit_player(err);
}
