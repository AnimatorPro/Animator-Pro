#ifndef COMPRESS_H
#define COMPRESS_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif


#define IOBLEN	 32000		   /* I/O buffer length */

/* opcodes for ega type compression */

#define ENDBUF	    0		   /* End of buffer */
#define ENDLINE     1		   /* End of line */
#define ZERORUN     2		   /* Zero run */
#define NZRUN	    3		   /* Nonzero run */
#define UCDAT	    4		   /* Uncompressed stream */
#define ENDROW	    5		   /* End of colour row */
#define UCSING	    6		   /* Single uncompressed byte */
#define QNZRUN	    0x80	   /* Quick nonzero run */
#define QZRUN	    0x40	   /* Quick zero run */
#define QNCOMP	    0x20	   /* Quick uncompressed string */






#endif /* COMPRESS_H Leave at end of file */
