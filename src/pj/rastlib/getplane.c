#include "memory.h"
#include "raster.h"
#include "errcodes.h"

/******* subroutines for RamRast open and close routines ******/

#define get_bplane(sz) pj_malloc(sz)
#define free_bplane(pt) pj_free(pt)

void pj_free_bplanes(PLANEPTR *bp, LONG num_planes)

/* frees bp in plane array, will set pointers to NULL when freed 
 * will not atempt to free NULL bp **/
{
	while(num_planes > 0)
	{
		--num_planes;
		if(*bp != NULL)
		{
			free_bplane(*bp);
			*bp = NULL;
		}
		++bp;
	}
}

LONG pj_get_bplanes(PLANEPTR *bp,LONG num_planes,LONG bpize)

/* allocate memory bp for bitbp or bytebp etc. will ovewrite
 * anything existing in PLANEPTR array returns number gotten or errcode */
{
LONG pcount = 0;

	while(pcount < num_planes)
	{
		if(NULL == (*bp = get_bplane(bpize)))
		{
			pj_free_bplanes((bp-pcount),pcount);
			return(Err_no_memory);
		}
		++bp;
		++pcount;
	}
	return(pcount);
}
