#include "stdtypes.h"
#include "errcodes.h"
#include "softmenu.h"

int get_errtext(Errcode err, char *buf)
/* this loads a buffer with text for an error returns length of string */
{
extern long pj_mem_last_fail;
char sbuf[ERRTEXT_SIZE];
int serr;

	if(err >= Success || err == Err_abort || err == Err_reported)
		goto no_message;


	sbuf[0] = 0;	/* in case of failure to find text... */
	err = -err;		/* user/message file thinks it's positive. */
	if ((serr = smu_get_errtext(&smu_sm, "errors", err, sbuf)) >= Success)
	{
		if(err == -Err_no_memory)
			return(snftextf(buf,ERRTEXT_SIZE,"!%d",sbuf,pj_mem_last_fail));
		else
			return(sprintf(buf, "%s", sbuf));

		/* return(sprintf(buf, "E%d %s", err, sbuf)); */
	}
	else
		return(sprintf(buf,	"Error code %d", err));

no_message:
	*buf = 0;
	return(0);

}
