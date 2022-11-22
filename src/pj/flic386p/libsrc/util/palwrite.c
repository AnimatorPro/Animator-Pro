#include "ptrmacro.h"
#include "palchunk.h"

Errcode pj_write_palchunk(Jfile fd, Cmap *cmap, SHORT id_type)

/* writes a palette chunk to the file input for the colormap input */
{
Errcode err;
LONG csize;
Fat_chunk id;

   	csize = (cmap->num_colors * sizeof(Rgb3));
	id.type = id_type;
	id.version = PAL_RGB256_VERS;
	id.size = sizeof(id) + csize;
	if((err = pj_write_ecode(fd,&id,sizeof(id))) < Success)
		return(err);
	return(pj_write_ecode(fd,cmap->ctab,csize));
}
