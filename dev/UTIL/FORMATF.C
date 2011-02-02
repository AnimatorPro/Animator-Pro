#define FORMATF_INTERNALS
#include <ctype.h>
#include <stdarg.h>
#include <math.h>
#include "errcodes.h"
#include "formatf.h"


/* #define TESTMAIN */

#define FSPEC_PLUS	0x0001 /* '+' flag was specified */
#define FSPEC_MINUS	0x0002 /* '-' flag was specified */
#define FSPEC_SPACE	0x0004 /* ' ' flag was specified */
#define FSPEC_PSIGN	0x0008 /* '#' flag was specified */
#define FSPEC_ZERO	 0x0010 /* '0' flag was specified */
#define FSPEC_FDONE	 0x0020 /* no more flags accepted */
#define FSPEC_GOTWID  0x0040 /* got a valid field width */
#define FSPEC_0WID	  0x0080 /* width spec'd is 0 */
#define FSPEC_GOTDOT  0x0100 /* found precision delimiter dot */
#define FSPEC_GOTPCIS 0x0200 /* got valid precision */
#define FSPEC_0PCIS   0x0400 /* precision spec'd is 0 */
#define FSPEC_WPDONE  0x0800 /* no more 999.999 accepted */
#define FSPEC_SHORT	  0x1000 /* arg is treated as a short */
#define FSPEC_LONG	  0x2000 /* arg is treated as a long */
#define FSPEC_DLONG	  0x4000 /* arg is treated as a double long */

static int get_fmtint(Formatarg *fa)

/* gets an int from the format string */
{
char *digit;
char digend;
int ret;

	digit = fa->fmt;
	while(isdigit(*digit))
		++digit;
	digend = *digit;
	*digit = 0;
	ret = atoi(fa->fmt);
	*digit = digend;
	fa->fmt = digit;
	return(ret);
}
static char trail_spaces(Formatarg *fa)
{
	if(!(--fa->width))
		fa->getchar = fa->root;
	return(' ');
}
static char do_char(Formatarg *fa)
{
	if(fa->width > 1)
	{
		--fa->width;
		if(fa->pflags & FSPEC_MINUS) /* char first then spaces */
			fa->getchar = trail_spaces;
		else if(fa->width)
			return(' ');
	}
	else
		fa->getchar = fa->root;

	return((char)va_arg(fa->args,int));
}
static char ljust_limit_str(Formatarg *fa)

/* limits string to precision length */
{
	--fa->width;
	--fa->precis;
	if(!(fa->str[1]) || fa->precis == 0)
	{
		if(fa->width > 0)
			fa->getchar = trail_spaces; 
		else
			fa->getchar = fa->root;
	}
	return(*(fa->str)++);
}
static char get_len_str(Formatarg *fa)

/* gets string until length is exhausted must start with at least one char */
{
	if((--fa->strlen) <= 0)
		fa->getchar = fa->root;
	return(*(fa->str)++);
}
static char rjust_get_str(Formatarg *fa)

/* assumes precision is length of string and width is leading spaces */
{
	if(fa->width-- > 0)
		return(' ');
	fa->getchar = get_len_str;
	return(get_len_str(fa));
}
static char do_string(Formatarg *fa)

/* will not be called if *(fa->str) == 0 */
{
	if(!(fa->pflags & FSPEC_GOTPCIS))
		fa->precis = 0x7FFF;	/* if not spec'd very big */

	if(fa->pflags & FSPEC_MINUS)
	{
		fa->getchar = ljust_limit_str;
		return(ljust_limit_str(fa));
	}
	else /* right justify */
	{
		fa->strlen = strlen(fa->str);
		if(fa->strlen > fa->precis)
			fa->strlen = fa->precis;
		fa->width -= fa->strlen;
		fa->getchar = rjust_get_str;
		return(rjust_get_str(fa));
	}
}
static char get_str(Formatarg *fa)
/* returns chars in fa->str fa->str must have a char first time called */
{
	--fa->width;
	if(fa->str[1] == 0)
	{
		if(fa->width > 0)
			fa->getchar = trail_spaces;
		else
			fa->getchar = fa->root;
	}
	return(*(fa->str)++);
}
static char get_zero_pad(Formatarg *fa)
/* only called first time if fa->zeropad > 0 */
{
	if(--fa->zeropad <= 0)
		fa->getchar = get_str;
	--fa->width;
	return('0');
}
static char pfix_str(Formatarg *fa)
{
	if(!(*(fa->prefix)))
	{
		if(fa->zeropad)
		{
			fa->getchar = get_zero_pad;
			return(get_zero_pad(fa));
		}
		fa->getchar = get_str;
		return(get_str(fa));
	}
	--fa->width;
	return(*(fa->prefix)++);
}
static char rjust_pfix_str(Formatarg *fa)
{
	if(fa->width-- > 0)
		return(' ');
	fa->getchar = pfix_str;
	return(pfix_str(fa));
}
static int upper_str(char *sstr)

