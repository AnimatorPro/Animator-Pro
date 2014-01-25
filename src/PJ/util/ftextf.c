#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#define FORMATF_INTERNALS
#include "errcodes.h"
#include "ptrmacro.h"
#include "ftextf.h"
#include "cstack.h"

static int load_arginfo(char *format,Ft_arginfo *ainfo, int maxargs)

/* load array of offsets to actual formats and to stack areas containing data 
 * returns errcode if a problem the number of arguments parsed if ok.  It will
 * load one more offset than the number of arguments so that the size for the
 * last argument may be calculated using it and the offset to the last and to
 * the one beyond last.  "cstack.h" is a compiler dependent header that
 * defines the size of objects on the stack.  This function will only work fo
 * va_lists which are pointers to the stack or a duplicate thereof.  No items 
 * will have an offset of 255 */
{
Formatarg fa;
Ft_arginfo *ai, *lastai;
char c;
SHORT offset;

	if(!format)
		return(0);

	ai = ainfo;
	lastai = ai + maxargs;

	init_format_parse(&fa,format);
	offset = ai->va_oset = 0; /* first one starts at zero */

	while(c = fa_getchar(&fa))
	{
		switch(FT_TYPE(c))
		{
			case FT_INT:
				offset += ssizeof(int);
				if(c == FT_STARTYPE)  /* '*' star type int args */
					continue;
				break;
			case FT_LONG:
			case FT_DLONG:
				offset += ssizeof(long);
				break;
			case FT_VOID_PTR:
			case FT_CHAR_PTR:
				offset += ssizeof(void *);
				break;
			case FT_SHORT_PTR:
			case FT_INT_PTR:
				if(FT_FMTCHAR(c) == 'n')
					return(Err_n_args_invalid);
			case FT_DOUBLE:
				offset += ssizeof(double);
				break;
		}
		if(SIZE(format,fa.str) >= 255)
			return(Err_too_many_args);
		ai->fmt_oset = SIZE(format,fa.str); 
		if(++ai >= lastai) 
			return(Err_too_many_args);
		ai->va_oset = offset; /* set offset to next one */
	}
	return(ai - ainfo); /* return number of arg formats parsed */
}

static char geta_ftextfchar(Ftextfarg *fa)
{
char retchar;
char *tend; 
Ft_arginfo *ai;
unsigned long argnum;

get_another_char:

	if((retchar = *(fa->text)++) != '!' || *fa->text != '[')
		return(retchar);

	if((argnum = strtol(fa->text + 1, &tend,10)) <= 0)
		goto error;

	if(argnum > fa->numargs)
		goto error;

	if(*tend != ']')
		goto error;

	fa->text = tend + 1; /* set to next text buffer char */

	ai = &(fa->ai[argnum-1]);

	fa->fa.fmt = (char *)OPTR(fa->formats,ai->fmt_oset);

	/* you can see that if the va_list isn't a pointer to the stack this will
	 * not work */

	*((void **)&(fa->fa.args)) = OPTR(*((void **)&(fa->va_root)),ai->va_oset); 

	/* if we exaust this format we still continue so we go back (to top of 
	 * this function) and check the next char in the higfh level format */

	if((retchar = geta_fmtchar(&fa->fa)) == 0)
		goto get_another_char;

	return(retchar);

error:
	fa->fa.fmt = --fa->text;
	fa->fa.error = Err_bad_argnum;
	return(0);
}
static char ftext_do_first(Ftextfarg *fa)
{
	copy_va_list(fa->fa.args,fa->va_root);
	fa->fa.root = (char (*)(struct formatarg *))geta_ftextfchar;
	return(geta_ftextfchar(fa));
}
Errcode init_eitherfarg(Ftextfarg *fa, char *formats, char *text)

/* returns error code or the number of arguments in formats if formats is
 * NULL it will initialize as a formatarg! and will parse text as if it had
 * printf style format info in it */
{
Errcode err;

	init_formatarg(&(fa->fa),text);
	if(formats == NULL)
		return(Success);
	if((err = load_arginfo(formats,fa->ai,Array_els(fa->ai))) < Success)
		return(err);
	fa->numargs = err;
	fa->formats = formats;
	fa->text = text;
	fa->fa.getchar = (char (*)(struct formatarg *))ftext_do_first;
	fa->fa.mflags |= FA_ABORT_ON_ERROR; 
	return(err);
}
