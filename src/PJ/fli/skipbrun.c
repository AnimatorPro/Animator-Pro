
#define RASTCOMP_INTERNALS
#include "ptrmacro.h"
#include "rastcomp.h"

void *pj_unbrun_skip_line(BYTE *src, Coor sw)
/* skips one brunned line in source buffer returns pointer to next "chunk" */
{
int psize;

	++src;	/* skip over obsolete opcount byte */
	while(sw > 0)
	{
		if((psize = *src++) < 0)
		{
			src -= psize;
			sw += psize;
		}
		else
		{
			++src;
			sw -= psize;
		}
	}
	return(src);
}
