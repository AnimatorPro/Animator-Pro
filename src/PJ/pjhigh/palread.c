#include "cmap.h"
#include "errcodes.h"
#include "palchunk.h"
#include "ptrmacro.h"

Errcode pj_read_palchunk(Jfile fd, Fat_chunk *id,Cmap *cmap)

/* assuming leading fat_chunk is read in already this reads the rest of the
 * data.  Designed for use with chunk parser, will not neccessarily read 
 * all the data */
{
LONG ssize;
LONG dsize;

	if(id->version != PAL_RGB256_VERS)
		return(Err_version);

	ssize = id->size - sizeof(Fat_chunk);
	if(ssize < sizeof(Rgb3) || ssize % sizeof(Rgb3))
		return(Err_corrupted);

	dsize = cmap->num_colors * sizeof(Rgb3);
	if(ssize > dsize)
		ssize = dsize;
	return(pj_read_ecode(fd,cmap->ctab,dsize));
}

