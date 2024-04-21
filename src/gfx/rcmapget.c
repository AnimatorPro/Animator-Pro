#include "cmap.h"
#include "errcodes.h"
#include "memory.h"
#include "ptrmacro.h"

void pj_cmap_free(Cmap *cmap)
{
	if(cmap == NULL)
		return;
	pj_free(cmap);
}
Errcode pj_cmap_alloc(Cmap **pcm, LONG num_colors)
{
	if((*pcm=pj_malloc(OFFSET(Cmap,ctab)+(num_colors*sizeof(Rgb3)))) == NULL)
		return(Err_no_memory);
	(*pcm)->num_colors = num_colors;
	return(Success);
}

