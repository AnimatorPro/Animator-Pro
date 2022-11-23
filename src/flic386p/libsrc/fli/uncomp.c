#include "rastcall.h"
#include "fli.h"
#include "ptrmacro.h"

void pj_fli_uncomp_rect(Rcel *f, 
	             struct fli_frame *frame, Rectangle *rect,
			     int colors)	/* update color registers? */

/* Ok, now we've read in a frame of a FLI file ok.  Given a raster that
   has the last frame on it and the data for this frame, this function
   will switch through the chunks of the frame data updating the screen
   in the process.  The 'colors' parameter indicates whether we should
   update the hardware color map as well as the ram echo. this is NOT clipped
   to the cel size and the rectangle must be contained within the cel 
   it simply applys an x and y offset to the position of the result */
{
int j;
void *vp;
#define CHUNK ((Chunk_id *)vp)

	vp = frame+1;
	for (j=0;j<frame->chunks;j++)
	{
		switch(CHUNK->type)
		{
			case FLI_COLOR:
				if(colors)
				{
					pj_wait_rast_vsync(f);
					pj_uncc64(f, CHUNK+1);
				}
				pj_fcuncomp64(CHUNK+1,f->cmap->ctab);
				break;
			case FLI_COLOR256:
				if(colors)
				{
					pj_wait_rast_vsync(f);
					pj_uncc256(f, CHUNK+1);
				}
				pj_fcuncomp(CHUNK+1,f->cmap->ctab);
				break;
			case FLI_LC:
				pj_unlccomp_rect(f,CHUNK+1,1,
							  rect->x,rect->y,rect->width,rect->height);
				break;
			case FLI_SS2:
				pj_unss2_rect(f,CHUNK+1,1,
						   rect->x,rect->y,rect->width,rect->height);
				break;
			case FLI_COLOR_0:
				pj__set_rect(f,0,rect->x,rect->y,rect->width,rect->height);
				break;
			case FLI_BRUN:
				pj_unbrun_rect(f,CHUNK+1,1,
							rect->x,rect->y,rect->width,rect->height);
				break;
			case FLI_COPY:
				_pj_put_rectpix(f,CHUNK+1,
							   rect->x,rect->y,rect->width,rect->height);
				break;
		}
		vp = FOPTR(vp,CHUNK->size);
	}
#undef CHUNK
}
void pj_fli_uncomp_frame(Rcel *screen, 
	              	  Fli_frame *frame, 
			      	  int colors)	/* update color registers? */

/* does an fli_uncomp_rect() for entire screen size */
{
Rectangle rect;

	rect.x = 0;
	rect.y = 0;
	rect.width = screen->width;
	rect.height = screen->height;
	pj_fli_uncomp_rect(screen,frame,&rect,colors);
}
