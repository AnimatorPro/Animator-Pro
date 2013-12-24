/***************************************************************
Autodesk Movie file pdr modules:

	Created by Peter Kennard.  Oct 5, 1991
		These modules implement a PDR for Autodesk movie file compressed
		EGA pixel animations.  Slide records, buttons looping and other
		functions of movie files are not implemented.
****************************************************************/
#include "errcodes.h"
#include "movie.h"

/*** a bunch of basic read and write byte file calls that use a mfile's 
 *** file handle.  These handle the pict file basic data types ***/

#ifdef SLUFFED
/**** write *****/
Errcode mf_write(Mfile *mf, void *buf, int size)
/* Write bytes to a mfile's handle */
{
	if(fwrite(buf,1,size,mf->file) != size)
		return(mf->lasterr = pj_errno_errcode());
	return(Success);
}
Errcode mf_write_oset(Mfile *mf, void *buf, int size, int offset)
/* Write after seeking to an offset in a mfile's handle */
{
	if(fseek(mf->file,offset,SEEK_SET) < 0)
	 	goto error; 
	if(fwrite(buf,1,size,mf->file) != size)
		goto error;
	return(Success);
error:
	return(mf->lasterr = pj_errno_errcode());
}
#endif /* SLUFFED */

/**** read *****/
Errcode mf_read(Mfile *mf,void *buf,LONG size)
/* Read bytes from an Mfile's handle. */
{
	if(fread(buf,1,size,mf->file) != size)
		return(mf->lasterr = pj_errno_errcode());
	return(Success);
}
Errcode mf_seek(Mfile *mf, LONG count,int whence)
/* Seek using an mfile's handle. */
{
	if(fseek(mf->file,count, whence) < 0)
		return(mf->lasterr = pj_errno_errcode());
	return(Success);
}
Errcode mf_read_oset(Mfile *mf,void *buf,LONG size,LONG offset)
/* Read bytes from an mfile's handle at a specific offset. */
{
	if(fseek(mf->file,offset,SEEK_SET) < 0)
		return(mf->lasterr = Err_seek); /* pj_errno_errcode()); */
	if(fread(buf,1,size,mf->file) != size)
		return(mf->lasterr = pj_errno_errcode());
	return(Success);
}
void freez(void *pmem)
/* Free an item and set the pointer to it to NULL really gets passed
 * in a void **. */
{
void *mem;

	if((mem = *((void **)pmem)) != NULL)
		free(mem);
	*((void **)pmem) = NULL;
}
int strncmp(char *as,char *bs,int sz)
/* ansi strncmp function. */
{
register UBYTE a;
char *maxas;

	maxas = as + sz;
	for(;;)
	{
		if(as >= maxas)
			return(0);
		if((a = *as++) != *bs++)
			break;
		if(a == 0)
			break;
	}
	return(a-bs[-1]);
}
