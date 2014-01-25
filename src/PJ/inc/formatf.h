#ifndef FORMATF_H
#define FORMATF_H

#ifndef _STDARG_H_INCLUDED
	#include <stdarg.h>
#endif

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

typedef struct formatarg {
	char *fmt;         /* this will contain pointer to next char in format 
						* string after any fa_getc() call */
	va_list args; 	   /* arguments to be formatted */

	char (*getchar)(struct formatarg *fa); 	/* current 'getchar' function */
	char (*root)(struct formatarg *fa); 	/* root state to return to */

	UBYTE mflags;      /* see defines below */
	UBYTE parse_stars; /* only used in format mode */
	USHORT pflags;     /* flags internal to do_fmtchar */
	SHORT width;
	SHORT zeropad;
	SHORT precis;
	SHORT strlen;
	char *str;         /* this will contain pointer to '%' that starts this
					    * format desriptor when in format parse mode used for
						* local data in formatting mode */
	char *prefix;
	LONG count;
	double darg;
	Errcode error;    /* error code if there is an error, only valid when 
					   * return char is 0 */
	char strbuf[64];  /* needs to be big enough to hold largest formatted 
					   * double (as string) part */
} Formatarg;

#ifdef FORMATF_INTERNALS

/* internal format state functions */

char geta_fmtchar(struct formatarg *fa);

/* flags for mflags set by formatter read only by caller */

#define FA_PARSE_MODE 0x01 /* if set, fa_getchar() will only return the 
					    	* conversion type bytes ie the "*s" for "abc%2.*s"
					    	* to pre-scan format strings */

#endif  /* FORMATF_INTERNALS */

#define FA_FORMAT_DONE 0x02 /* a format operation was done */
#define FA_ABORT_ON_ERROR 0x04  /* abort format if an invalid format is 
								 * discovered put error code in fa->error */

/*** to be used like va_start() and va_end() for a formatarg ***/
extern void init_formatarg(Formatarg *fa, char *fmt);

#define start_formatarg(fa,fmt) \
 {va_start(fa.args,fmt);init_formatarg(&fa,fmt);}

#define end_formatarg(fa) { va_end(fa.args); }

/** the meat **/
extern char fa_getchar(Formatarg *fa);
#define fa_getc(fa)	(++(fa)->count,((*((fa)->getchar))(fa)))

int fa_lenf(Formatarg *fa);
int fa_sprintf(char *buf,int maxlen, Formatarg *fa);
int local_sprintf(char *buf, char *format, ...);
Errcode vnsprintf(char *buf, int maxlen, char *format, va_list args);

/****************************************************************************/
/* Parsing format type values desired by a format string */

/* use init_format_parse() to initialize the format arg instead of 
 * init_formatarg() or start_formatarg(). (It must be re-initialized for
 * formatting with init or start_formatarg() if the same struct is to be used
 * for formatting */

void init_format_parse(Formatarg *fa, char *fmt);

/* after initialized with init_format_parse() fa_getc(), fa_getchar()
 * and fa_sprintf() will return characters that represent the types desired
 * by the format. The count field will represent the number of types. The string
 * will be NULL terminated as a formatted string.  The data is bit packed and
 * is extracted with the macros below. */

/* data type indicator in lower 3 bits if char 
 * NOTE: integer types, on watcom the LONG and DLONG types won't exist */

#define FT_INT 			0
#define FT_LONG     	1
#define FT_DLONG    	2

/* double type */

#define FT_DOUBLE   	3

/* pointer types */

#define FT_VOID_PTR 	4	
#define FT_CHAR_PTR		5
#define FT_SHORT_PTR    6	
#define FT_INT_PTR	    7

/* this extracts the type id from the fa_getc() output for format parsing */
/* FT_TYPE will extract the type numbers from the returned chars */

#define FT_TYPE(c) ((c)&0x07)

/* FT_FMTCHAR() extracts the format char type ie: "%s" will give 's' 
 *
 * The chars map as follows: "*cdieEfgGnopsuxX"
 *						 to: "jcdieefggnopsuxx"
 *
 * "%s abc %*.2s %*.*G %f %n" will return chars "sjsjjgfn"
 *
 */

#define FT_FMTCHAR(c) ((((UBYTE)c)>>3)|0x60)

/* special case for a '*' type argument */

#define FT_STARTYPE ((UBYTE)(('*'<<3)|FT_INT))


#endif /* FORMATF_H */
