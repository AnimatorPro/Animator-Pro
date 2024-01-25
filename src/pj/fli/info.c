#include "errcodes.h"
#include "jfile.h"
#include "fli.h"
#include "animinfo.h"

Errcode pj_fli_info(char *path, Anim_info *ainfo)
{
Errcode err;
Flifile flif;

	if((err = pj_fli_info_open(&flif, path, ainfo)) >= Success)
		pj_fli_close(&flif);
	return(err);
}
