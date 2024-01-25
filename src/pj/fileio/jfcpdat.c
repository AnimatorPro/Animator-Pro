/* jfcpdat.c */

#include "errcodes.h"
#include "jfile.h"
#include "memory.h"
#include "pjassert.h"
#include "xfile.h"

/* Function: pj_copydata
 *
 *  Copies size bytes from src to dst, starting from the current
 *  position in each file.
 */
Errcode
pj_copydata(XFILE *src, XFILE *dst, size_t size)
{
	Errcode err;
	char sbuf[256]; /* stack buffer */
	char *buf = NULL;
	size_t blocksize;

	if (!pj_assert(src != NULL)) return Err_bad_input;
	if (!pj_assert(dst != NULL)) return Err_bad_input;

	blocksize = 32L*1024;
	if (blocksize > size)
		blocksize = size;

	if (blocksize > sizeof(sbuf))
		buf = pj_malloc(blocksize);

	if (buf == NULL) {
		buf = sbuf;
		blocksize = sizeof(sbuf);
	}

	err = Success;
	while (size > 0) {
		if (blocksize > size)
			blocksize = size;

		err = xffread(src, buf, blocksize);
		if (err < Success)
			break;

		err = xffwrite(dst, buf, blocksize);
		if (err < Success)
			break;

		size -= blocksize;
	}

	if (buf != sbuf)
		pj_free(buf);
	return err;
}

/* Function: pj_copydata_oset */
Errcode
pj_copydata_oset(XFILE *src, XFILE *dst, LONG soset, LONG doset, size_t size)
{
	Errcode err;

	if (!pj_assert(src != NULL)) return Err_bad_input;
	if (!pj_assert(dst != NULL)) return Err_bad_input;

	err = xffseek(src, soset, XSEEK_SET);
	if (err < Success)
		return err;

	err = xffseek(dst, doset, XSEEK_SET);
	if (err < Success)
		return err;

	return pj_copydata(src, dst, size);
}
