#ifndef JFILE_H
#define JFILE_H

#include <stdio.h>

enum JReadWriteMode {
	JREADONLY   = 0,
	JWRITEONLY  = 1,
	JREADWRITE  = 2
};

enum JSeekMode {
	JSEEK_START = 0,
	JSEEK_REL   = 1,
	JSEEK_END   = 2
};

/* Function: jexists
 *
 *  Does file exist?
 */
extern int jexists(const char *title);

/* Function: jdelete
 *
 *  Delete a file if possible.  Don't complain if it's not there.
 */
extern int jdelete(const char *title);

/* Function: jcreate
 *
 *  Create a file to read/write.
 */
extern FILE *jcreate(const char *title);

/* Function: jopen
 *
 *  Open a file to read/write/read-write depending on mode.
 *  Returns 0 on failure, otherwise file handle.
 *
 *  mode - (0) read, (1) write, (2) read-write.
 */
extern FILE *jopen(const char *title, enum JReadWriteMode mode);

/* Function: jclose
 *
 *  Close file.
 */
extern void jclose(FILE *f);

/* Function: gentle_close
 *
 *  Close a non-zero file handle.
 */
extern void gentle_close(FILE *f);

/* Function: jread
 *
 *  Read file to a buffer.  Returns bytes of data successfully transfered.
 */
extern long jread(FILE *f, void *buf, long size);

/* Function: jwrite
 *
 *  Returns bytes of data successfully transfered.
 */
extern long jwrite(FILE *f, const void *buf, long size);

/* Function: jseek
 *
 *  Seek to a long offset.  Return file position on success, -1 on failure.
 *
 *  mode - relative to (0) start, (1) current position, (2) end of file.
 */
extern long jseek(FILE *f, long offset, enum JSeekMode mode);

#endif
