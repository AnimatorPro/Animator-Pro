#include "errcodes.h"
#include "fli.h"
#include "flicomp.h"


static Errcode fli_write_error(Errcode err, char *name)
{
	if(name)
		err = pj_fli_error_report(err,"Error writing FLC \"%s\"", name);
	return(err);
}
Errcode pj_i_add_next_rec(char *name, /* name for error reporting
										  * if NULL no reports */
							  Flifile *flif, Fli_frame *frame )
{
Errcode err;
LONG size = ((Fli_frame *)frame)->size;

	err = xffwrite(flif->xf, frame, size);
	if (err < Success) {
		if (err == Err_eof)
			err = Err_truncated;

		err = fli_write_error(err,name);

		/* attempt flush of header */
		pj_i_flush_head(flif);
		return(err);
	}
	flif->hdr.size += size;
	++flif->hdr.frame_count;
	return(Success);
}
Errcode pj_i_add_frame1_rec(char *name, /* name for error reporting
										  * if NULL no reports */
							   Flifile *flif, Fli_frame *frame )

/* writes first frame record of a fli assuming the output file is positioned
 * to the first frame's offset position it will write the record and set up
 * the Flifile header to recieve subsequent delta frames or a ring frame */
{
Errcode err;

	flif->hdr.size = flif->hdr.frame1_oset = xfftell(flif->xf);
	if(flif->hdr.size < 0)
		return(fli_write_error((Errcode)flif->hdr.size,name));
	flif->hdr.frame_count = 0;
	if((err = pj_i_add_next_rec(name,flif,frame)) < Success)
		return(err);

	flif->hdr.frame2_oset = flif->hdr.size; /* size set in fii_write_frame() */
	return(Success);
}
Errcode pj_i_add_ring_rec(char *name, /* name for error reporting
										  * if NULL no reports */
							  Flifile *flif, Fli_frame *frame )

/* Same as fli_add_ring() but writes a frame that has already been
 * compressed in the input buffer */
{
Errcode err;
int ocount;

	ocount = flif->hdr.frame_count;
	if((err = pj_i_add_next_rec(name,flif,frame)) < Success)
		goto error;
	flif->hdr.frame_count = ocount;
	flif->hdr.flags = (FLI_FINISHED | FLI_LOOPED);
	if((err = pj_i_flush_head(flif)) < Success)
		goto error;
	return(Success);
error:
	return(fli_write_error(err,name));
}
