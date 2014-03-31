#include <string.h>
#include "errcodes.h"
#include "memory.h"
#include "softok.h"

static void stok_init(Stok *smt)
/* constructor for a Stok */
{
dstring_init(&smt->d);
smt->ttype = 0;
smt->tpos = 0;
smt->longval = 0;
}

static void stok_cleanup(Stok *smt)
/* Destroy an Stok */
{
dstring_cleanup(&smt->d);
}

void swork_init(Swork *swork, XFILE *xf, long ipos, long iline)
{
int i;

swork->lastfpos = swork->fpos = ipos;
swork->line_pos = NULL;
swork->xf = xf;
swork->fline = iline;
xfseek(xf, ipos, XSEEK_SET);
for (i=0; i<SWORKLOOK; i++)
	{
	swork->look[i] = &swork->rtoks[i];
	stok_init(swork_tok(swork,i));
	}
dstring_init(&swork->line);
swork_advance(swork,SWORKLOOK);
}

void swork_cleanup(Swork *swork)
{
int i;

for (i=0; i<SWORKLOOK; i++)
	stok_cleanup(swork_tok(swork,i));
dstring_cleanup(&swork->line);
}

static Errcode swork_read_token(Swork *swork,Stok *smt)
/* read in next token, and classify it if it's a constant */
{
Errcode err = Success;
char buf[DST_SMALL];
char *pt = buf;		/* pt will point to buf unless token is big */
char *comment;
SHORT slen;

do	{
	if (swork->line_pos == NULL)
		{
		smt->tpos = swork->lastfpos = swork->fpos;
		if ((err = dstring_get_line(&swork->line, swork->xf)) < Success)
			{
			goto OUT;
			}
		swork->fpos += swork->line.blen-1;
		swork->line_pos = swork->line.buf;
		swork->fline += 1;
		if ((comment = strstr(swork->line.buf, "//")) != NULL)
			*comment = 0;
		}
	else
		{
		smt->tpos = swork->lastfpos + swork->line_pos - swork->line.buf;
		}
	if (swork->line.blen > (int)sizeof(buf))
		{
		if ((pt = pj_malloc(swork->line.blen)) == NULL)
			{
			err = Err_no_memory;
			goto OUT;
			}
		}
	swork->line_pos = tokenize_word(swork->line_pos, pt, NULL, &slen,
		&smt->ttype, FALSE);
	} while (swork->line_pos == NULL);
smt->tline = swork->fline;
switch (smt->ttype)
    {
	case TOK_SQUO:
        smt->ttype = TOK_LONG;
        smt->longval = (unsigned char)(pt[0]);
        break;
	case TOK_INT:
        smt->ttype = TOK_LONG;
	case TOK_LONG:
		if (pt[1] == 'X')
           smt->longval = htol(pt+2);
        else
           smt->longval = atol(pt);
		break;
	case 26:		/* control Z */
		smt->ttype = TOK_EOF;
		break;
    }
err = dstring_memcpy(&smt->d,pt,slen+1);
OUT:
if (pt != buf)
	pj_free(pt);
if (err < Success)
	smt->ttype = TOK_EOF;
return(err);
}


void swork_advance(Swork *swork, int count)
{
while (--count >= 0)
	{
	swork->look[SWORKLOOK] = swork->look[0];
	memmove(swork->look, swork->look+1, SWORKLOOK*sizeof(swork->look[0]));
	swork_read_token(swork,swork->look[SWORKLOOK-1]);
	}
}

void swork_advance_over(Swork *swork, SHORT ttype)
/* advance over current token, and also following token if it matches ttype */
{
swork_advance(swork, 1);
if (swork_top(swork)->ttype == ttype)
	swork_advance(swork,1);
}


