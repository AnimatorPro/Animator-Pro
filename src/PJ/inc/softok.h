/* softok.h - a tokenizer system originally developed for the
 * Softmenu system. */
#ifndef SOFTOK_H
#define SOFTOK_H

	#ifndef DSTRING_H
		#include "dstring.h"
	#endif
	#ifndef PTRMACRO_H
		#include "ptrmacro.h"
	#endif
	#ifndef TOKEN_H
		#include "token.h"
	#endif

#include "xfile.h"

/* Tokenizer data structures */
	typedef struct stok
	/* Tokenizer structure */
		{
		Dstring d;
		SHORT ttype;
		long longval;	/* if ttype == TOK_LONG result is here */
		long tpos;		/* file byte position */
		long tline;		/* file line position */
		} Stok;

#define SWORKLOOK 3
typedef struct
/* an object that keeps a 4 token lookahead for parsing text files */
	{
	Stok *look[SWORKLOOK+1]; /* one extra for spw_rotate_toks() */
	Stok rtoks[SWORKLOOK];
	XFILE *xf; /* same as s->sf */
	Dstring line;
	char *line_pos;
	long lastfpos;
	long fpos;		/* current byte position */
	long fline;		/* current line position */
	} Swork;
void swork_init(Swork *swork, XFILE *xf, long ipos, long iline);
void swork_cleanup(Swork *swork);
void swork_advance(Swork *swork, int count);
void swork_advance_over(Swork *swork, SHORT ttype);
#define swork_tok(swork, ix) (((swork)->look[(ix)]))
#define swork_top(swork) (swork_tok(swork,0))
#define swork_tops(swork) (swork_top(swork)->d.buf)
#define swork_topt(swork) (swork_top(swork)->ttype)

#endif /* SOFTOK_H */
