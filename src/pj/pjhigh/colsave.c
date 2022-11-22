#include "palchunk.h"

/* Function: pj_col_save
 *
 *  Save colour map file.
 */
Errcode
pj_col_save(const char *name, struct cmap *cmap)
{
	Errcode err;
	XFILE *xf;

	err = xffopen(name, &xf, XREADWRITE_CLOBBER);
	if (err < Success)
		return err;

	err = pj_write_palchunk(xf, cmap, CMAP_MAGIC);

	xffclose(&xf);
	return err;
}

