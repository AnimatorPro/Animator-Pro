#include "cmap.h"
#include "errcodes.h"
#include "palchunk.h"
#include "ptrmacro.h"

/* Function: pj_read_palchunk
 *
 *  Assuming leading fat_chunk is read in already, this reads the rest
 *  of the data.  Designed for use with chunk parser, will not
 *  necessarily read all the data.
 */
Errcode
pj_read_palchunk(XFILE *xf, Fat_chunk *id, Cmap *cmap)
{
	LONG ssize;
	LONG dsize;

	if (id->version != PAL_RGB256_VERS)
		return Err_version;

	ssize = id->size - sizeof(Fat_chunk);
	if (ssize < (LONG)sizeof(Rgb3) || (ssize % sizeof(Rgb3)) != 0)
		return Err_corrupted;

	dsize = cmap->num_colors * sizeof(Rgb3);
	if (ssize > dsize)
		ssize = dsize;

	return xffread(xf, cmap->ctab, dsize);
}
