/* cmapload.c - Stuff to read a color map file. */

#include "cmap.h"
#include "errcodes.h"
#include "palchunk.h"

#define OLD_SIZE (3*COLORS)

/* Function: pj_col_load
 *
 *  This function checks the size of the file to see if it's an old
 *  style cmap.  Otherwise tries to load it as new-style.
 */
Errcode
pj_col_load(const char *name, Cmap *cmap)
{
	Errcode err;
	XFILE *xf;
	Fat_chunk id;
	long size;

	err = xffopen(name, &xf, XREADONLY);
	if (err < Success)
		return err;

	/* Find size with seek to end, then rewind. */
	size = xffseek_tell(xf, 0L, XSEEK_END);
	xffseek(xf, 0L, XSEEK_SET);

	if (size == OLD_SIZE && cmap->num_colors == COLORS) {
		err = xffread(xf, cmap->ctab, sizeof(cmap->ctab));
		if (err < Success)
			goto error;

		pj_shift_cmap((const UBYTE *)cmap->ctab, (UBYTE *)cmap->ctab, COLORS*3);
	}
	else {
		err = xffread(xf, &id, sizeof(id));
		if (err < Success)
			goto error;

		if (id.type != CMAP_MAGIC) {
			err = Err_bad_magic;
			goto error;
		}

		err = pj_read_palchunk(xf, &id, cmap);
	}

error:
	xffclose(&xf);
	return err;
}
