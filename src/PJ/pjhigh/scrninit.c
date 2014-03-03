/* scrninit.c whats needed to initialize screen and menu fonts and mouse input
 * and to clean it up */

#define SCRNINIT_CODE
#include "errcodes.h"
#include "pjbasics.h"
#include "rastext.h"
#include "rastcurs.h"
#include "resource.h"
#include "vdevcall.h"
#include "vdevinfo.h"

extern char key_idriver_name[], mouse_idriver_name[], summa_idriver_name[];

Errcode init_input(void)
/* called on startup to initialize the mouse and other input */
{
Errcode err;
static char *idr_names[] = {mouse_idriver_name, 
							summa_idriver_name,
							vconfg.idr_name };
char idr_path[PATH_SIZE];
char *idr_name;

	idr_name = idr_names[vconfg.dev_type];
	make_resource_name(idr_name, idr_path);

	if((err = init_idriver(idr_path,vconfg.idr_modes,
						   vconfg.comm_port)) < Success)
	{
		init_idriver(key_idriver_name, vconfg.idr_modes, 0);
	}

	/* setup input to current icb.input_screen loaded by open_wscreen() */

	reset_input();
	enable_textboxes();

	if (err < Success)
	{
		softerr(err,"!%s","nomouse", idr_name );
	}
	return(0);
}

static Vfont menufont;
void free_menu_font(void)
{
	close_vfont(&menufont);
}

static Errcode load_menu_font(Wscreen *s)
{
extern char *lrmenu_font_name;
extern char *menu_font_name;
extern char *hrmenu_font_name;
char path[PATH_SIZE];
char *name;

	disable_textboxes();
	free_menu_font();

	if (s->wndo.width < 500) 
		name = lrmenu_font_name;
	else if(s->wndo.width < 800)
		name = menu_font_name;
	else
		name = hrmenu_font_name;

	make_resource_name(name, path);
	if(load_font(path, &menufont,0,FALSE) >= Success)
		s->mufont = &menufont;
	else
		s->mufont = get_sys_font();
	return(Success);
}

static void set_menu_scale(Wscreen *s)
/* scale all staticly defined menus and menu parts */
{
	if (s->wndo.width >= 500)
	{
		if (s->wndo.width >= 800)
		{
			s->menu_scale.xscalep = 200;
			s->menu_scale.yscalep = 200;
			s->bbevel = 4;
		}
		else
		{
			s->menu_scale.xscalep = 150;
			s->menu_scale.yscalep = 150;
			s->bbevel = 3;
		}
		s->is_hires = TRUE;
	}
	else
	{
		s->menu_scale.xscalep = 100;
		s->menu_scale.yscalep = 100;
		s->is_hires = FALSE;
		s->bbevel = 2;
	}
	s->menu_scale.xscaleq = 100;
	s->menu_scale.yscaleq = 100;
	load_menu_font(s);
}

static Errcode get_drv_default_size(SHORT mode, SHORT *width, SHORT *height)
{
Errcode err;
Vmode_info mode_info;

	if((err = pj_vd_get_mode(vb.vd,mode,&mode_info)) < Success)
		return(err);

	*width = mode_info.width.min;
	*height = mode_info.height.min;
	return(Success);
}
static Errcode open_screen(Screen_mode *sm, char *path)
{
Errcode err;
char pathbuf[PATH_SIZE];

	make_resource_name(path, pathbuf);
	if((err = pj_open_ddriver(&vb.vd, pathbuf)) < Success)
		goto error;

	if((err = pj_vd_verify_hardware(vb.vd)) < Success)
		goto error;

	/* use default size of mode if width and height not set */
	if(!sm->width || !sm->height)
	{
		if((err = get_drv_default_size(sm->mode,&sm->width,
										&sm->height)) < Success)
		{
			goto error;
		}
	}
	/* open the screen cel */

 	if((err = alloc_display_rcel(vb.vd,&vb.cel_a,
								 sm->width,sm->height,sm->mode)) < Success)
	{
		goto error;
	}
	return(Success);
error:
	pj_close_vdriver(&vb.vd);
	restore_ivmode();
	return(softerr(err, "!%s%d%d%d", "driver_open", 
				   pathbuf, sm->width, sm->height,
				   sm->mode ));
}
Errcode init_screen(Screen_mode *smode,Screen_mode *altmode, 
					Errcode (*init_with_screen)(void *iwdat), void *iwdat ) 

/* try to open video in smode, if that fails try altmode if altmode is
 * is the same as smode, simply fail. If altmode is NULL try generic MCGA 
 * mode. if a screen is opened successfully setup every thing else
 * needed for menuing system, if window and menu system open fails, cleanup
 * screen and return Error. If smode is selected return 0 altmode 1. 
 * if screen stuff is open before this is called one must call 
 * cleanup_screen() first.  The init_with_screen() function
 * is used since most systems here need to have things intimately linked
 * with the screen that must be present for the screen to be considered
 * valid if it is NULL it won't be called. If it returns error the screen is
 * cleaned up and the error returned */
{
int err = Err_nogood;
Screen_mode sm;
Errcode goodret = 0;
extern char pj_mcga_name[];

	/* set up GS segment */
	pj_set_gs();

	/* open screen driver and open screen */

	sm = *smode;

	if(vb.ivmode == -1) /* not yet saved */
		vb.ivmode = pj_get_vmode(); /* save startup dos video mode */

	if((err = open_screen(&sm, sm.drv_name)) < Success)
	{
		if(altmode == smode) /* no alternate */
			goto error;
		if(altmode != NULL)
			sm = *altmode;
		else
		{
			clear_struct(&sm);
			strcpy(sm.drv_name,pj_mcga_name);
		}

		if ((err = open_screen(&sm, sm.drv_name)) < Success)
			goto error;
		goodret = 1;
	}
	else
		goodret = 0;

	/* install and set initial colormap */
	pj_get_default_cmap(vb.cel_a->cmap);
	pj_cmap_load(vb.cel_a,vb.cel_a->cmap);

	/* open window control on display rasters */
	{
	WscrInit newscr;

		clear_mem(&newscr,sizeof(newscr));
		newscr.max_wins = 10;
		newscr.disp_driver = vb.vd;
		newscr.cel_a = vb.cel_a;
		newscr.cel_b = vb.cel_b;
		newscr.flags = (WS_MUCOLORS); /* yep we want 'em */
		newscr.cursor = (Cursorhdr *)get_default_cursor();

		if((err = open_wscreen(&vb.screen,&newscr)) < 0)
			goto error;

		vb.screen->mc_ideals = &vconfg.mc_ideals;
	}

	/* set pre calc'd center point */

	vb.scrcent.x = vb.screen->wndo.width>>1;
	vb.scrcent.y = vb.screen->wndo.height>>1;

	/* for safety set initial window to whole screen */
	vb.pencel = (Rcel *)&(vb.screen->wndo); 

	if((err = init_muscreen(vb.screen)) < 0) /* initialize menu control */
		goto error;

	set_menu_scale(vb.screen);
	set_cursor(vb.screen->cursor);

	if((err = init_input()) < 0)  /* at this point we may use text boxes */
		goto error;

	if(init_with_screen != NULL
		&& (err = init_with_screen(iwdat)) < Success)
	{
		goto error;
	}

	init_wrefresh(vb.screen); 

	/* save actual operational setting in config file */

	if(memcmp(&sm,&vconfg.smode))
	{
		vconfg.smode = sm;
		rewrite_config();
	}
	return(goodret);
error:
	cleanup_screen();
	return(err);
}
