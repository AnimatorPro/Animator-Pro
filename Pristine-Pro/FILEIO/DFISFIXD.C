#include "dfile.ih"

Errcode pj_is_fixed(char *device)

/* returns 1 if device is fixed 0 if not < 0 if error */
{
char dc;

	dc = toupper(*device);
	if(dc == 'A' || dc == 'B')
		return(0);
	return(1);
}
