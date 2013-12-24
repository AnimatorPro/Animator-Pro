#include "pocolib.h"
#include <stdarg.h>
#include <string.h>
#include "poco.h"
#include "ptrmacro.h"
#include "errcodes.h"

extern char *getenv(); /* can't use watcom stdlib.h, conflicts with pj hdrs */

extern Errcode builtin_err;
extern char *clone_string();

extern Errcode vnsprintf(char *buf, int maxlen, char *format, va_list args);

static Popot null_popot = {NULL, NULL, NULL};

static int po_sprintf(long vargcount, long vargsize, Popot buf, Popot format, ...)
/*****************************************************************************
 * int sprintf(char *buf, char *format, ...)
 ****************************************************************************/
{
	int 	rv;
	va_list args;

	if (buf.pt == NULL)
		return(builtin_err = Err_null_ref);

	va_start(args, format);

	if (Success > (rv = po_check_formatf(Popot_bufsize(&buf), vargcount, vargsize,
				format.pt, args)))
		return rv;

	  rv = vsprintf(buf.pt, format.pt, args);

	va_end(args);
	return rv;
}

static int po_strcmp(Popot d, Popot s)
/*****************************************************************************
 * int strcmp(char *a, char *b)
 ****************************************************************************/
{
	if (d.pt == NULL || s.pt == NULL)
		return(builtin_err = Err_null_ref);
	return(strcmp(d.pt, s.pt));
}

static int po_stricmp(Popot d, Popot s)
/*****************************************************************************
 * int stricmp(char *a, char *b)
 ****************************************************************************/
{
	if (d.pt == NULL || s.pt == NULL)
		return(builtin_err = Err_null_ref);
	return(stricmp(d.pt,s.pt));
}

static int po_strncmp(Popot d, Popot s, int maxlen)
/*****************************************************************************
 * int strncmp(char *a, char *b, int maxlen)
 ****************************************************************************/
{
	if (d.pt == NULL || s.pt == NULL)
		return(builtin_err = Err_null_ref);
	return(strncmp(d.pt, s.pt,maxlen));
}

static int po_strlen(Popot s)
/*****************************************************************************
 * int strlen(char *a)
 ****************************************************************************/
{
	if (s.pt == NULL)
		return(builtin_err = Err_null_ref);
	return(strlen(s.pt));
}

static Popot po_strcpy(Popot d, Popot s)
/*****************************************************************************
 * char *strcpy(char *dest, char *source)
 ****************************************************************************/
{

	if (d.pt == NULL || s.pt == NULL)
		builtin_err = Err_null_ref;
	else if (Popot_bufsize(&d) < (1+strlen(s.pt)))
		builtin_err = Err_string;
	else
		strcpy(d.pt,s.pt);

	return(d);
}

static Popot po_strncpy(Popot d, Popot s, int maxlen)
/*****************************************************************************
 * char *strncpy(char *dest, char *source, int maxlen)
 ****************************************************************************/
{
	int dlen;

	if (d.pt == NULL || s.pt == NULL)
		builtin_err = Err_null_ref;
	else
		{
		dlen = Popot_bufsize(&d);
		if (maxlen >= dlen)
			maxlen = dlen-1;
		strncpy(d.pt,s.pt,maxlen);
		}
	return(d);
}

static Popot po_strcat(Popot d, Popot tail)
/*****************************************************************************
 * char *strcat(char *dest, char *source)
 ****************************************************************************/
{
	if (d.pt == NULL || tail.pt == NULL)
		builtin_err = Err_null_ref;
	else if (Popot_bufsize(&d) < (1+strlen(d.pt)+strlen(tail.pt)))
		builtin_err = Err_null_ref;
	else
		strcat(d.pt,tail.pt);

	return(d);
}

