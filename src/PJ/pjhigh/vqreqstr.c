#include "jimk.h"
#include "commonst.h"
#include "ftextf.h"

Boolean varg_qreq_string(char *strbuf,int bufsize,
						 char *formats,char *text,va_list args)
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

	if((err = build_qstrreq(icb.input_screen,&mh,text,
							okcan,strbuf,bufsize)) < 0)
		goto error;

	mh->cursor = vb.screen->cursor;
	err = do_reqloop(vb.screen,mh,mh->mbs,NULL,NULL);
	cleanup_qstrreq(mh);
error:
	if(tbuf != sbuf)
		pj_freez(&tbuf);
	return(softerr(err,"smu_string")  >= 0);
}
