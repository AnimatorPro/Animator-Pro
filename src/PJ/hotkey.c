#include <string.h>
#define INPUT_INTERNALS
#include "jimk.h"
#include "errcodes.h"
#include "picfile.h"
#include "vsetfile.h"

static Errcode increment_old(char *path)

{
char *pname;
char iname[FILE_NAME_SIZE]; /* big for safety */

	if(pj_exists(path))
	{
	 	pname = pj_get_path_name(path);
		if(strlen(pname) > sizeof(iname)-1)
			return(Err_file_name_err);

		strcpy(iname,pname);
		pj_inc_filename(iname);

		switch(soft_qchoice(NULL,"!%s%s", "over_inc_old", pname, iname ))
		{
			default:
				return(Err_abort);
			case 0:
				pj_inc_filename(path);
			case 1:
				break;
		}
	}
	return(Success);
}
static void save_snapshot(void)
/* this routine is called recursively in input only allowed to be called 
 * twice so it can snapshoot itself */
{
Errcode err;
char path[PATH_SIZE];
Icb_savebuf *pushed;
Icb_savebuf icb_save;
static BYTE level = 0;

	if(level >= 1)
		return;

	++level;
	pushed = check_push_icb();

	if((err = vset_get_path(SNAPSHOT_PATH,path)) < Success)
		goto error;

	if(*path == 0)
	{
		if((err = make_file_path(vb.init_drawer,"snapshot.gif", path)) < 0)
			goto error;
	}

	if(soft_qreq_string(path,sizeof(path)-1,"gif_snap"))
	{
	char *sfix;

		if( *(sfix = fix_suffix(path)) == 0)
		{
			if(sfix > &path[sizeof(path) - 5])
			{
				err = Err_file_name_err;
				goto error;
			}
			strcpy(sfix,".GIF");
		}

		if((err = increment_old(path)) >= Success)
		{
			save_icb_state(&icb_save);
			restore_icb_state(pushed); /* restore state ie: cursor */ 
			err = save_gif(path, vb.cel_a);
			restore_icb_state(&icb_save);
			vset_set_path(SNAPSHOT_PATH,path);
		}
	}
error:
	softerr(err,"!%s","cant_save",path);
	--level;
}
Boolean do_pj_hotkey(Global_icb *gicb)

/********* routines called from within input loop for "HOT key" functions ***/
{
	switch(gicb->inkey)
	{
		case FKEY3:
			vs.mkx = gicb->sx; 
			vs.mky = gicb->sy; 
			gicb->state |= MBPEN;
			break;
		case FKEY4:
			gicb->sx = vs.mkx;
			gicb->sy = vs.mky;
			gicb->state |= MBPEN;
			break;
		case FKEY5:
			save_snapshot();
			break;
#ifdef TESTING
		case FKEY10:
			debug = !debug;
			if(debug)
				boxf("Debug flag = TRUE");
			else
				boxf("Debug flag = FALSE");
			break;
#endif /* TESTING */
		default:
			return(FALSE);
	}
	return(TRUE);
}
