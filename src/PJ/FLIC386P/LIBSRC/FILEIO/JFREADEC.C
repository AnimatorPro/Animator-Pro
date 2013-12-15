#include "errcodes.h"
#include "jfile.h"

Errcode pj_read_ecode(Jfile f, void *buf, LONG size)
/* jread that returns errorcode */
{
	if(pj_read(f,buf,size) < size)
		return(pj_ioerr());
	return(Success);
}
