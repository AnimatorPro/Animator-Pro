/***************************************************************
RND file pdr modules:

	Created by Peter Kennard.  Oct 11, 1991
		These modules implement reading scan line and polygon data in
		256 color Autoshade render slide files and drawing the image into
		a screen.
****************************************************************/
#include "errcodes.h"
#include "rnd.h"

/*** a bunch of basic read and write byte file calls that use a rfile's 
 *** file handle.  These handle the pict file basic data types ***/

#ifdef SLUFFED 
/**** write *****/
Errcode rf_write(Rfile *rf, void *buf, int size)
/* write bytes to a rfile's handle */
{
	if(fwrite(buf,1,size,rf->file) != size)
		return(rf->lasterr = pj_errno_errcode());
	return(Success);
}
Errcode rf_write_oset(Rfile *rf, void *buf, int size, int offset)
/* write after seeking to an offset in a rfile's handle */
{
	if(fseek(rf->file,offset,SEEK_SET) < 0)
	 	goto error; 
	if(fwrite(buf,1,size,rf->file) != size)
		goto error;
	return(Success);
error:
	return(rf->lasterr = pj_errno_errcode());
}
#endif /* SLUFFED */

/**** read *****/
Errcode rf_read(Rfile *rf,void *buf,LONG size)
{
	if(fread(buf,1,size,rf->file) != size)
		return(rf->lasterr = pj_errno_errcode());
	return(Success);
}
Errcode rf_seek(Rfile *rf, LONG count,int whence)
/* skips forward a number of bytes using seek */
{
	if(fseek(rf->file,count, whence) < 0)
		return(rf->lasterr = pj_errno_errcode());
	return(Success);
}
Errcode rf_read_oset(Rfile *rf,void *buf,LONG size,LONG offset)
{
	if(fseek(rf->file,offset,SEEK_SET) < 0)
		return(rf->lasterr = Err_seek); /* pj_errno_errcode()); */
	if(fread(buf,1,size,rf->file) != size)
		return(rf->lasterr = pj_errno_errcode());
	return(Success);
}
void freez(void *pmem)
/* free an item and set the pointer to it to NULL really gets passed
 * in a void ** */
{
void *mem;

	if((mem = *((void **)pmem)) != NULL)
		free(mem);
	*((void **)pmem) = NULL;
}
int strncmp(char *as,char *bs,int sz)
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