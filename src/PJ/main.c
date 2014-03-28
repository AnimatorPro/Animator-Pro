/* main.c -  Make sure system initializes ok, and then flop up the menuing
  system. */

#include <stdio.h>
#include "jimk.h"
#include "aaconfig.h"
#include "argparse.h"
#include "brush.h"
#include "errcodes.h"
#include "flx.h"
#include "inks.h"
#include "menus.h"
#include "palmenu.h"
#include "pentools.h"
#include "picdrive.h"
#include "progids.h"
#include "rastcurs.h"

static Errcode resize_pencel(Boolean err_on_abort,Boolean reset);

USHORT program_id = 0;
USHORT program_version = 0;
extern Errcode builtin_err;

extern void do_auto_redo(), swap_undo();

static Errcode set_flisize(Rectangle *newsize);

static Errcode init_after_screen()
/* initializes and allocs every thing that has to be done after the screen
 * and before the dynamic stuff (tempflx) and push/pop stuff is opened */
{
	init_cursors();
	vb.screen->menu_cursor = &menu_cursor.hdr;
	vb.screen->cursor = &menu_cursor.hdr;
	set_cursor_ccolor((Pixel *)(&vs.ccolor)); /* only works with 80x86 */
	return(init_brushes());
}
static void close_init_after()
{
	cleanup_brushes();
	cleanup_cursors();
}

static Errcode force_temp_files(void)

/* Check for a tempflx file on current scratch device.	If it's there
   set up to use it.  Otherwise check for a default settings file in
   temp file directory and start up program with color map and settings
   from that. if window open fails give user option of using smaller
   size and abandoning tflx, If this doesn't work give up and return error. */
{
Errcode err;
Rectangle flxsize;

	if((err = open_tempflx(TRUE)) >= Success)
	{
		flxsize.width = flix.hdr.width;
		flxsize.height = flix.hdr.height;
		close_tflx();
		if((err = set_flisize(&flxsize)) < Success)
			return(resize_pencel(TRUE,TRUE)); /* user will abort and exit */
		return(open_tempflx(TRUE));
	}

	if((err = set_penwndo_size(vb.screen->wndo.width,
							   vb.screen->wndo.height)) < 0)
	{
		return(err);
	}
	return(open_default_flx());
}

static Errcode clear_vtemps(Boolean reset)
{
	if (reset)
		{
		pj_delete(optics_name); /* get rid of optics moves */
		pj_delete(ppoly_name);	/* and optics path */
		}
	pj_delete(tflxname);	  /* and old tempflx */
	change_dir(vb.init_drawer);  /* go back to startup-dir */
	return(Success);
}

static void push_close_toscreen()
{
	push_most();
	pj_clear_rast(vb.screen->viscel);  /* clear anything left in there */
	close_downto_screen();	/* close all but screen */
}
Errcode empty_newflx(void)
/* Open a new empty flx, but with old settings */
{
Vset_flidef fdef;

	if(load_default_flidef(&fdef) < Success
		|| fdef.frame_count < 1)
	{
		fdef.frame_count = 1;
	}
	vs.bframe_ix = 0;
	rethink_settings();
	return(empty_tempflx(fdef.frame_count));
}
static Errcode reopen_tempflx(Boolean reset)
{
Errcode err;

	/* if tflx is there re open it, otherwise open a new default flx
	 * if that fails put up a new flx */

	if( (!pj_exists(tflxname))
		|| open_tempflx(TRUE) < Success)
	{
		if(reset)
			err = open_default_flx();
		else
		{
			err = empty_newflx();
		}
		if(err < Success)
			return(err);
	}
	return(RESTART_VPAINT); /* return to quick menu */
}
static Errcode resize_screen()
{
Errcode err;
Rectangle oldsize;
Screen_mode oldmode;
Cmap ocolors;



	oldmode = vconfg.smode;
	copy_rectfields(vb.pencel,&oldsize);
	scrub_cur_frame();
	flush_tflx();

	/* save old colors on stack */
	ocolors.num_colors = COLORS;
	pj_cmap_copy(vb.pencel->cmap,&ocolors);

	push_close_toscreen();

	for(;;)
	{
		free_undof(); /* re alloc'd by set penwndo size */
		if((err = go_resize_screen(init_after_screen,
									close_init_after)) < Success
			&& err != Err_abort)
		{
			return(err); /* screen init failed, fatal */
		}

		if (err >= Success
			&& soft_yes_no_box("full_flic"))
		{
			if(set_flisize((Rectangle *)&(vb.screen->wndo.RECTSTART))< Success)
				continue; /* try again */

			/* put back old cmap since it was trashed when screen was freed */
			pj_cmap_copy(&ocolors,vb.pencel->cmap);
			see_cmap();
			if ((err = empty_newflx()) < Success)
				return(err);
		}
		else
		{
			if((err = set_flisize(&oldsize)) < Success)
				continue; /* try again */

			if((err = open_tempflx(TRUE)) < Success)
			{
				softerr(err,"tflx_screen");
				close_init_after();
				cleanup_screen();
				if((err = init_screen(&oldmode,NULL,init_after_screen))
					< Success)
					return(err); /* fatal */
				/* if next reopen fails waste file but leave settings */
				return(reopen_tempflx(FALSE));
			}
		}
		return(RESTART_VPAINT);
	}
}
static Errcode set_flisize(Rectangle *newsize)
{
Errcode err;

	if((err = set_penwndo_size(newsize->width,newsize->height)) < Success)
		flisize_error(err,newsize->width,newsize->height);
	return(err);
}
char *cl_poco_name;  /* loaded from arguments */
char *cl_flic_name;  /* Flic loaded from arguments. */
static char po_suffix[] = ".POC";

