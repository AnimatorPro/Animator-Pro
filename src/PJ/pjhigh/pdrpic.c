/* this is a local pdr s that are set up for linking in to the code
 * and used by the pdr managing routines */

#include "errcodes.h"
#include "picfile.h"
#include "picdrive.h"
#include "pjbasics.h"


Errcode pdr_load_picture(char *pdr_path,char *picname, Rcel *screen)
{
Errcode err;
Pdr *pd;
Image_file *ifile;
Rcel virt;
Anim_info ainfo;

	if((err = load_pdr(pdr_path, &pd)) < Success)
		return(cant_use_module(err,pdr_path));

	get_screen_ainfo(screen,&ainfo);
	if((err = pdr_open_ifile(pd, picname, &ifile, &ainfo)) < Success)
		goto error;

	if (ainfo.depth > 8)			/* only the CONVERT program can load */
	{								/* RGB/truecolor pictures; tell the  */
		err = Err_rgb_convert;		/* user that. */
		goto closefile;
	}

	screen = center_virtual_rcel(screen, &virt, ainfo.width, ainfo.height);

	err = pdr_read_first(ifile,screen);

closefile:
	pdr_close_ifile(&ifile);

error:
	free_pdr(&pd);
	return(err);
}
