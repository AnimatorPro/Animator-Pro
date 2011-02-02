#include "fli.h"

int fli_wrap_frame(Flifile *flif,int frame)
/* Force a frame index to be inside the fli or flx (between 0 and count - 1)  */
{
	if((frame = frame % flif->hdr.frame_count) < 0)
		frame += flif->hdr.frame_count;
	return(frame);
}