/* returns length of string too */
{
register char *str = sstr;
char c;

	while((c = *str) != 0)
	{
		if(c >= 'a' && c <= 'z')
			*str -= 'a' - 'A';
		++str;
	}
	return(str - sstr);
}
static char finish_num_string(Formatarg *fa)
{
	if(fa->pflags & FSPEC_MINUS) /* left justified */
	{
		fa->getchar = pfix_str;
		return(pfix_str(fa));
	}

	/* right justified */

	fa->width -= fa->strlen; /* now width left after strings and zero pad */

	if(fa->pflags & FSPEC_ZERO) /* whole field lead with zeros */
	{
		if(fa->width > 0)
			fa->zeropad += fa->width;
		fa->width = 0; /* no space pad */
	}
	fa->getchar = rjust_pfix_str;
	return(rjust_pfix_str(fa));
}
static char do_int(Formatarg *fa,int base)

/* if base < 0 treat as unsigned */
{
long val;

	fa->str = fa->prefix = fa->strbuf;

	/* get arg of right size */

	if(base < 0) /* unsigned */
	{

#if (!INT_IS_SHORT)
		if(fa->pflags & FSPEC_SHORT)
		{
			val = va_arg(fa->args,unsigned short);
			goto got_unsigned;
		}
#endif

#if (!INT_IS_LONG)
		if(fa->pflags & (FSPEC_LONG|FSPEC_DLONG))
		{
			val = va_arg(fa->args,unsigned long);
			goto got_unsigned;
		}
#endif
		val = va_arg(fa->args,unsigned int);

got_unsigned:
		base = -base;
	}
	else /* signed */
	{

#if (!INT_IS_SHORT)
		if(fa->pflags & FSPEC_SHORT)
		{
			val = va_arg(fa->args,short);
			goto got_signed;
		}
#endif

#if (!INT_IS_LONG)
		if(fa->pflags & (FSPEC_LONG|FSPEC_DLONG))
		{
			val = va_arg(fa->args,long);
			goto got_signed;
		}
#endif
		val = va_arg(fa->args,int);

got_signed:

		if(val < 0)
		{
			*fa->str++ = '-';
			val = -val;
			goto sign_done;
		}
	}

	if(fa->pflags & FSPEC_PLUS)
		*fa->str++ = '+';
	else if(fa->pflags & FSPEC_SPACE)
		*fa->str++ = ' ';

sign_done:

	if(fa->pflags & FSPEC_PSIGN && val != 0)
	{
		switch(base)
		{
			case 8:
				*fa->str++ = '0';
				--fa->precis;   /* a zero used */
				break;
			case 16:
				*fa->str++ = '0';
				if(*fa->fmt == 'X')
					*fa->str++ = 'X';
				else
					*fa->str++ = 'x';
				fa->precis -= 2;  /* 2 "zeros" used */
			default:
				break;
		}
	}
	*fa->str++ = 0;  /* terminate prefix */

	ultoa(val,fa->str,base); /* Val positive at this point. Get digits */

	if(*fa->fmt == 'X') /* fudge but works */
		fa->strlen = upper_str(fa->str);
	else
		fa->strlen = strlen(fa->str); /* length of digits */

	fa->zeropad = 0; /* start with no zeros */

	if(fa->pflags & FSPEC_GOTPCIS) /* got a precision, minimum digits */
	{
		if(fa->strlen < fa->precis)
		{
			fa->zeropad = fa->precis - fa->strlen;
			fa->strlen = fa->precis;
		}
	}
	fa->strlen += strlen(fa->prefix);
	return(finish_num_string(fa));
}
static void start_double(Formatarg *fa)
{
	fa->str = fa->prefix = fa->strbuf;
	if(fa->darg < 0.0)
	{
		*fa->str++ = '-';
		fa->darg = -fa->darg;
	}
	else
	{
		if(fa->pflags & FSPEC_PLUS)
			*fa->str++ = '+';
		else if(fa->pflags & FSPEC_SPACE)
			*fa->str++ = ' ';
	}

	fa->strlen = fa->str - fa->strbuf;
	*fa->str++ = 0; /* terminate prefix */
}
static double pcismult[] = {
	1.0,
	10.0,
	100.0,
	1000.0,
	10000.0,
	100000.0,
	1000000.0,
	10000000.0,
	100000000.0,
	1000000000.0,
};
static void pcis_roundit(Formatarg *fa)
{
	if(!(fa->pflags & (FSPEC_GOTPCIS|FSPEC_0PCIS)))
		fa->precis = 6;
	else if(fa->precis > 8)
		fa->precis = 8;

	/* since converting to a long will round down (truncate)
	 * adding the .(sigdig)5 will round on both negative and positive
	 * side */

	fa->darg += 0.5/pcismult[fa->precis];
}
static char *add_dubldigits(Formatarg *fa,Boolean gmode)
{
double frac;
double ival;
ULONG lval;
int sigdig;
int len;
char *suffix;

	suffix = fa->str;
	fa->zeropad = 0;

	if((sigdig = fa->precis) == 0)
	{
		ultoa((ULONG)(fa->darg),suffix,10);
		len = strlen(suffix);
		fa->strlen += len;
		return(suffix + len);
	}
	else
	{
		frac = modf(fa->darg,&ival);
		lval = (ULONG)ival;
		ultoa(lval,suffix,10);
		suffix += len = strlen(suffix);
		fa->strlen += len;
		*suffix++ = '.';

		frac *= pcismult[sigdig];

		ultoa(((ULONG)frac),suffix,10);

		/*** 'G' mode chops off trailing zeros ***/
		if(gmode && (suffix[1] == 0) && (suffix[0] == '0'))
		{
			*(--suffix) = 0;
			return(suffix);
		}

		if((len = strlen(suffix)) < sigdig)
		{
			memmove(suffix + (sigdig - len),suffix,len);
			memset(suffix,'0',sigdig - len);
		}

		suffix += sigdig;
		if(gmode) /* chop zeros */
		{
			while(*(--suffix) == '0')
				--sigdig;
			++suffix;
		}
		*suffix = 0;
		fa->strlen += sigdig + 1;
		return(suffix);
	}
}
static char do_fdouble(Formatarg *fa)
{
	start_double(fa);
	pcis_roundit(fa);
	add_dubldigits(fa,0);
	return(finish_num_string(fa));
}
static char do_edouble(Formatarg *fa, Boolean gmode)
{
double darg;
double odarg;
char *suffix;
int exp;
int len;

	exp = 0;
	odarg = fa->darg;
	start_double(fa);
	darg = fa->darg;

	/* shift over by base 10 radix until we get to d.dddd fit */

	if(darg != 0.0)
	{
		if(darg < 1.0)
		{
			for(;;)
			{
				--exp;
				darg *= 10.0;
				if(darg >= 1.0)
					break;
			}
		}
		else while(darg >= 10.0)
		{
			++exp;
			darg /= 10.0;
		}
		fa->darg = darg;
	}
	else
	{
		exp = 1;
	}

	pcis_roundit(fa);
	if(fa->darg >= 10.0)
	{
		fa->darg /= 10.0;
		++exp;
	}

	if(gmode && (exp >= -4 && exp < fa->precis))
	{
		fa->darg = odarg;
		start_double(fa);
		pcis_roundit(fa);
		add_dubldigits(fa,1);
		return(finish_num_string(fa));
	}

	suffix = add_dubldigits(fa,gmode);

	if(isupper(*(fa->fmt)))
		*suffix++ = 'E';
	else
		*suffix++ = 'e';

	if(exp < 0)
	{
		*suffix++ = '-';
		exp = -exp;
	}
	else
		*suffix++ = '+';

	itoa(exp,suffix,10);
	if((len = strlen(suffix)) < 3)
	{
		memmove(suffix + (3 - len),suffix,len);
		memset(suffix,'0',3 - len);
		suffix[3] = 0;
	}
	fa->strlen += 5;
	return(finish_num_string(fa));
}
static char do_parse_stars(Formatarg *fa)

