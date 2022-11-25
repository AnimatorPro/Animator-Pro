#include "jimk.h"
#include "commonst.h"
#include "scroller.h"

Boolean qscroller(
	char *result,			/* Put string user selects here */
	char *hailing,  		/* Character string for move/title area */
	Names *items, 		/* List of things to put in scroller */
	SHORT lines,			/* # of lines visible in scroller.  10 is good */
	SHORT *ipos)			/* Initial scroller position */
/* Return index of element selected, negative Errcode if CANCEL or
   out of memory or something. */
{
Errcode err;
Menuhdr *qc;

	if((err = build_qscroller(result, vb.screen, &qc, hailing,
		items,lines,ok_str,cancel_str,ipos)) >= Success)
	{
		err = do_reqloop(vb.screen,qc,qc->mbs,NULL,NULL);
		cleanup_qscroller(qc, ipos);
	}
	if (err < Success)
	{
		softerr(err,NULL);
		return(FALSE);
	}
else
	return(TRUE);
}
