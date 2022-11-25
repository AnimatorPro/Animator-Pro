/*****************************************************************************
 * pocolib.c - Common routines for other poco library functions.
 *
 * 05/15/92 (Ian)
 *			Tweaked po_check_formatf() so that a NULL pointer passed for
 *			a %p arg doesn't cause an abort -- it's reasonable to print
 *			the value of a NULL or wild pointer, and printing it won't
 *			cause it to be dereferenced.
 ****************************************************************************/


#include "poco.h"
#include "errcodes.h"
#include "ptrmacro.h"
#include "pocolib.h"
#include "formatf.h"

extern Errcode builtin_err;

Errcode po_init_libs(Poco_lib *lib)
/*****************************************************************************
 *
 ****************************************************************************/
{
	Errcode err;

	while (lib != NULL)
		{
		init_list(&lib->resources);
		if (lib->init)
			if (Success > (err = lib->init(lib)))
				return err;
		lib = lib->next;
		}
	return(Success);
}

void po_cleanup_libs(Poco_lib *lib)
/*****************************************************************************
 *
 ****************************************************************************/
{
	while (lib != NULL)
		{
		if (lib->cleanup)
			lib->cleanup(lib);
		lib = lib->next;
		}
}

void poco_freez(Popot *pt)
/*****************************************************************************
 * free buffer allocated into Poco space and set it to NULL
 ****************************************************************************/
{
if (pt->pt != NULL)
	po_free(*pt);
pt->pt = pt->min =	pt->max = NULL;
}

Errcode po_check_formatf(int maxlen, int vargcount, int vargsize,
						 char *fmt, va_list pargs)
/*****************************************************************************
 * sanity-check the printf-like format & args passed in from a poco program.
 ****************************************************************************/
{

#if defined(__BORLANDC__) || defined(__TURBOC__)

	return Success; /* for bc/tc, we have to pretend all is well */

#else

	va_list 	args;
	Formatarg	fa;
	UBYTE		curfmt;
	void		*chkptr;
	int 		expected_size  = 0;
	int 		expected_count = 0;

	if (fmt == NULL)
		return builtin_err = Err_null_ref;

	init_format_parse(&fa, fmt);
	copy_va_list(pargs, args);

	for (;;)
		{
		if (0 == (curfmt = fa_getc(&fa)))
			break;
		++expected_count;
		switch (FT_TYPE(curfmt))
			{
			default:			/* shouldn't happen? */
			case FT_INT:
			case FT_LONG:
			case FT_DLONG:
				expected_size += sizeof(long);
				(void)va_arg(args, int);
				break;
			case FT_DOUBLE:
				expected_size += sizeof(double);
				(void)va_arg(args, double);
				break;
			case FT_VOID_PTR:
			case FT_CHAR_PTR:
			case FT_SHORT_PTR:
			case FT_INT_PTR:
				expected_size += sizeof(void *);
				if (FT_FMTCHAR(curfmt) == 'p') {
					(void)va_arg(args, void *);
				} else {
					chkptr = va_arg(args, void *);
					if (chkptr == NULL)
						return builtin_err = Err_null_ref;
					if ((long)chkptr < 4096L)
						return builtin_err = Err_parameter_range;
				}
				break;
			}
		}

	if (vargcount < expected_count)
		return builtin_err = Err_too_few_params;
	if (vargcount > expected_count)
		return builtin_err = Err_too_many_params;
	if (vargsize != expected_size)
		return builtin_err = Err_parameter_range;

	if (maxlen > 0) /* caller wants a buffer length check, too... */
		{
		copy_va_list(pargs, fa.args);
		init_formatarg(&fa, fmt);
		if (maxlen < fa_lenf(&fa))
			return builtin_err = Err_string;
		}

	return Success;

#endif /* not borland or turbo c */

}
