/* jfcpfile.c */

#include "jimk.h"
#include "errcodes.h"
#include "jfile.h"
#include "memory.h"
#include "xfile.h"

/* Function: pj_cpfile
 *
 *  Copy a file.
 *
 *  opt_errfile is set to Success if the file is copied successfully,
 *  Err_read if an error occurred while reading from src, or
 *  Err_write if an error occurred while writing to dst.
 */
Errcode
pj_cpfile(const char *src, const char *dst, Errcode *opt_errfile)
{
	Errcode err;
	XFILE *s = NULL;
	XFILE *d = NULL;
	size_t size;
	char sbuf[256]; /* stack buffer */
	char *buf = sbuf;
	size_t blocksize;

	err = xffopen(src, &s, XREADONLY);
	if (err != Success)
		goto read_error;

	err = xffopen(dst, &d, XWRITEONLY);
	if (err != Success)
		goto write_error;

	blocksize = PJ_COPY_FILE_BLOCK;
	buf = trd_laskmem(blocksize);
	if (buf == NULL) {
		blocksize = sizeof(sbuf);
		buf = sbuf;
	}

	for (;;) {
		size = xfread(buf, 1, blocksize, s);

		err = xffwrite(d, buf, size);
		if (err < 0)
			goto write_error;

		if (size < blocksize)
			break;
	}

	if (opt_errfile != NULL)
		*opt_errfile = Success;

	err = Success;
	goto cleanup;

read_error:
	if (opt_errfile != NULL)
		*opt_errfile = Err_read;
	goto cleanup;

write_error:
	if (opt_errfile != NULL)
		*opt_errfile = Err_write;
	goto cleanup;

cleanup:
	if (s != NULL)
		xffclose(&s);

	if (d != NULL)
		xffclose(&d);

	if (buf != sbuf)
		trd_freemem(buf);

	return err;
}

/* Function: pj_copyfile
 *
 *  Copy a file.  Report errors except for source file not existing.
 */
Errcode
pj_copyfile(const char *src, const char *dst)
{
	Errcode err;
	Errcode errfile;

	err = pj_cpfile(src, dst, &errfile);
	if (err != Success && err != Err_no_file)
		err = errline(err, "%s", (errfile == Err_read) ? src : dst);

	return err;
}
