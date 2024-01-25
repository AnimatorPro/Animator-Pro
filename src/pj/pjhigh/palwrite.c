#include "cmap.h"
#include "palchunk.h"
#include "ptrmacro.h"

/* Function: pj_write_palchunk
 *
 *  Writes a palette chunk to the file.
 */
Errcode
pj_write_palchunk(XFILE *xf, Cmap *cmap, SHORT id_type)
{
	Errcode err;
	LONG csize;
	Fat_chunk id;

	csize = (cmap->num_colors * sizeof(Rgb3));
	id.type = id_type;
	id.version = PAL_RGB256_VERS;
	id.size = sizeof(id) + csize;

	err = xffwrite(xf, &id, sizeof(id));
	if (err < Success)
		return err;

	return xffwrite(xf, cmap->ctab, csize);
}
