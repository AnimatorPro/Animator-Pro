/* jfinsert.c */

#include "errcodes.h"
#include "jfile.h"
#include "memory.h"
#include "pjassert.h"

/* Function: pj_insert_space
 *
 *  Increases file size by gapsize, and copys all data in file at
 *  offset toward end of file by gapsize, leaves current position at
 *  offset start of gap unless there is an error.
 */
Errcode
pj_insert_space(XFILE *xf, LONG offset, LONG gapsize)
{
	Errcode err;
	long cpos;
	long oldend;

	if (!pj_assert(xf != NULL)) return Err_bad_input;

	cpos = xffseek_tell(xf, offset, XSEEK_SET);
	if (cpos < 0)
		return (Errcode)cpos;

	oldend = xffseek_tell(xf, 0, XSEEK_END);
	if (oldend < 0)
		return (Errcode)oldend;

	/* Extend end of file. */
	err = pj_write_zeros(xf, oldend, gapsize);
	if (err < Success)
		return err;

	/* Copy tail of file from cpos to cpos + gapsize. */
	err = copy_in_file(xf, oldend - cpos, cpos, cpos + gapsize);
	if (err < Success)
		return err;

	/* Seek to start of gap. */
	err = xffseek(xf, cpos, XSEEK_SET);
	if (err < Success)
		return err;

	return Success;
}
