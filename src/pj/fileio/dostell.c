#include "msfile.h"
#include "filemode.h"

long pj_dtell(int f)
/* Reposition file pointer returns file position < 0 if error */
{
	return(pj_dseek(f, 0L, JSEEK_REL));
}