static Popot po_strdup(Popot s)
/*****************************************************************************
 * char *strdup(char *source)
 ****************************************************************************/
{
	int len;
	Popot d = {NULL,NULL,NULL};

	if (s.pt == NULL)
		{
		builtin_err = Err_null_ref;
		}
	else
		{
		len = strlen(s.pt)+1;
		d = po_malloc(len);
		strcpy(d.pt,s.pt);
		}
	return(d);
}

static Popot po_strchr(Popot s1, int c)
/*****************************************************************************
 * char *strchr(char *source, int c)
 ****************************************************************************/
{
	if (s1.pt == NULL)
		{
		builtin_err = Err_null_ref;
		return null_popot;
		}
	if (NULL == (s1.pt = strchr(s1.pt,c)))
		return null_popot;

	return(s1);
}

static Popot po_strrchr(Popot s1, int c)
/*****************************************************************************
 * char *strrchr(char *source, int c)
 ****************************************************************************/
{
	if (s1.pt == NULL)
		{
		builtin_err = Err_null_ref;
		return null_popot;
		}
	if ((s1.pt = strrchr(s1.pt,c)) == NULL)
		return null_popot;

	return(s1);
}

static Popot po_strstr(Popot s1, Popot s2)
/*****************************************************************************
 * char *strstr(char *string, char *substring)
 ****************************************************************************/
{

	if (s1.pt == NULL || s2.pt == NULL)
		{
		builtin_err = Err_null_ref;
		return null_popot;
		}
	if ((s1.pt = strstr(s1.pt,s2.pt)) == NULL)
		return null_popot;

	return(s1);
}


static Popot po_stristr(Popot s1, Popot s2)
/*****************************************************************************
 * char *stristr(char *string, char *substring)
 ****************************************************************************/
{
	char *p1 = NULL, *p2 = NULL;
	char *res;

	if (s1.pt == NULL || s2.pt == NULL)
		{
		builtin_err = Err_null_ref;
		goto ERR;
		}
	if ((p1 = clone_string(s1.pt)) == NULL)
		{
		builtin_err = Err_no_memory;
		goto ERR;
		}
	if ((p2 = clone_string(s2.pt)) == NULL)
		{
		builtin_err = Err_no_memory;
		goto ERR;
		}
	upc(p1);
	upc(p2);
	if ((res = strstr(p1,p2)) == NULL)
		goto ERR;
	s1.pt = OPTR(s1.pt,res-p1);
	goto OUT;
ERR:
	s1.pt = s1.min = s1.max = NULL;
OUT:
	pj_gentle_free(p1);
	pj_gentle_free(p2);
	return(s1);
}

static int po_atoi(Popot str)
/*****************************************************************************
 * int atoi(char *str);
 ****************************************************************************/
{
	extern int atoi(char *s);

	if (str.pt == NULL)
		return builtin_err = Err_null_ref;

	return atoi(str.pt);
}

static double po_atof(Popot str)
/*****************************************************************************
 * double atof(char *str);
 ****************************************************************************/
{
	extern double atof(char *s);

	if (str.pt == NULL)
		return builtin_err = Err_null_ref;

	return atof(str.pt);
}

static int po_strspn(Popot s1, Popot s2)
/*****************************************************************************
 * int strspn(char *string, char *charset)
 ****************************************************************************/
{

	if (s1.pt == NULL || s2.pt == NULL)
		{
		return builtin_err = Err_null_ref;
		}
	return strspn(s1.pt,s2.pt);
}

static int po_strcspn(Popot s1, Popot s2)
/*****************************************************************************
 * int strcspn(char *string, char *charset)
 ****************************************************************************/
{

	if (s1.pt == NULL || s2.pt == NULL)
		{
		return builtin_err = Err_null_ref;
		}
	return strcspn(s1.pt,s2.pt);
}

static Popot po_strpbrk(Popot s1, Popot s2)
/*****************************************************************************
 * char *strpbrk(char *string, char *breakset)
 ****************************************************************************/
{

	if (s1.pt == NULL || s2.pt == NULL)
		{
		builtin_err = Err_null_ref;
		return null_popot;
		}
	if ((s1.pt = strpbrk(s1.pt,s2.pt)) == NULL)
		return null_popot;

	return(s1);
}