/* this function is only used in parse mode */
{
	if(fa->parse_stars)
	{
		--fa->parse_stars;
		return((char)(('*'<<3)|FT_INT));
	}
	fa->getchar = fa->root;
	return(((*fa->fmt++)<<3)|fa->strbuf[0]);
}
char geta_fmtchar(Formatarg *fa)

/* outputs char or gets the format spec and calls function for argument type */
{
char retchar;
USHORT flags;

get_another_char: /* here from below to reset things and dump format chars */

	retchar = *(fa->fmt)++;

	if(retchar != '%')
		return(retchar);

	/* get optional leading flags */
	fa->pflags = 0;
	fa->width = 0;
	fa->precis = 0;

	for(;;)
	{
		switch(*(fa->fmt))
		{
			/* conversion "flag" specifiers */
			case ' ':
				flags = FSPEC_SPACE;
				goto check_fflags;
			case '-':
				flags = FSPEC_MINUS;
				goto check_fflags;
			case '+':
				flags = FSPEC_PLUS;
				goto check_fflags;
			case '#':
				flags = FSPEC_PSIGN;
				goto check_fflags;
			case '0':
				if(fa->pflags & FSPEC_GOTDOT)
					goto get_fmt_precis;
				flags = FSPEC_ZERO;

			check_fflags:
				if(flags & fa->pflags || fa->pflags & FSPEC_FDONE)
					goto error;
				break;

			/* conversion field width and precision specifiers */

			case '.':
				flags = FSPEC_GOTDOT;
				goto check_ctflags;
			case '*':
				if(fa->pflags & FSPEC_GOTDOT)
				{
					flags = FSPEC_GOTPCIS;
					if(fa->mflags & FA_PARSE_MODE)
						goto inc_pstars;

					else if(0 == (fa->precis = va_arg(fa->args,int)))
						flags = FSPEC_0PCIS;
				}
				else
				{
					flags = FSPEC_GOTWID;
					if(fa->mflags & FA_PARSE_MODE)
						goto inc_pstars;

					if(0 == (fa->width = va_arg(fa->args,int)))
						flags = FSPEC_0WID;
				}
				goto check_ctflags;
			inc_pstars:
				++fa->parse_stars;
				goto check_ctflags;
			default:

				if(!isdigit(*(fa->fmt)))
				{
					if(fa->pflags)
						goto error;
					retchar = *(fa->fmt);
					goto done;
				}
				if(fa->pflags & FSPEC_GOTDOT)
				{
			get_fmt_precis:
					flags = FSPEC_GOTPCIS;
					if(0 == (fa->precis = get_fmtint(fa)))
						flags = FSPEC_0PCIS;
				}
				else
				{
					flags = FSPEC_GOTWID;
					if(0 == (fa->width = get_fmtint(fa)))
						flags = FSPEC_0WID;
				}
				--fa->fmt; /* since get_fmtint() increments */

			check_ctflags:
				if(flags & fa->pflags || fa->pflags & FSPEC_WPDONE)
					goto error;
				flags |= FSPEC_FDONE;
				break;
			/*** argument size specifiers ****/

			case 'h':
				flags = FSPEC_SHORT;
				goto check_szflags;
			case 'l':
				flags = FSPEC_LONG;
				goto check_szflags;
			case 'L':
				flags = FSPEC_DLONG;
			check_szflags:
				if(flags & fa->pflags)
					goto error;
				flags |= FSPEC_WPDONE;
				break;

			/**** finally  --  phew~~ conversion types ****/
			case 'n':
				if(fa->pflags & ~(FSPEC_SHORT|FSPEC_WPDONE))
					goto error;

				if(fa->mflags & FA_PARSE_MODE)
				{
					if(fa->pflags & FSPEC_SHORT)
						retchar = FT_SHORT_PTR;
					else
						retchar = FT_INT_PTR;
					goto got_type;
				}

				/* note that fa->count is pre-incremented */

				if(fa->pflags & FSPEC_SHORT)
					*(va_arg(fa->args,short*)) = fa->count - 1; 
				else
					*(va_arg(fa->args,int*)) = fa->count - 1;

				goto reset; /* we don't need char, output only */

			/**** floating point types ****/
			case 'g':
			case 'G':
				if(fa->mflags & FA_PARSE_MODE)
					goto got_double_type;
				fa->darg = va_arg(fa->args,double);
				retchar = do_edouble(fa,1);
				goto done;
			case 'e':
			case 'E':
				if(fa->mflags & FA_PARSE_MODE)
					goto got_double_type;
				fa->darg = va_arg(fa->args,double);
				retchar = do_edouble(fa,0);
				goto done;
			case 'f':
				if(fa->mflags & FA_PARSE_MODE)
					goto got_double_type;
				fa->darg = va_arg(fa->args,double);
				retchar = do_fdouble(fa);
				goto done;

			/**** integer types ****/

			case 'c':
				if(fa->pflags & ~(FSPEC_MINUS|FSPEC_GOTWID|FSPEC_FDONE))
					goto error;
				if(fa->mflags & FA_PARSE_MODE)
					goto got_int_type;
				fa->getchar = do_char;
				retchar = do_char(fa);
				goto done;

			case 'd':
			case 'i':
				if(fa->mflags & FA_PARSE_MODE)
					goto got_int_type;
				retchar = do_int(fa,10);
				goto done;
			case 'u':
				if(fa->mflags & FA_PARSE_MODE)
					goto got_int_type;
				retchar = do_int(fa,-10);
				goto done;
			case 'o':
				if(fa->mflags & FA_PARSE_MODE)
					goto got_int_type;
				retchar = do_int(fa,-8);
				goto done;

			case 'p': /* pointer for this we use a long */
				if(fa->mflags & FA_PARSE_MODE)
				{
					retchar = FT_VOID_PTR;
					goto got_type;
				}
				fa->pflags |= FSPEC_LONG;
				fa->pflags &= ~(FSPEC_SHORT);
				goto do_hexnum;

			case 'X':
			case 'x':
				if(fa->mflags & FA_PARSE_MODE)
					goto got_int_type;
			do_hexnum:
				retchar = do_int(fa,-16);
				goto done;

			/* string type */

			case 's':
				if(fa->pflags & ~(FSPEC_MINUS|FSPEC_FDONE
								 |FSPEC_GOTDOT|FSPEC_GOTWID|FSPEC_GOTPCIS
								 |FSPEC_WPDONE))
				{
					goto error;
				}
				if(fa->mflags & FA_PARSE_MODE)
				{
					retchar = FT_CHAR_PTR;
					goto got_type;
				}
				fa->str = va_arg(fa->args,char*);
				if(fa->str == NULL)
					fa->str = "NULL";
				else if(!*fa->str)
				{	
					if(!(fa->pflags & FSPEC_GOTWID)) /* print nothing */
						goto reset;
					fa->getchar = trail_spaces;
					retchar = trail_spaces(fa);
					goto done;
				}
				retchar = do_string(fa);
				goto done;
		}

		++fa->fmt;
		fa->pflags |= flags;
		continue;
	reset:
		fa->mflags |= FA_FORMAT_DONE;
		++fa->fmt;
		goto get_another_char; /** go back to top and do next one **/
	}

done:
	fa->mflags |= FA_FORMAT_DONE;
	++fa->fmt;
	return(retchar);

got_double_type:
	retchar = FT_DOUBLE;
	goto got_type;
got_int_type:

	/* for now the double long feature is un implemented */

	if(fa->pflags & FSPEC_LONG)
		retchar = FT_LONG;
	else
		retchar = FT_INT; 

got_type:
	if(fa->parse_stars)
	{
		fa->strbuf[0] = retchar;
		fa->getchar = do_parse_stars;
		return(do_parse_stars(fa));
	}
	return(((*fa->fmt++)<<3) | retchar);

error:
	while(*(--fa->fmt) != '%'); /* go back to start of this format */
	if(fa->mflags & FA_ABORT_ON_ERROR)
	{
		fa->error = Err_format_invalid;
		return(0);
	}
	++fa->fmt;
	if(fa->mflags & FA_PARSE_MODE)
	{
		fa->parse_stars = 0;
		return(geta_fmtchar(fa)); /* note recursion, this will only go one level
								 * since the next char will NOT be a '%', 
								 * a %% will be passed through as '%' */
	}
	return('%');
}
void init_formatarg(Formatarg *fa, char *fmt)
{
	fa->mflags = 0;
	fa->fmt = fmt;
	fa->root = fa->getchar = geta_fmtchar;
	fa->count = 0;
	fa->error = Success;
}
char fa_getchar(Formatarg *fa)
{
	return(fa_getc(fa));
}

