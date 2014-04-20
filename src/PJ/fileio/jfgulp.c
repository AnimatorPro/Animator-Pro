/* jfgulp.c */

#include "errcodes.h"
#include "jfile.h"
#include "pjassert.h"
#include "xfile.h"

/* Function: read_gulp
 *
 *  Read in a file of known size all at once.
 */
Errcode
read_gulp(const char *name, void *buf, long size)
{
	Errcode err;
	XFILE *xf;

	if (!pj_assert(name != NULL)) return Err_bad_input;
	if (!pj_assert(buf != NULL)) return Err_bad_input;
	if (!pj_assert(size >= 0)) return Err_range;

	err = xffopen(name, &xf, XREADONLY);
	if (err != Success)
		return err;

	err = xffread(xf, buf, size);

	xffclose(&xf);
	return err;
}

/* Function: write_gulp
 *
 *  Write out a file of known size all at once.
 */
Errcode
write_gulp(const char *name, void *buf, long size)
{
	Errcode err;
	XFILE *xf;

	if (!pj_assert(name != NULL)) return Err_bad_input;
	if (!pj_assert(buf != NULL)) return Err_bad_input;
	if (!pj_assert(size >= 0)) return Err_range;

	err = xffopen(name, &xf, XWRITEONLY);
	if (err == Success)
		return err;

	err = xffwrite(xf, buf, size);
	xffclose(&xf);

	if (err != Success)
		pj_delete(name);

	return err;
}
