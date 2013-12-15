#include "errcodes.h"
#include "memory.h"
#include "fli.h"

Errcode pj_fli_alloc_cbuf(Fli_frame **pcbuf, USHORT width,USHORT height, 
				   LONG num_colors)
/************************************************************************* 
 * Calculates compression buffer size needed and allocates memory 
 * for it in *pcbuf.  This should have a matching pj_freez(pcbuf)
 * somewhere.
 *
 * Parameters:
 *		Fli_frame 	**pcbuf;		Returns buffer big enough for a frame.
 *		USHORT		width;			Fli width.
 *		USHORT		height;			Fli height;
 *		LONG		num_colors;		# of colors in color map (256 mostly)
 * Returns:
 *		Success if all goes well, negative error code (see errcodes.h) if
 *		there's trouble.
 *************************************************************************/
{
	if(NULL == (*pcbuf = pj_zalloc(pj_fli_cbuf_size(width,height,num_colors))))
		return(Err_no_memory);
	return(Success);
}

Errcode pj_fli_cel_alloc_cbuf(Fli_frame **pcbuf, Rcel *cel)
/* same as fli_alloc_cbuf() but allocs a cbuf sized for an rcel screen */
{
	return(pj_fli_alloc_cbuf(pcbuf,cel->width,cel->height,cel->cmap->num_colors));
}
