#include "errcodes.h"
#include "memory.h"
#include "jfile.h"

Errcode pj_copydata_oset(Jfile src, Jfile dest, 
						LONG soset, LONG doset, ULONG size)
{
long lerr;

	if((lerr = pj_seek(src,soset,JSEEK_START)) < 0)
		return(lerr);
	if((lerr = pj_seek(dest,doset,JSEEK_START)) < 0)
		return(lerr);
	return(pj_copydata(src,dest,size));
}
