#include "jfile.h"

Errcode pj_closez(Jfile *pjf)
{
Errcode err;

	if((err = pj_close(*pjf)) >= Success)
		*pjf = JNONE;
	return(err);
}
