#include "rectang.h"

void bclip_dim(SHORT *pos,USHORT *len, SHORT minpos, SHORT maxlen)
{
SHORT overedge;

	if(*len > maxlen)
		*len = maxlen;

	if(*pos < minpos)
		*pos = minpos;
	else
	{
		if(0 < (overedge = (*pos + *len) - (minpos + maxlen)))
			*pos -= overedge;
	}
}
