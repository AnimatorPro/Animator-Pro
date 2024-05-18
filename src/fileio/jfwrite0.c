/* jfwrite0.c */

#include "errcodes.h"
#include "jfile.h"
#include "memory.h"
#include "pjassert.h"

/* Function: pj_write_zeros */
Errcode
pj_write_zeros(XFILE *xf, LONG oset, ULONG bytes)
{
	Errcode err;
	char sbuf[256];	/* stack buffer */
	char *buf = NULL;
	size_t blocksize;

	if (!pj_assert(xf != NULL)) return Err_bad_input;

	blocksize = 16L*1024;
	if (blocksize > bytes)
		blocksize = bytes;

	if (blocksize > sizeof(sbuf))
		buf = pj_zalloc(blocksize);

	if (buf == NULL) {
		buf = sbuf;
		blocksize = sizeof(sbuf);
		clear_mem(sbuf, sizeof(sbuf));
	}

	err = Success;
	while (bytes > 0) {
		if (blocksize > bytes)
			blocksize = bytes;

		err = xffwriteoset(xf, buf, oset, blocksize);
		if (err < Success)
			break;

		oset += blocksize;
		bytes -= blocksize;
	}

	if (buf != sbuf)
		pj_free(buf);
	return err;
}
