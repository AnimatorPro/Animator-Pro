#include "formatf.h"

int fa_sprintf(char *buf,int maxlen, Formatarg *fa)

/* a self limiting sprintf that limits output size to maxlen. If maxlen < 0
 * it doesn't limit. output will be terminated by a null if buffer
 * is stuffed to maxlen.  A maxlen of 0 will get one '\0' char */
{
	while((*buf++ = fa_getc(fa)) != 0)
	{
		if (fa->count >= maxlen)
		{
			buf[-1] = 0; /* null terminate */
			break;
		}
	}
	return(fa->count - 1);
}
