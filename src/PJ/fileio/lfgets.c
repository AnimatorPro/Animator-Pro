#include "lfile.ih"

char *lfgets(char *buf,int max, LFILE *f)
{
int c;
char *s = buf;

if ((c = lgetc(f)) < Success)
	return(NULL);
if ((*s++ = c) == '\n')
	goto OUT;
max -= 2;
while ( --max >= 0)
	{
	if ((c = lgetc(f)) < Success)
		goto OUT;
	if ((*s++ = c)  == '\n')
		goto OUT;
	}
OUT:
*s++ = 0;
return(buf);
}
