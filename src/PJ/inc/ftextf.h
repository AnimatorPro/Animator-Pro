#ifndef FTEXTF_H
#define FTEXTF_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef FORMATF_H
	#include "formatf.h"
#endif

/************ The Ftext formatted text format type and calls *************/
/* Ftext formatter uses a format that works in tandem with a "printf" style
 * list of formats.  The formats argument to the init_ftextarg() call is 
 * a list of '%' format descriptors.
 * ie: "%d%*.*s%f%02X". This describes 3 argument formats "%d","%*.*s", and
 * "%02X",  These formats and the arguments they represent are substituted
 * into a string with argument descriptors in the format "![number]" as in
 * "there are ![1] files on the disk".  "![1]" refers to the first argument
 * ![2] the second and so on. For now only 32 arguments are allowed. argument
 * substitutions can be used more than once. as in "![1] is the same as ![1]"
 * All other characters in the string other than the ![??]s are passed though
 * un altered 
 *
 * The fa.args va_list must be set to the va_list pointing to the arguments
 * for the formats list as part of the initialization. Just like a Formatarg.
 *
 * An initialized Ftextfarg * can be passed into any routine that uses a 
 * Formatarg * and will perform the format operation using the Ftext format
 * upon reading an invalid format "![a0]" for example the format wil abort
 * and an Errcode for the error can be found in fa->fa.error 
 *
 */

#define FTEXTF_MAXARGS	32

typedef struct ft_arginfo {
	UBYTE va_oset;		/* va list offset */
	UBYTE fmt_oset;     /* format string offset */   
} Ft_arginfo;

typedef struct ftextfarg {
	Formatarg fa;
	char *formats;
	char *text;
	va_list va_root; /* internally set to root va_list or pointer to arg[0] */
	Ft_arginfo ai[FTEXTF_MAXARGS]; /* argument info array */
	UBYTE numargs; /* number of arguments found in formats */
} Ftextfarg;


/* this is used to implement an escpae "!%" which will iterpret a string
 * with a leading "!%" as a formats string and a printf() style string
 * if not */

char *ftext_format_type(char **pfmt,va_list *pargs);

/* this is the same as init_ftextfarg() but, if formats is NULL it will 
 * initialize it the same way as a Formatarg for printf style formatting */

Errcode init_eitherfarg(Ftextfarg *fa, char *formats, char *text);

Errcode get_formatted_ftext(char **pbuf, int max_to_alloc,
					        char *formats, char *text, va_list args, 
							Boolean force_copy);

int snftextf(char *buf, int maxlen, char *fmt,...);
int sftextf(char *buf, char *fmt,...);


#endif /* FTEXTF_H */
