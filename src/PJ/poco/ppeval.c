/*****************************************************************************
 * ppeval.c - limited expression evaluator for preprocessor #if statements
 *
 * MAINTENANCE
 *	10/19/90	(Ian)
 *				Fixed order-of-evaluation glitches for && and || in
 *				pp_log_or() and pp_log_and() routines.
 ****************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include "token.h"

static long 	pp_exp(void);
extern void 	pp_say_fatal(char *fmt, ...);

static char 	*lbuf;
static char 	*tok;
static Boolean	reuse;
static SHORT	ttype;

static char 	missing_rparen[] = "missing )";

static Boolean pp_token(void)
/*****************************************************************************
 *
 ****************************************************************************/
{
if (reuse)
	return FALSE == (reuse = FALSE);	/* ie, return TRUE */
else
	{
	if (lbuf == NULL)
		return(FALSE);
	return (NULL != (lbuf = tokenize_word(lbuf, tok, NULL, NULL, &ttype, TRUE)));
	}
}

static long pp_atom(void)
/*****************************************************************************
 *
 ****************************************************************************/
{
long ret;

if (!pp_token())
	return 0;
if (ttype == TOK_LPAREN)
	{
	ret = pp_exp();
	if (!pp_token() || ttype != TOK_RPAREN)
		pp_say_fatal(missing_rparen);
	return ret;
	}
else if (isdigit(tok[0]))
	{
	if (tok[0] == '0' && tok[1] == 'X')
		return htol(tok);
	else
		return atol(tok);
	}
else
	{
	return 0;
	}
}

static long pp_not(void)
/*****************************************************************************
 *
 ****************************************************************************/
{
if (!pp_token())
	return 0;
if (ttype == '!')
	return !pp_atom();
else if (ttype == '~')
	return ~pp_atom();
else if (ttype == '-')
	return -pp_atom();
else
	{
	reuse = 1;
	return pp_atom();
	}
}

static long pp_multiply(void)
/*****************************************************************************
 *
 ****************************************************************************/
{
long ret;
long tmp;

ret = pp_not();
for (;;)
	{
	if (pp_token())
		{
		if (ttype == '/')
			{
			if (0 == (tmp = pp_not()))
				ret = 0;
			else
				ret /= tmp;
			}
		else if (ttype == '%')
			{
			if (0 == (tmp = pp_not()))
				ret = 0;
			else
				ret %= tmp;
			}
		else if (ttype == '*')
			ret *= pp_not();
		else
			{
			reuse = TRUE;
			return ret;
			}
		}
	else
		return ret;
	}
}

static long pp_plus(void)
/*****************************************************************************
 *
 ****************************************************************************/
{
long ret;

ret = pp_multiply();
for (;;)
	{
	if (pp_token())
		{
		if (ttype == '+')
			ret += pp_multiply();
		else if (ttype == '-')
			ret -= pp_multiply();
		else
			{
			reuse = 1;
			return ret;
			}
		}
	else
		return ret;
	}
}


static long pp_shift(void)
/*****************************************************************************
 *
 ****************************************************************************/
{
long ret;

ret = pp_plus();
for (;;)
	{
	if (pp_token())
		{
		if (ttype == TOK_RSHIFT)
			ret >>= pp_plus();
		else if (ttype == TOK_LSHIFT)
			ret <<= pp_plus();
		else
			{
			reuse = 1;
			return ret;
			}
		}
	else
		return ret;
	}
}

static long pp_compare(void)
/*****************************************************************************
 *
 ****************************************************************************/
{
long ret;

ret = pp_shift();
for (;;)
	{
	if (pp_token())
		{
		if (ttype == '<')
			ret = (ret <  pp_shift());
		else if (ttype == TOK_LE)
			ret = (ret <= pp_shift());
		else if (ttype == '>')
			ret = (ret >  pp_shift());
		else if (ttype == TOK_GE)
			ret = (ret >= pp_shift());
		else
			{
			reuse = 1;
			return ret;
			}
		}
	else
		return ret;
	}
}

static long pp_equality(void)
/*****************************************************************************
 *
 ****************************************************************************/
{
long ret;

ret = pp_compare();
for (;;)
	{
	if (pp_token())
		{
		if (ttype == TOK_NE)
			ret = (ret != pp_compare());
		else if (ttype == TOK_EQ)
			ret = (ret == pp_compare());
		else
			{
			reuse = 1;
			return ret;
			}
		}
	else
		return ret;
	}
}

static long pp_bit_and(void)
/*****************************************************************************
 *
 ****************************************************************************/
{
long ret;

ret = pp_equality();
for (;;)
	{
	if (pp_token())
		{
		if (ttype == '&')
			ret &= pp_equality();
		else
			{
			reuse = 1;
			return ret;
			}
		}
	else
		return ret;
	}
}

static long pp_bit_xor(void)
/*****************************************************************************
 *
 ****************************************************************************/
{
long ret;

ret = pp_bit_and();
for (;;)
	{
	if (pp_token())
		{
		if (ttype == '^')
			ret ^= pp_bit_and();
		else
			{
			reuse = 1;
			return ret;
			}
		}
	else
		return ret;
	}
}

static long pp_bit_or(void)
/*****************************************************************************
 *
 ****************************************************************************/
{
long ret;

ret = pp_bit_xor();
for (;;)
	{
	if (pp_token())
		{
		if (ttype == '|')
			ret |= pp_bit_xor();
		else
			{
			reuse = 1;
			return ret;
			}
		}
	else
		return ret;
	}
}

static long pp_log_and(void)
/*****************************************************************************
 *
 ****************************************************************************/
{
long ret;

ret = pp_bit_or();
for (;;)
	{
	if (pp_token())
		{
		if (ttype == TOK_LAND)
			{
			ret = pp_bit_or() && ret;
			}
		else
			{
			reuse = 1;
			return ret;
			}
		}
	else
		return ret;
	}
}


static long pp_log_or(void)
/*****************************************************************************
 *
 ****************************************************************************/
{
long ret;

ret = pp_log_and();
for (;;)
	{
	if (pp_token())
		{
		if (ttype == TOK_LOR)
			{
			ret = pp_log_and() || ret;
			}
		else
			{
			reuse = 1;
			return ret;
			}
		}
	else
		return ret;
	}
}

static long pp_exp(void)
/*****************************************************************************
 *
 ****************************************************************************/
{
return(pp_log_or());
}

long po_pp_eval(char *line, char *buf)
/*****************************************************************************
 *
 ****************************************************************************/
{
/* null expression evaluates to zero  */
if (NULL == tokenize_word(line, buf, NULL, NULL, &ttype, TRUE))
	return 0;

reuse = 0;
lbuf  = line;
tok   = buf;
return(pp_exp());
}
