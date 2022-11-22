
#include "errcodes.h"
#include "jfile.h"

Errcode pj_write_ecode(Jfile f, void *buf, LONG size)
/* jwrite that returns errorcode */
{
	if(pj_write(f,buf,size) < size)
		return(pj_ioerr());
	return(Success);
}