#ifdef TESTMAIN /********************* for testing only *******************/

int fa_sprintf(char *buf,int maxlen, Formatarg *fa)

/* a self limiting sprintf that limits output size to maxlen. If maxlen < 0
 * it doesn't limit. output will NOT be terminated by a null if buffer
 * is stuffed to maxlen, done this way to allow filling without overflow */
{
	while((*buf++ = fa_getc(fa)) != 0)
	{
		/*
		*buf++ = FT_TYPE(c) + '0';
		printf("(%c)", 0x60|FT_FMTCHAR(c));
		*/
		if(fa->count > ((unsigned)maxlen))
			break;
	}
	return(fa->count - 1);
}

/* #define puts(x) */

rintf(char *fmt,...)
{
char buff[300];
char *cout;
Formatarg fa;

	/* init_format_parse(&fa,fmt); */

	start_formatarg(fa,fmt); 
	cout = buff + fa_sprintf(buff,299,&fa);
	cout[-1] = 0;
	puts(buff); 
	end_formatarg(fa);
}
srintf(char *fmt,...)
{
char buff[300];
char *cout;
va_list args;

	va_start(args, fmt);
	cout = buff + vsprintf(buff,fmt,args);
	cout[-1] = 0;
	puts(buff); 
	va_end(args);
}