static Errcode go_vpaint()
{
extern Errcode go_quick_menu();

	init_wrefresh(vb.screen); /* clear window refresh stack start over */
	fake_push();
	pop_most();

	/* restore screen from pic file if there if not seek in fli */

	if(pop_screen_id(flix.hdr.id.update_time) != Success)
	{
		fli_abs_tseek(undof, vs.frame_ix);
		zoom_unundo();
	}
	else
		save_undo();

	init_zoom();
	find_colors();
	fliborder_on();
	vl.undoit = swap_undo;
	vl.redoit = do_auto_redo;
	menu_to_quickcent(&quick_menu);

#ifdef WITH_POCO
	if(cl_poco_name != NULL)
	{
		Errcode err = do_cl_poco(cl_poco_name);
		if (err < Success && err != Err_abort)
		{
			cleanup(TRUE);
			if (err == Err_in_err_file)
				po_file_to_stdout(poco_err_name);
			exit(err);
		}
		cl_poco_name = NULL; /* don't keep re-executing */
	}
#endif /* WITH_POCO */

	return(go_quick_menu());
}

static Errcode get_poco_arg(Argparse_list *ap,int argc,
							 char **argv,int position)
{
(void)ap;
(void)position;

if (argc < 2 )
	return Err_bad_input;
cl_poco_name = argv[1];
if (cl_poco_name[0] == '-')
	return Err_bad_input;
}

static Errcode get_flic_arg(Argparse_list *ap,int argc,
							 char **argv,int position)
{
(void)ap;
(void)position;

if (argc < 2 )
	return Err_bad_input;
cl_flic_name = argv[1];
if (cl_flic_name[0] == '-')
	return Err_bad_input;
}


static Errcode get_rest_of_command_line(Argparse_list *ap,int argc,
							 char **argv,int position)
{
	char *arg;
	int i;
	(void)ap;
	(void)position;

	for (i=0; i<argc; ++i)
		{
		arg = argv[i];
		if (suffix_in(arg, po_suffix))
			cl_poco_name = arg;
		else
			cl_flic_name = arg;
		}
	return argc;
}

#ifdef TEST_LO_MEM
int eat_memory()
/* Just for testing eat all but 10K. */
{
	char *pt;
	char *lastpt = NULL;
	int count = 0;

	while ((pt = malloc(10000)) != NULL)
		{
		lastpt = pt;
		++count;
		}
	if (lastpt != NULL)
		free(lastpt);
	return count;
}
#endif /* TEST_LO_MEM */

static void add_local_pdrs(void)
{
	extern Local_pdr bmp_local_pdr;
	extern Local_pdr flilo_local_pdr;
	extern Local_pdr gif_local_pdr;
	extern Local_pdr pcx_local_pdr;
	extern Local_pdr rif_local_pdr;
	extern Local_pdr targa_local_pdr;

	add_local_pdr(&fli_local_pdr);
	add_local_pdr(&pic_local_pdr);

	add_local_pdr(&bmp_local_pdr);
	add_local_pdr(&flilo_local_pdr);
	add_local_pdr(&gif_local_pdr);
	add_local_pdr(&pcx_local_pdr);
	add_local_pdr(&rif_local_pdr);
	add_local_pdr(&targa_local_pdr);
}

static void delete_file_list(char **list)
{
	const char *name;

	while ((name = *list++) != NULL)
		pj_delete(name);
}

