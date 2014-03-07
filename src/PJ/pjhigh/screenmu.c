#define SCRNINIT_CODE
#include "aaconfig.h"
#include "errcodes.h"
#include "pjbasics.h"
#include "resource.h"
#include "rexlib.h"
#include "softmenu.h"
#include "util.h"
#include "vdevinfo.h"
#include "wildlist.h"

typedef struct mode_entry {
	void *next;
	char *mode_name;
	char *drv_file;
	Srange wrange;
	Srange hrange;
	USHORT mode;
	char name_buf[80];
} Mode_entry;

static void free_mode_list(Mode_entry **pmentry,Names **wildlist)
{
Mode_entry *this;

	while((this = *pmentry) != NULL)
	{
		*pmentry = this->next;
		pj_free(this);
	}
	free_wild_list(wildlist);
}
static Errcode build_mode_list(Mode_entry **pmlist,Names **pwildlist,
							   Names **pcurrent )
{
extern char pj_mcga_name[];
Errcode err;
Names mcga_entry;
Names *drv_entry;
Mode_entry **pmode_entry;
Mode_entry *mentry;
Vmode_info mode_info;
Vdevice *drv = NULL;
Boolean is_current = FALSE;
USHORT mode;
char wstr[32];
char hstr[32];
int mode_count;

#ifdef USE_DYNAMIC_VIDEO_DRIVERS
	build_wild_list(pwildlist, "*.drv", FALSE);
#else /* USE_DYNAMIC_VIDEO_DRIVERS */
	*pwildlist = NULL;
#endif /* USE_DYNAMIC_VIDEO_DRIVERS */

	drv_entry = &mcga_entry;
	mcga_entry.name = pj_mcga_name;
	mcga_entry.next = *pwildlist;

	pmode_entry = pmlist;
	*pcurrent = NULL;

	while(drv_entry != NULL)
	{
		is_current = !txtcmp(vconfg.smode.drv_name, drv_entry->name);

		if(is_current)
		{
			drv = vb.vd;
		}
		else if((err = pj_open_ddriver(&drv, drv_entry->name)) < Success)
		{
			if(err != Err_no_display)
				cant_query_driver(err,drv_entry->name);
			goto next_drv_entry;
		}
		mode_count = pj_vd_get_mode_count(drv);
		for(mode = 0;mode < mode_count;++mode)
		{
			if ((err = pj_vd_get_mode(drv,mode,&mode_info)) < Success)
				goto error;
			if(NULL == (mentry = *pmode_entry = pj_zalloc(sizeof(Mode_entry))))
			{
				err = Err_no_memory;
				goto error;
			}
			mentry->mode_name = mentry->name_buf;
			mentry->drv_file = drv_entry->name;
			mentry->mode = mode;
			mentry->wrange = mode_info.width;
			mentry->hrange = mode_info.height;

			if(mode_info.width.min == mode_info.width.max)
				sprintf(wstr,"%d", mode_info.width.max);
			else
			{
				sprintf(wstr,"%d:%d",
						mode_info.width.min,mode_info.width.max);
			}

			if(mode_info.height.min == mode_info.height.max)
				sprintf(hstr,"%d", mode_info.height.max);
			else
			{
			    sprintf(hstr,"%d:%d",
						mode_info.height.min,mode_info.height.max);
			}
			sprintf(mentry->name_buf, "%-12.12s %-2d %-18.18s %s X %s", 
					mentry->drv_file, mode, mode_info.mode_name,
					wstr, hstr );

			/* load pointer to current entry if found */
			if(is_current && mode == vconfg.smode.mode)
				*pcurrent = (Names *)mentry;

			pmode_entry = (Mode_entry **)&(mentry->next);
		}
	next_drv_entry:

		if(!is_current)
			pj_close_vdriver(&drv);
		drv = NULL;
		drv_entry = drv_entry->next;
	}
	err = Success;
	goto done;
error:
	free_mode_list(pmlist,pwildlist);
done:
	if(!is_current)
		pj_close_vdriver(&drv);
	return(err);
}
static Errcode pick_screen_mode(Names *entry,void *dat)
{
Mode_entry *mentry = (Mode_entry *)entry;
Screen_mode *sm = (Screen_mode*)dat;

	strncpy(sm->drv_name,mentry->drv_file,sizeof(sm->drv_name));
	sm->mode = mentry->mode;
	sm->width = mentry->wrange.actual;
	sm->height = mentry->hrange.actual;
	return(Success);
}
static Boolean show_driver_info(Names *entry,void *dat)
{
Errcode err;
Vmode_info mi;
Mode_entry *mentry;
Vdevice *drv = NULL;
Boolean is_current;
char *more_info;
char *soft_info = NULL;

	if (NULL == (mentry = (Mode_entry *)entry)) /* it's possible for us to */
		return FALSE;						/* get called with a NULL ptr! */


	hide_mp();

	is_current = !txtcmp(vconfg.smode.drv_name, mentry->drv_file);
	if(is_current)
		drv = vb.vd;
	else
	{
		if((err = pj_open_ddriver(&drv, mentry->drv_file)) < Success)
			goto error;
	}
	if((err = pj_vd_get_mode(drv,mentry->mode,&mi)) < Success)
		goto error;

	/* check for softmenu key and retrieve pointer to text */

	more_info = pj_vd_get_more_info(drv,mentry->mode); 

	/* replace key with allocated softmenu text. If we have a key.
	 * will set soft_info to NULL if an error */

	if( ((soft_info = rex_key_or_text(more_info,&more_info)) != NULL)
		&& (smu_load_name_text(&smu_sm,"vdriver_texts", 
							   soft_info, &soft_info) >= Success))
	{
		more_info = soft_info;
	}

	soft_continu_box("!%s%s%d%d%d%d%d%d%d", "vdriver_info",
				more_info,
				mentry->drv_file, mentry->mode,
				mi.bits,
				mi.width.min, mi.width.max,
				mi.height.min, mi.height.max,
				mi.store_bytes ); 

error:
	smu_free_text(&soft_info);
	if(!is_current)
		pj_close_vdriver(&drv);
	softerr(err,"!%s%d", "driver_info", 
		    mentry->drv_file, mentry->mode);
	show_mp();
	return FALSE;  /* indicate we don't want exit from menu */
}
static Errcode go_screen_menu(Screen_mode *sm)
{
Errcode err;
Mode_entry *mlist;
Names *wildlist;
Names *current;
char opath[PATH_SIZE];
char sbuf[80];

	get_dir(opath);
	if((err = change_dir(resource_dir)) < Success)
		return(no_resource(err));

	hide_mp();

	if((err = build_mode_list(&mlist,&wildlist,&current)) < Success)
		goto error;

	err = go_driver_scroller(stack_string("screen_menu", sbuf),
								(Names *)mlist, current,
							  pick_screen_mode, show_driver_info, sm, NULL);

error:
	free_mode_list(&mlist,&wildlist);
	change_dir(opath);
	show_mp();
	return(err);
}
Errcode go_resize_screen(Errcode (*reinit)(void *dat),
						 void (*close_reinit)(void *dat), void *dat)

/* this should only be called after evrything is closed but the screen.
 * The reinit function is for initializing things that must be present for 
 * the screen to be used and should abort the request for this screen size
 * the cleanup will insure things dont get fragged by making sure the screen
 * is always openen in a clean environment */
{
Errcode err;
Screen_mode smode;

	for(;;)
	{
		if((err = go_screen_menu(&smode)) < Success)
			break;

		cleanup_screen();
		if(close_reinit)
			close_reinit(dat);
		if((err = init_screen(&smode,&vconfg.smode,reinit,dat)) < Success)
			goto error;
		if(err == 0) /* we got the primary mode */
			break;
		/* oops, didn't open what we wanted try again */
	}

error:
	return(err);
}
