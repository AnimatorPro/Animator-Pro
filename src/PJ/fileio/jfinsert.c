#include "errcodes.h"
#include "jfile.h"
#include "memory.h"

Errcode pj_insert_space(Jfile f,LONG offset, LONG gapsize)

/* increases file size by gapsize, and copys all data in file at offset
 * toward end of file by gapsize, leaves current
 * position at offset start of gap unless there is
 * an error */
{
Errcode err;
LONG cpos;
LONG oldend;

	if((cpos = pj_seek(f,offset,JSEEK_START)) < 0)
		return((Errcode)cpos);

	if((oldend = pj_seek(f,0,JSEEK_END)) < 0)
		return((Errcode)oldend);

	/* extend end of file */
	if((err = pj_write_zeros(f,oldend,gapsize)) < 0)
		return(pj_ioerr());

	/* copy tail of file from cpos to cpos + size */
	if((err = copy_in_file(f, oldend - cpos,cpos,cpos + gapsize))<0)
		return(err);

	if((cpos = pj_seek(f,cpos,JSEEK_START)) < 0)
		return((Errcode)cpos);
	return(Success);
}
