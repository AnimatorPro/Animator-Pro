#ifndef JFILE_H
#define JFILE_H

#include <stdio.h>

/* Function: jopen
 *
 *  Open a file to read/write/read-write depending on mode.
 *  Returns 0 on failure, otherwise file handle.
 *
 *  mode - (0) read, (1) write, (2) read-write.
 */
extern FILE *jopen(const char *title, int mode);

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
extern unsigned int jread(FILE *f, void *buf, unsigned int size);

/* Function: jseek
 *
 *  Seek to a long offset.  Return file position on success, -1 on failure.
 *
 *  mode - relative to (0) start, (1) current position, (2) end of file.
 */
extern long jseek(FILE *f, long offset, int mode);

#endif