static Popot po_strtok(Popot s1, Popot s2)
/*****************************************************************************
 * char *strtok(char *string, char *delimset)
 ****************************************************************************/
{

	if (s2.pt == NULL) /* note that NULL s1 is allowed! */
		{
		builtin_err = Err_null_ref;
		return null_popot;
		}
	if ((s1.pt = strtok(s1.pt,s2.pt)) == NULL)
		return null_popot;

	return(s1);
}

static Popot po_getenv(Popot s1)
/*****************************************************************************
 * char *getenv(char *varname)
 ****************************************************************************/
{

	if (s1.pt == NULL)
		{
		builtin_err = Err_null_ref;
		return null_popot;
		}
	if ((s1.pt = getenv(s1.pt)) == NULL)
		return null_popot;

	return(s1);
}

static Popot po_strlwr(Popot d)
/*****************************************************************************
 * char *strlwr(char *string)
 ****************************************************************************/
{

	if (d.pt == NULL)
		builtin_err = Err_null_ref;
	else
		strlwr(d.pt);
	return(d);
}

static Popot po_strupr(Popot d)
/*****************************************************************************
 * char *strupr(char *string)
 ****************************************************************************/
{

	if (d.pt == NULL)
		builtin_err = Err_null_ref;
	else
		strupr(d.pt);
	return(d);
}

static Popot po_strerror(int err)
/*****************************************************************************
 * char *strerror(int errnum)
 ****************************************************************************/
{
	static char  errmsg[ERRTEXT_SIZE];
	static Popot retval = {errmsg, errmsg, errmsg+ERRTEXT_SIZE};

	get_errtext(err, errmsg);
	return retval;
}

/*----------------------------------------------------------------------------
 * library protos...
 *
 * Maintenance notes:
 *	The stringent rules that apply to maintaining most builtin lib protos
 *	don't apply here.  These functions are not visible to poe modules;
 *	you can add or delete protos anywhere in the list below.
 *--------------------------------------------------------------------------*/

static Lib_proto lib[] = {
/* string stuff */
{po_sprintf,
	"int     sprintf(char *buf, char *format, ...);"},
{po_strcmp,
	"int     strcmp(char *a, char *b);"},
{po_stricmp,
	"int     stricmp(char *a, char *b);"},
{po_strncmp,
	"int     strncmp(char *a, char *b, int maxlen);"},
{po_strlen,
	"int     strlen(char *a);"},
{po_strcpy,
	"char    *strcpy(char *dest, char *source);"},
{po_strncpy,
	"char    *strncpy(char *dest, char *source, int maxlen);"},
{po_strcat,
	"char    *strcat(char *dest, char *source);"},
{po_strdup,
	"char    *strdup(char *source);"},
{po_strchr,
	"char    *strchr(char *source, int c);"},
{po_strrchr,
	"char    *strrchr(char *source, int c);"},
{po_strstr,
	"char    *strstr(char *string, char *substring);"},
{po_stristr,
	"char    *stristr(char *string, char *substring);"},
{po_atoi,
	"int     atoi(char *string);"},
{po_atof,
	"double  atof(char *string);"},
{po_strpbrk,
	"char    *strpbrk(char *string, char *breakset);"},
{po_strspn,
	"int     strspn(char *string, char *breakset);"},
{po_strcspn,
	"int     strcspn(char *string, char *breakset);"},
{po_strtok,
	"char    *strtok(char *string, char *delimset);"},
{po_getenv,
	"char    *getenv(char *varname);"},
{po_strlwr,
	"char    *strlwr(char *string);"},
{po_strupr,
	"char    *strupr(char *string);"},
{po_strerror,
	"char    *strerror(int errnum);"},
};

Poco_lib po_str_lib =
	{
	NULL,
	"(C standard) String",
	lib,
	Array_els(lib),
	};