void main(int argc, char **argv)
{
USHORT i;
double d;
int mask, cs;

	d = -0.00001;
	if(argc > 1)
		goto do_printf;

	printf("mine \n");
	for(i = 0;;++i)
	{
/*
	rintf("%*X%*.*f%n%hn%G%g%e%E\n");
		rintf("r%10.0f%20e%20g%10f\n",
				d,	d,	(1000000000.0 * d), d);			
		d += .000001;
*/
rintf("r%6hu%5hx%5hx%5hx%5hx%5hx%5hx%5hx%5hx%5hx%5hx%5hx%5hx%5hx%n\n",
	    i, i, i, i, i, i, i, i, i, i, i, i, i, i, &cs ); 
#ifdef puts
	if((i & 0x01F) == 0x010)
		printf("?%ud ", i); 
#endif
	}

do_printf:

	printf("theirs \n");
	for(i = 0;;++i)
	{
/*
		srintf("p%10.0f%20e%20g%10f\n",
				d,	d,	(1000000000.0 * d), d);			
		d += .000001;
*/
srintf("r%6hu%5hx%5hx%5hx%5hx%5hx%5hx%5hx%5hx%5hx%5hx%5hx%5hx%5hx%n\n",
	    i, i, i, i, i, i, i, i, i, i, i, i, i, i, &cs );
#ifdef puts
	if((i & 0x01F) == 0x010)
		printf("!p%ud ", i); 
#endif
	}

}
#endif /* TESTMAIN */

