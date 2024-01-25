#ifndef REXLOAD_H
#define REXLOAD_H

#ifndef PLEXP_H
	#include "plexp.h"
#endif

#define	 REX_BLK_SIZE  512	/* size of rex block */
#define  REL32	0x80000000	/* if bit set: relocate 32 bit value
					   		   if reset  : relocate 16 bit value */

/* Compute number of pages needed to fit "val" bytes of memory */

#define	MIN_PAGES(val) (val ? (( (val-1) >> PAGE_SHIFT) +1):0)

/* structure of data found in the entry point of a pj style rex library */

#define PJREX_MAGIC 0x9abcdef0
#define PJREX_MAGIC2 0x0def0
#define PJREX_BETAVERS 0x9abc
#define PJREX_VERSION 0

typedef struct rexlib_entry {
	USHORT safety; 	/* actually two return instructions 0xc3c3 */

	ULONG magic;    /* set to PJREX_MAGIC */

	void *header_data; /* pointer to "rexlib_header" in rex library code */

	USHORT magic2;  /* set to PJREX_MAGIC2 */  

	USHORT version; /* set to PJREX_VERSION */

	ULONG data_end; /* Set to offset of symbol "_end" in pharlap and watcom
				     * linkers, "_end" is specified as the location of the end 
					 * of the uninitialized data segment which is the maximum 
					 * address we have to load.  If you want it to load to the
					 * next even 4K page as specified by the linker in the
					 * rex file header set this to 0.  
					 *
					 * This value will eliminate page padding in the 
					 * loader by specifying the total number of bytes to 
					 * allocate for the rex module. It will override the 
					 * last potentially padded page of uninitialized data
					 * specified in the rex header if it is smaller. */
} Rexlib_entry;

Errcode pj_open_rex(char *path, Jfile *pfile, EXP_HDR *hdrsp);

#endif /* REXLOAD_H */
