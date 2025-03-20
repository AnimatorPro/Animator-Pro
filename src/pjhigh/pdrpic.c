/* this is a local pdr s that are set up for linking in to the code
 * and used by the pdr managing routines */

#include "jimk.h"
#include "animinfo.h"
#include "errcodes.h"
#include "picdrive.h"


Errcode pdr_load_picture(char *pdr_path,char *picname, Rcel *screen)
{
	Errcode err;
	Pdr *pd;
	Image_file *ifile;
	Rcel virt;
	Anim_info ainfo;

	err = load_pdr(pdr_path, &pd);
	if(err < Success) {
		return cant_use_module(err,pdr_path);
	}

	get_screen_ainfo(screen, &ainfo);
	err = pdr_open_ifile(pd, picname, &ifile, &ainfo);
	if(err < Success) {
		free_pdr(&pd);
		return err;
	}

	if (ainfo.depth > 8)			/* only the CONVERT program can load */
	{								/* RGB/truecolor pictures; tell the  */
		err = Err_rgb_convert;		/* user that. */
		pdr_close_ifile(&ifile);
		free_pdr(&pd);
		return err;
	}

	screen = center_virtual_rcel(screen, &virt, ainfo.width, ainfo.height);

	err = pdr_read_first(ifile, screen);

	pdr_close_ifile(&ifile);
	free_pdr(&pd);
	return err;
}
