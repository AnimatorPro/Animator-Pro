#include "errcodes.h"
#include "memory.h"
#include "ftextf.h"

Errcode get_formatted_ftext(char **pbuf, int max_to_alloc,
					        char *formats, char *text,va_list args, 
							Boolean force_copy)

/* checks size if text is smaller or = to max_to_alloc or *pbuf == NULL it 
 * will put text in *pbuf, if bigger it will attempt allocate of a buffer
 * for *pbuf and put the text in the buffer returns length of text or errcode.
 * If formats is non-NULL it will use the Ftext format processor, if NULL
 * it will use the sprintf() format processor, If no formatting is done it
 * will set *pbuf to NULL and not allocate or copy the text.  If force_copy
 * is true it will always copy the text */
{
Ftextfarg fa;
int tlen;
char *tbuf;

	if(text == NULL)
		return(0);

	copy_va_list(args,fa.fa.args);
	init_eitherfarg(&fa,formats,text);
	tlen = fa_lenf(&fa.fa);
	if(fa.fa.error < Success)
		return(fa.fa.error);

	if(!force_copy && !(fa.fa.mflags & FA_FORMAT_DONE))
	{
		*pbuf = NULL;
		return(tlen);
	}

	if(*pbuf == NULL || tlen >= max_to_alloc)
	{
		if((tbuf = pj_malloc(tlen + 1)) == NULL)
			return(Err_no_memory);
		*pbuf = tbuf;
	}
	else
		tbuf = *pbuf;

	copy_va_list(args,fa.fa.args);
	init_eitherfarg(&fa,formats,text);
	fa_sprintf(tbuf,tlen + 1,&fa.fa);
	if(fa.fa.error < Success)
	{
		pj_freez(pbuf);
		return(fa.fa.error);
	}
	return(tlen);
}