static void cleanup(Boolean save_state)
{
	/* delete back buffer screen */
	pj_delete(bscreen_name);
	vs.bframe_ix = 0;
	if (save_state) {
		soft_put_wait_box("wait_quit");
		flush_tempflx(); /* update tempflx header and stuff */
		flush_tsettings(TRUE); /* update temp settings file */
	}
	close_tflx();
	/* push a copy of current screen and alt,cel etc for when program
	 * started again with id of last tflx flush...
	 */

	delete_file_list(work_temp_files);
	if (save_state) {
		/* move files from memory to filing system */
		push_pics_id(flix.hdr.id.update_time);

		softerr(trd_ram_to_files(), "!%s", "temp_copy", get_temp_path());
		rcompact(); /* free blocks used for ram-disk */
	}
	else {
		delete_file_list(state_temp_files);
	}
	cleanup_all(Success);
}

static void outofhere(Boolean save_state)
{
	cleanup(save_state);
	exit(0);
}

void main(int argc, char **argv)
{
Errcode err;
UBYTE oldconfig;
static Argparse_list apl[] = {
	ARGP(apl,0,"-flic",get_flic_arg),
	ARGP(apl,APLAST,"-poc",get_poco_arg),
};

	if((err = init_pj_startup(apl,get_rest_of_command_line,argc,argv,
							  "pj_help","aa.mu")) < Success)
	{
		goto error;
	}
	oldconfig = err;

	add_local_pdrs();
	set_hotkey_func(do_pj_hotkey); /* set input hot key function */

	/* initialize pj resource files */

	if((err = init_pj_resources()) < Success)
		goto error;

	if((err = init_ptools()) < Success) /* initialize tools */
		goto error;
	if((err = init_inks()) < Success) /* load any loadable inks */
		goto error;

#ifdef WITH_POCO
	if(cl_poco_name != NULL)
	{
		if ((err = compile_cl_poco(cl_poco_name)) < Success)
		{
			if (err == Err_in_err_file)
				po_file_to_stdout(poco_err_name);
			err = Err_reported;
			goto error;
		}
	}
#endif /* WITH_POCO */

	vs = default_vs; /* copy in default settings */


	if((err = open_pj_startup_screen(init_after_screen)) < Success)
		goto error;

	if(!oldconfig)
		soft_continu_box("newconfig");

	if (cl_flic_name != NULL)
		pj_delete(tflxname);	  /* Delete old tempflx */

	if((err = force_temp_files()) < Success)
		goto error;

	if (cl_flic_name != NULL)
		resize_load_fli(cl_flic_name);

	err = go_vpaint();

	for(;;)
	{
		switch(err)
		{
			case RESET_SCREEN_SIZE:
				err = resize_screen();
				break;
			case RESET_NEW_SIZE:
			case KILL_NEW_SIZE:
				scrub_cur_frame();	/* clean up act in case user aborts */
				flush_tflx();
				err = resize_pencel(FALSE,err == RESET_NEW_SIZE);
				break;
			case RESET_DEFAULT_FLX:
				push_close_toscreen();
				if((err = clear_vtemps(TRUE)) < 0)
					goto error;
				if((err = open_default_flx()) < 0)
					goto error;
			case RESTART_VPAINT:
				err = go_vpaint();
				break;
			case EXIT_SYSTEM:
				outofhere(TRUE); /* we've exited, don't need to break... */
			case QUIT_SYSTEM:
				outofhere(FALSE);
			default: /* not a good return */
				goto error;
		}
	}
error:
	cleanup_all(err);
	exit(err);
}

static Errcode resize_pencel(Boolean err_on_abort,Boolean reset)
{
Errcode err;
Rectangle newsize;
Rectangle flisize;
Boolean was_zoom;

	if(!reset)
		was_zoom = vs.zoom_open;

	copy_rectfields(vb.pencel,&flisize); /* save original fli size */
	push_close_toscreen();

	for(;;) /* keep doing until success error or aborted */
	{
		err = go_format_menu(&newsize);
		if(err == Err_abort)
		{
			if(err_on_abort)
				goto error;

			if(!pj_exists(tflxname))
				break;
			if(set_flisize(&flisize) >= Success)
				break;
			flisize = newsize;
			clear_vtemps(reset);
			continue;
		}
		else if(err == RESET_SCREEN_SIZE)
		{
			for(;;)
			{
				free_undof(); /* re alloc'd with set penwndo size */
				err = go_resize_screen(init_after_screen,close_init_after);

				if(err >= Success)
				{
					if((err = clear_vtemps(reset)) < Success)
						goto error;
				}
				else if(err != Err_abort)
					goto error;

				if(set_flisize((Rectangle *)&(vb.screen->wndo.RECTSTART))
								< Success)
				{
					continue;
				}
				break;
			}
			continue; /* go do format menu again */
		}
		else if(err < Success)
			goto error;

		if((err = clear_vtemps(reset)) < Success)
			goto error;

		if(set_flisize(&newsize) >= Success)
			break;
		/* try again */
	}
	err = reopen_tempflx(reset);
	if(!reset)
		vs.zoom_open = was_zoom;
error:
	return(err);
}
