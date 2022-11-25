/* serrtext.c - search resource file for text string describing
 * Errcode. */

#include <string.h>
#include "errcodes.h"
#include "softmenu.h"

#ifdef SLUFFED
Errcode smu_get_signed(Swork *swork, long *signedp)
/* return a signed number in signedp */
{
Stok *t;

t = swork_top(swork);
if (t->ttype == TOK_LONG)
	{
	*signedp = t->longval;
	swork_advance_over(swork,',');
	return(Success);
	}
else if (t->ttype == '-')
	{
	swork_advance(swork,1);
	t = swork_top(swork);
	if (t->ttype == TOK_LONG)
		{
		*signedp = -t->longval;
		swork_advance_over(swork,',');
		return(Success);
		}
	}
return(Err_expecting_number);
}
#endif /* SLUFFED */

static int smu_str_for_num(Swork *swork, int num, char *buf,
	int bufsize)
{
Stok *t;
int got_num;

for (;;)
	{
	if (swork_topt(swork) == TOK_RBRACE)
		return(Err_not_found);
	t = swork_top(swork);
	if (t->ttype != TOK_LONG)
		return(Err_expecting_number);
	got_num = t->longval;
	swork_advance_over(swork,',');
	t = swork_top(swork);
	if (t->ttype != TOK_QUO)
		return(Err_expecting_string);
	if (got_num == num)
		{
		if (t->d.blen > bufsize)
			return(Err_too_big);
		strcpy(buf,t->d.buf);
		return(t->d.blen-1);
		}
	swork_advance_over(swork,',');
	}
}

int smu_get_errtext(struct softmenu *sm,
	char *symname,		/* name of symbol in resource file */
	Errcode err_to_find, /* errcode to find */
	char *buf)			/* place to put string */
/* Scan for err in NumString symname.  Fill in buf with matching text
 * if found, and return length of text.  If a problem return errcode. */
{
Smu_symbol *sym;
Swork rswork;
Errcode err;

buf[0] = 0;		/* in case we don't find anything... */
if ((err = smu_lookup(sm,&sym,SMU_NUMSTRING_CLASS,symname)) < Success)
	return(err);
swork_init(&rswork, sm->xf, sym->foff, sym->fline);
err = smu_str_for_num(&rswork, err_to_find, buf, ERRTEXT_SIZE);
swork_end(&rswork,sm);
return(err);
}
