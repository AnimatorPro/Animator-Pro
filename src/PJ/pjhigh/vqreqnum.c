#include "ftextf.h"
#include "pjbasics.h"
#include "commonst.h"

extern Image *leftright_arrs[];

Boolean varg_qreq_number(SHORT *val,SHORT min, SHORT max,
					     Errcode (*update)(void *data, SHORT val),
					     void *ud_dat,char *formats,char *text,
					     va_list args)
{
Errcode err;
Menuhdr *mh;
char *okcan[3];
char sbuf[256];
char *tbuf = sbuf;

	okcan[0] = ok_str;
	okcan[1] = cancel_str;
	okcan[2] = NULL;

	if((err = get_formatted_ftext(&tbuf,sizeof(sbuf),formats,text,args,
								  FALSE )) < Success)
	{
		goto error;
	}
	if(tbuf)
		text = tbuf;

	if((err = build_qnumreq(icb.input_screen,&mh,text,okcan,leftright_arrs,
							*val,min,max,update,ud_dat)) < Success)
	{
		return(err);
	}

	mh->cursor = vb.screen->cursor;
	if((err = do_reqloop(vb.screen,mh,mh->mbs,NULL,NULL)) < Success)
		goto error;

	*val = get_qnumval(mh);
error:
	if(tbuf != sbuf)
		pj_freez(&tbuf);
	cleanup_qnumreq(mh);
	return(softerr(err,"smu_number") >= Success);
}
