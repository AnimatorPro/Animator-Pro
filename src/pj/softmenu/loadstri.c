/* loadstri.c - Parse simply (optional) comma separated lists of strings.
 * Parses class Strings and class Qchoice SoftMenu objects */

#include <string.h>
#include "errcodes.h"
#include "linklist.h"
#include "memory.h"
#include "softmenu.h"
#include "util.h"

static int text_to_dstring(Swork *swork, Dstring *ds, Boolean is_text)
/* Read in a quoted string from file into Dstring.   If is_text parameter
 * is TRUE, then all quoted string will be concatenated into one string.
 * if FALSE then the strings are concatenated with 0 chars between them. 
 * and commas between the strings are ignored  */
{
int count = 0;
Stok *t;
#define SS (&(t->d))
Errcode err;

	for (;;)
	{
		t = swork_top(swork);
		if (t->ttype == TOK_RBRACE)
			break;
		if (is_text && t->ttype == ',')
			break;
		if (t->ttype != TOK_QUO)
		{
			err = Err_expecting_string;
			goto OUT;
		}
		if (is_text)
		{
			if ((err = dstring_strcat(ds, SS)) < Success)
				goto OUT;
			swork_advance(swork,1);
		}
		else
		{
			if ((err = dstring_memcat(ds, SS)) < Success)
				goto OUT;
			swork_advance_over(swork,',');
		}
		count+=1;
	}

	err = count;
OUT:
	return(err);
#undef SS
}

#ifdef SLUFFED 
#endif /* SLUFFED */

static Errcode smu_parse_strings(Swork *swork, char ***pstrings)
{
Errcode err = Success;
Dstring ds;
char *dpt;
char **strings = NULL;
char **spts;
char *sbuf;
int count = 0;
int ptlen;
int slen;
int i;

dstring_init(&ds);
if ((err = count = text_to_dstring(swork, &ds, FALSE)) < Success)
	goto OUT;
ptlen = (count+1)*sizeof(char *);
if ((sbuf = pj_malloc(ds.blen + ptlen)) == NULL)
	{
	err = Err_no_memory;
	goto OUT;
	}
spts = strings = (char **)sbuf;
sbuf += ptlen;
i = count;
dpt = ds.buf;
memcpy(sbuf,dpt,ds.blen);

while (--i >= 0)
	{
	slen = strlen(dpt)+1;
	*spts++ = sbuf;
	sbuf += slen;
	dpt += slen;
	}
*spts = NULL;
err = count;
OUT:
	{
	*pstrings = strings;
	dstring_cleanup(&ds);
	return(err);
	}
}

static Errcode smu_get_ss(struct softmenu *sm, char *symname, Smu_strings *s, 
	unsigned class)
/* Get an array of strings. */
{
Errcode err;
Smu_symbol *sym;
Swork rswork;

clear_struct(s);
if ((err = smu_lookup(sm,&sym,class,symname)) < Success)
	goto OUT;
swork_init(&rswork, sm->xf, sym->foff, sym->fline);
err = s->count = smu_parse_strings(&rswork, &s->strings);
swork_end(&rswork,sm);
OUT:
return(err);
}

Errcode smu_get_strings(struct softmenu *sm, char *symname, Smu_strings *s)
{
return(smu_get_ss(sm,symname,s,SMU_STRINGS_CLASS));
}

void smu_free_strings(Smu_strings *s)
{
pj_freez(&s->strings);
}

Errcode smu_string(struct softmenu *sm, char *symname, char *buf, int len)
/* Read in text to a fixed sized buffer.  Returns length of string read in,
 * or negative error code. */
{
Errcode err;
Dstring ds;
Swork rswork;
Smu_symbol *sym;


	if ((err = smu_lookup(sm,&sym,SMU_TEXT_CLASS,symname)) < Success)
		{
		buf[0] = 0;		/* prevent garbage on error anyways... */
		return(err);
		}
	dstring_init(&ds);
	swork_init(&rswork, sm->xf, sym->foff, sym->fline);
	if ((err = text_to_dstring(&rswork, &ds, TRUE)) >= Success)
		{
		if (ds.blen > len)		/* Too big for buffer, copy most of it
								 * in, but return error. */
			{
			err = Err_too_big;
			memcpy(buf, ds.buf, len);
			buf[len-1] = 0;
			}
		else					/* Fits in buffer.  Strcpy it and
								 * set up to return # of characters. */
			{
			strcpy(buf, ds.buf);
			err = ds.blen;
			}
		}
	dstring_cleanup(&ds);
	swork_end(&rswork,sm);
	return(err);
}

Errcode smu_load_text(struct softmenu *sm,
							   	   char *symname,
							   	   char **ptext)

/* Load text.  Returns count of string read in. *ptext will be set to NULL
 * if error */
{
Errcode err;
Dstring ds;
Swork rswork;
Smu_symbol *sym;

	*ptext = NULL;
	if ((err = smu_lookup(sm,&sym,SMU_TEXT_CLASS,symname)) < Success)
		return(err);
	dstring_init(&ds);
	swork_init(&rswork, sm->xf, sym->foff, sym->fline);
	if ((err = text_to_dstring(&rswork, &ds, TRUE)) < Success)
		goto error;
	err = dstring_get_clone(&ds,ptext);
error:
	dstring_cleanup(&ds);
	swork_end(&rswork,sm);
	return(err);
}

void smu_free_text(char **ptext)
/* Free text string */
{
	pj_freez(ptext);
}
Errcode smu_load_qchoice_text(Softmenu *sm, char *key, char **ptext)

/* gets text for a qchoice menu. It must not have any newlines in the text.
 * It must have a count reasonable for a qchoice.  Newlines will be put at
 * the end of every choice in a contiguous buffer. It will accept optional
 * commas between choice lines. Newlines are not allowed in the strings 
 * input count must match count in program. If successful it will return
 * the number of lines in the text */
{
Errcode err = Success;
Dstring ds;
char *buf;
char *maxbuf;
Swork rswork;
Smu_symbol *sym;

	*ptext = NULL;
	if ((err = smu_lookup(sm,&sym,SMU_QCHOICE_CLASS,key)) < Success)
		return(err);
	dstring_init(&ds);
	swork_init(&rswork, sm->xf, sym->foff, sym->fline);
	if ((err = text_to_dstring(&rswork, &ds, FALSE)) < Success)
		goto error;

	if(err <= 0)
	{
		err = Err_not_enough_fields;
		goto error;
	}
	if(err > 11) /* line 0 is the header */
	{
		err = Err_too_many_fields;
		goto error;
	}

	buf = ds.buf;
	maxbuf = buf + ds.blen - 1; 
	while(buf < maxbuf)
	{
		switch(*buf++)
		{
			case 0:
				buf[-1] = '\n';
				break;
			case '\n':
				err = Err_invalid_char;
				goto error;
		}
	}
	err = dstring_get_clone(&ds,ptext);
error:
	dstring_cleanup(&ds);
	swork_end(&rswork,sm);
	return(err);
}
