#include "palchunk.h"

Errcode pj_col_save(char *name,Cmap *cmap)
/* Save color map file */
{
Errcode err;
Jfile fd; 

	if((fd = pj_create(name, JREADWRITE)) == JNONE)
		return(pj_ioerr());
	err = pj_write_palchunk(fd,cmap,CMAP_MAGIC);
	pj_close(fd);
	return(err);
}

