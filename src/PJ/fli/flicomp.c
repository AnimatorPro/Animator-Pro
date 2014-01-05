#include "fli.h"
#include "flicomp.h"
#include "memory.h"
#include "ptrmacro.h"
#include "rastcomp.h"

LONG pj_fli_comp_rect(void *comp_buf,
					Rcel *last_screen,
					Rcel *this_screen, Rectangle *rect,
					Boolean do_colors, SHORT frame_type, Flicomp comp_lib)

/* given two screens, or one in the case of the types FLI_BRUN and FLI_COLOR_0
 * compresses the delta between them or the data in the one and puts
 * resulting record in *comp_buf,
 * returns length of compressed buffer including a prepended and loaded
 * Chunk_id this does not clip the rectangle with the screen and rectangle
 * must be within both screens provided */
{
void *vp;
Fli_frame *frame;
Chunk_id *chunk;
extern void *pj_fccomp();
LONG stamp_size;



	frame = (Fli_frame *)comp_buf;
	pj_stuff_bytes(0, frame, sizeof(*frame));
	chunk = (Chunk_id *)(frame+1);
	stamp_size = 0;

	if(do_colors)
	{
		switch(frame_type)
		{
			case COMP_FIRST_FRAME:
			{
				stamp_size = (*(comp_lib->make_pstamp))(this_screen, chunk,
													rect->x,rect->y,
													rect->width,rect->height);
				if(stamp_size)
				{
					++frame->chunks;
					chunk = OPTR(chunk,stamp_size);
				}
			}
			case COMP_BLACK_FIRST:
			{
				vp = (*(comp_lib->first_colors))(this_screen->cmap->ctab,
											  chunk+1,COLORS);
				chunk->type = comp_lib->first_color_type;
				break;
			}
			default:
			{
				if(last_screen == NULL) /* protection from bad input */
					return(0);

				vp = (*(comp_lib->delta_colors))(last_screen->cmap->ctab,
											  this_screen->cmap->ctab,
											  chunk+1, COLORS);

				chunk->type = comp_lib->delta_color_type;
				break;
			}
		}

		chunk->size = SIZE(chunk,vp);

		if (chunk->size == EMPTY_DCOMP)
			vp = chunk;
		else
		{
			chunk = vp;
			frame->chunks += 1;
		}
	}
	else /* no color chunk requested */
	{
		vp = chunk;
	}

	switch(frame_type)
	{
		case COMP_DELTA_FRAME:
			vp = (*(comp_lib->delta_comp))((Raster *)last_screen,
											chunk+1, rect->x,rect->y,
											(Raster *)this_screen,
											rect->x,rect->y,
											rect->width,rect->height);
			chunk->type = comp_lib->delta_type;
			break;
		case COMP_FIRST_FRAME:
			vp = (*(comp_lib->first_comp))((Raster *)this_screen, chunk+1,
											rect->x,rect->y,
											rect->width,rect->height);
			chunk->type = comp_lib->first_type;
			break;
		case COMP_BLACK_FIRST: /* special case for a color 0 first frame */
			chunk->type = FLI_COLOR_0;
			vp = chunk+1;
			break;
	}

	if(vp == NULL)
	{
		vp = chunk+1;
		pj_get_rectpix(this_screen,vp,
					rect->x,rect->y,rect->width,rect->height);

		chunk->type = FLI_COPY;
		chunk->size = rect->width * rect->height;;
		vp = FOPTR(vp,chunk->size);
		chunk->size += sizeof(chunk);
	}
	else
	{
		chunk->size = SIZE(chunk,vp);
	}

	if(chunk->size & 1) /* force even allignment */
	{
		++chunk->size;
		vp = OPTR(vp,1);
	}

	if (chunk->size == EMPTY_DCOMP)
		vp = chunk;
	else
		++frame->chunks;

	frame->type = FCID_FRAME;
	frame->size = SIZE(comp_buf,vp);
	return(frame->size);
}
LONG pj_fli_comp_cel(void *comp_buf,
					Rcel *last_screen,
					Rcel *this_screen, SHORT frame_type, Flicomp comp_type)

/* shell around fli_comp_rect() that uses the size of the screen as the
 * rectangle to compress. assumes both screens the same size */
{
Rectangle rect;

	rect.x = 0;
	rect.y = 0;
	rect.width = this_screen->width;
	rect.height = this_screen->height;

	return(pj_fli_comp_rect(comp_buf,last_screen,this_screen,&rect,TRUE,
						 frame_type, comp_type));
}
