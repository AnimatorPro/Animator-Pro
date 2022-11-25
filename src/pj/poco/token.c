/*****************************************************************************
 *
 * token.c - Yer basic tokenizer.
 *
 *	htol()			- Convert hex-ascii string to long value.
 *	tokenize_word() - Chop a C token from a line buffer to a word buffer,
 *					  categorize it by token type.
 *
 *	The tokenize_word() function is used by the preprocessor and the parser.
 *
 * MAINTENANCE
 *	08/19/90	(Ian)
 *				Eliminated next_token() routine.  It duplicated some
 *				of the logic in po_lookup_freshtoken() (its caller), so
 *				po_lookup_freshtoken was tweaked to handle everything.
 *	08/22/90	(Ian)
 *				Moved the po_say_err() function to pocoface.c.
 *	09/07/90	(Ian)
 *				Made htol() cope with leading 0x if it exists.
 *	09/20/90	(Ian)
 *				Tweak to allow 'L' suffix on floating point numbers, per ANSI.
 *	10/22/90	(Ian)
 *				Added qstring parameter to tokenize_word routine.  This is
 *				part of the support for multi-token lookaheads in poco; it
 *				allows normal tokens to be tokenized directly into the small
 *				ctoke field in the Tstack structures, but (possibly long)
 *				quoted strings go into a separate buffer.
 *	10/24/90	(Ian)
 *				Bugfixes:  in get_digits(), we will stop getting digits when
 *				MAX_SYM_LEN chars have be gotten.  This should never happen,
 *				as we should never get a valid number that long.  Still, if
 *				we do, this prevents us from overflowing our caller's buffer,
 *				and hopefully the caller will catch the error in some other
 *				way (like, when the next token is the rest of the digits
 *				sequence).	Also, when chopping out a C-symbol token, we
 *				now keep going until we hit the terminating token, but
 *				we stop stuffing chars into the buffer after MAX_SYM_LEN
 *				chars have been done (we were stopping mid-token, leaving
 *				the rest of the symbol name as the next token, causing errors).
 *	10/28/90	(Ian)
 *				Added routine translate_escape(), and added calls to it from
 *				the tokenize_word routine.
 *				Also, added handling of 'F' suffix in get_digits(), per ANSI.
 *	10/30/90	(Ian)
 *				Added new parameter 'plen' to tokenize_word.  This is a
 *				pointer to a SHORT; the routine will return the number of
 *				characters placed into the word buffer NOT including the
 *				terminating \0 into the SHORT that this parm points to, as
 *				long as the parm is not NULL.  This helps a fix poco bugs
 *				involving string concat, and the fact that char a[] = "a\0b";
 *				needs to allocate a 4 byte array, not 2 bytes.
 ****************************************************************************/

#include <ctype.h>
#include "token.h"

#define SZTOKE			512 	/* max line length, token length */
#define MAX_SYM_LEN 	40		/* max significant chars in sym name */

/*----------------------------------------------------------------------------
 * Some useful macros missing from most ctype.h files...
 *--------------------------------------------------------------------------*/

#ifndef isoctal
  #define isoctal(a) ((a) >= '0' && (a) <= '7')
#endif

#ifndef iscsymf
  #define iscsymf(x) ( isalpha(x) || (x) == '_')
#endif

#ifndef iscsym
  #define iscsym(x)  ( isalnum(x) || (x) == '_')
#endif

/*****************************************************************************
 *
 * htol - Convert an ASCII hex string to a long integer value.
 *
 *	This routine counts on finding only uppercase characters and digits; the
 *	getdigits() routine below will ensure this is the case.  If this routine
 *	is made globally-visible, bear this in mind.
 *
 ****************************************************************************/

long htol(const char *s)
{
long acc = 0;
int  c;

if (s[0] == '0' && s[1] == 'X')
	s += 2;

while (isxdigit(c = *s++))
	{
	acc <<= 4;
	if (isdigit(c))
		acc += c - '0';
	else
		acc += c - 'A' + 10;
	}
return(acc);
}

static char translate_escape(char **s)
/*****************************************************************************
 * translate the character(s) after a '\' into a single character.
 ****************************************************************************/
{
char	*in_str;
short	counter;
char	inchar;
char	outchar;

	in_str = *s;
	switch (inchar = *in_str++)
		{
		case 'a':   outchar = '\a';  break;
		case 'b':   outchar = '\b';  break;
		case 'f':   outchar = '\f';  break;
		case 'n':   outchar = '\n';  break;
		case 'r':   outchar = '\r';  break;
		case 't':   outchar = '\t';  break;
		case 'v':   outchar = '\v';  break;
		case '?':   outchar = '\?';  break;
		case '\\':  outchar = '\\';  break;
		case '\'':  outchar = '\'';  break;
		case '\"':  outchar = '\"';  break;
		case 'x':
			outchar = 0;
			inchar = *in_str;
			while (isxdigit(inchar))
				{
				if (inchar <= '9')
					inchar -= '0';
				else if (inchar <= 'F')
					inchar -= 'A' - 10;
				else
					inchar -= 'a' - 10;
				outchar = (outchar << 4) | inchar;
				inchar = *++in_str;
				}
			break;
		default:
			if (isoctal(inchar))
				{
				counter = 4;
				outchar = 0;
				while (--counter)
					{
					outchar = (outchar << 3) | (inchar & 0x07);
					inchar = *in_str;
					if (isoctal(inchar))
						++in_str;
					else
						break;
					}
				}
			else
				outchar = inchar;
			break;
		}

	*s = in_str;
	return outchar;
}

/*****************************************************************************
 *
 * get_digits - Copy a sequence of digits to a word buffer.
 *
 *	This routine understands decimal, hex, and floating-point strings.
 *	It also understands C suffixes (L & U) that indicate the datatype of a
 *	constant.  (U is currently accepted but ignored).
 *	This routine both isolates the sequence of digits and sets the token type
 *	(INT, LONG, DOUBLE) of the string.
 *	The number of digits placed into the word buffer is returned.
 ****************************************************************************/

static int get_digits(char *line, char *word, SHORT *ttype)
{
enum {DECIMAL, HEX, FLOAT} numtype; 			/* String type				*/
int count = 0;									/* Count of chars in word	*/
register int c; 								/* Current char 			*/

numtype = DECIMAL;								/* Assume decimal number	*/
*ttype	= TOK_INT;								/* Assume short-int datatype*/

if (*line == '0')                               /* Some special handling... */
	{											/* If the sequence starts	*/
	*word++ = *line++;							/* with 0x it's hex string. */
	++count;
	if ('X' == (c = toupper(*line)))
		{
		*word++ = c;
		++count;
		++line;
		numtype = HEX;
		}
	}

for (;;)
	{
	c = toupper(*line++);

	if (c == 'L')                               /* L allowed on all types of*/
		{										/* numbers, be we ignore it */
		if (numtype != FLOAT)					/* when specified with a	*/
			*ttype = TOK_LONG;					/* float type number.		*/
		}

	else if (c == 'U' && numtype != FLOAT)      /* U allowed in non-floats, */
		{}										/* is ignored, for now. 	*/

	else if (c == 'F' && numtype != HEX)        /* F allowed in decimal or  */
		{										/* float types, implies a	*/
		numtype = FLOAT;						/* change from decimal to	*/
		*ttype = TOK_DOUBLE;					/* float type.				*/
		}

	else if (c == '.' && numtype == DECIMAL)    /* Dot allowed in decimal,  */
		{										/* when encountered, implies*/
		numtype = FLOAT;						/* a change to a float type.*/
		*ttype	= TOK_DOUBLE;
		}

	else if (c == 'E' && numtype == FLOAT)      /* E allowed in floats only.*/
		{
		if (*line == '-')       /* A minus sign is allowed after an E in a  */
			{					/* floating point constant.  If a minus sign*/
			*word++ = c;		/* is present, copy the E char, and let the */
			++count;			/* minus sign get copied at the end of the	*/
			c = *line++;		/* the for(;;) loop.						*/
			}
		}

	else if ((numtype != HEX && !isdigit(c)) || /* Terminate loop if char is */
			 (numtype == HEX && !isxdigit(c)))	/* non of the above, & is not*/
			break;								/* in charset for its type.  */

	*word++ = c;								/* Copy char to word buffer.*/
	++count;

	if (count > MAX_SYM_LEN-2)
		break;
	}

return(count);
}

/*****************************************************************************
 *
 * tokenize_word - Chop a C token from a line into a word buffer, classify it.
 *
 *	This routine is used by both the preprocessor (during macro expansion)
 *	and the parser.  The preprocessor always passes a value of TRUE for the
 *	quote parameter (preserve quotes), while the parser passes FALSE.
 *
 *	This routine chops a sequence of one or more chars out of a line, places
 *	the sequence (null-termed) into a word buffer, and roughly categorizes
 *	the token type.  The token type will be in one of these categories:
 *		C operators (+, =, ++, >>=, etc)
 *		Constants	(numbers of all types; quoted string and char constants)
 *		Undefined	(keywords, types, symbols, and ellipsis)
 *	It is important to note that this routine doesn't handle the value of a
 *	constant, it only isolates the sequence of chars.  Also, this routine
 *	is blind to keywords, type and symbols; it lumps them all into the
 *	generic category of TOK_UNDEF.
 *
 *	For non-quoted-string tokens, a maximum of MAX_SYM_LEN-1 characters will
 *	be placed into the token buffer.  For quoted strings, a maximum of
 *	SZTOKE-3 characters (plus quotes) will be placed into the buffer pointed
 *	to by the qstring parameter.  If the qstring parameter is NULL, the
 *	string will be placed into the regular word buffer.  In either case, the
 *	buffer is assumed to be large enough to hold the characters.  (Poco uses
 *	this feature to tokenize normal things right into its Tstack structure,
 *	while placing quoted strings (the only thing that might be longer than
 *	MAX_SYM_LEN) go into a special large buffer instead of right into the
 *	Tstack structure.)
 *
 *	A pointer to the remainder of the line is returned, or NULL if nothing is
 *	left on the line.  A NULL return implies that nothing was placed into the
 *	word buffer (eg, you won't get a word and a NULL return on the same call).
 ****************************************************************************/

char *tokenize_word(char *line,    /* (in) -> current line position */
					 char *word,    /* (in) -> output token buffer */
					 char *qstring, /* (in) -> quoted string o/p buffer */
					 SHORT	*plen,		   /* (out) # of bytes put in word buf */
					 SHORT	*ttype, 	   /* (out) token type				   */
					 Boolean quote		   /* (in)	preserve quotes on string? */
				   )
{
char	*wrkptr;
char	*sword = word;
int 	toklen;
SHORT	toktype;
register unsigned int c;
UBYTE	c1;
UBYTE	c2;

/*----------------------------------------------------------------------------
 * Skip leading whitespace, get the first character, return NULL if no char.
 *--------------------------------------------------------------------------*/

	while (isspace(*line) )
		++line;

	if ('\0' == (c = *line))
		{
		toktype = TOK_EOF;
		line = NULL;
		goto OUT;
		}

/*----------------------------------------------------------------------------
 * Handle keywords, types, symbols
 *--------------------------------------------------------------------------*/

	if (iscsymf(c))
		{
		line = po_chop_csym(line, word, MAX_SYM_LEN-1, &wrkptr);
		word = wrkptr;
		toktype = TOK_UNDEF;
		}
#ifdef DEADWOOD
	else if (iscsymf(c))
		{
		toktype = TOK_UNDEF;
		*word++ = c;
		++line;
		toklen = MAX_SYM_LEN;
		for (;;)
			{
			c = *line;
			if (iscsym(c))
				{
				++line;
				if (toklen)
					{
					*word++ = c;
					--toklen;
					}
				 }
			else
				break;
			}
		}

#endif /* DEADWOOD */

/*----------------------------------------------------------------------------
 * Handle numeric constants
 *--------------------------------------------------------------------------*/

	else if (isdigit(c))
		{
		toklen = get_digits(line,word,&toktype);
		line  += toklen;
		word  += toklen;
		}

/*----------------------------------------------------------------------------
 * Handle C operators and string/char constants...
 * This processes non-alphanumeric characters.	Most will be passed through
 * as single character tokens.	Some, like ==, !=, >= and <= are easier to
 * handle here than in parser (which only has a one-token look-ahead).
 * Also, quoted strings and chars are now handled in this switch statement.
 *--------------------------------------------------------------------------*/

	else
		{

		c1 = line[1];		/* lookahead characters */
		c2 = line[2];

		switch (c)
			{

			case '"':                       /* note: a shortcut in this loop */
											/* assumes that a quoted string  */
				if (qstring != NULL)		/* could never be longer than	 */
					sword = word = qstring; /* SZTOKE-3 chars.				 */
				toktype = TOK_QUO;
				if (quote)
					*word++ = '"';
				++line;
				toklen = SZTOKE-3; /* room for nullterm and two quotes */
				while(--toklen)
					{
					c = *line++;
					if (c == 0)
						break;
					else if (c == '\\')
						{
						if (quote)
							{
							*word++  = c;
							*word++ = *line++;
							}
						else
							{
							wrkptr = line;
							*word++ = translate_escape(&wrkptr);
							line = wrkptr;
							}
						}
					else if (c == '\"')
						{
						break;
						}
					else
						{
						*word++ = c;
						}
					}
				if (quote)
					*word++ = '"';
				break;

			case '\'':

				if (quote)
					*word++ = '\'';
				toktype = TOK_SQUO;
				++line;
				for (;;)
					{
					c = *line++;
					if (c == 0)
						break;
					else if (c == '\\')
						{
						if (quote)
							{
							*word++  = c;
							*word++ = *line++;
							}
						else
							{
							wrkptr = line;
							*word++ = translate_escape(&wrkptr);
							line = wrkptr;
							}
						}
					else if (c == '\'')
						{
						break;
						}
					else
						{
						*word++ = c;
						}
					}
				if (quote)
					*word++ = '\'';
				break;

			case '.':

				if (c1 == '.' && c2 == '.')
					{
					toktype = TOK_UNDEF;
					goto THREECHAR;
					}
				else
					goto SIMPLE;

			case '%':

				if (c1 == '=') /* mod-equals */
					{
					toktype = TOK_MOD_EQUALS;
					goto TWOCHAR;
					}
				else
					goto SIMPLE;

			case '/':

				if (c1 == '=') /* div-equals */
					{
					toktype = TOK_DIV_EQUALS;
					goto TWOCHAR;
					}
				else
					goto SIMPLE;

			case '*':

				if (c1 == '=')
					{
					toktype = TOK_MUL_EQUALS;
					goto TWOCHAR;
					}
				else
					goto SIMPLE;

			case '+':

				if (c1 == '+')
					{
					toktype = TOK_PLUS_PLUS;
					goto TWOCHAR;
					}
				else if (c1 == '=')
					{
					toktype = TOK_PLUS_EQUALS;
					goto TWOCHAR;
					}
				else
					goto SIMPLE;

			case '-':

				if (c1 == '-')
					{
					toktype = TOK_MINUS_MINUS;
					goto TWOCHAR;
					}
				else if (c1 == '=')
					{
					toktype = TOK_MINUS_EQUALS;
					goto TWOCHAR;
					}
				else if (c1 == '>')
					{
					toktype = TOK_ARROW;
					goto TWOCHAR;
					}
				else
					goto SIMPLE;

			case '=':

				if (c1 == '=') /* double equals */
					{
					toktype = TOK_EQ;
					goto TWOCHAR;
					}
				else
					goto SIMPLE;

			case '!':

				if (c1 == '=') /* != */
					{
					toktype = TOK_NE;
					goto TWOCHAR;
					}
				else
					goto SIMPLE;

			case '<':

				if (c1 == '=') /* <= */
					{
					toktype = TOK_LE;
					goto TWOCHAR;
					}
				else if (c1 == '<')    /* << */
					{
					if (c2 == '=')
						{
						toktype = TOK_LSHIFT_EQUALS;
						goto THREECHAR;
						}
					else
						{
						toktype = TOK_LSHIFT;
						goto TWOCHAR;
						}
					}
				else
					goto SIMPLE;

			case '>':

				if (c1 == '=') /* >= */
					{
					toktype = TOK_GE;
					goto TWOCHAR;
					}
				else if (c1 == '>')    /* >> */
					{
					if (c2 == '=')
						{
						toktype = TOK_RSHIFT_EQUALS;
						goto THREECHAR;
						}
					else
						{
						toktype = TOK_RSHIFT;
						goto TWOCHAR;
						}
					}
				else
					goto SIMPLE;

			case '&':

				if (c1 == '&') /* logical and - && */
					{
					toktype = TOK_LAND;
					goto TWOCHAR;
					}
				else if (c1 == '=')
					{
					toktype = TOK_AND_EQUALS;
					goto TWOCHAR;
					}
				else
					goto SIMPLE;

			case '|':

				if (c1 == '|') /* logical or - || */
					{
					toktype = TOK_LOR;
					goto TWOCHAR;
					}
				else if (c1 == '=')
					{
					toktype = TOK_OR_EQUALS;
					goto TWOCHAR;
					}
				else
					goto SIMPLE;

			case '^':

				if (c1 == '=')
					{
					toktype = TOK_XOR_EQUALS;
					goto TWOCHAR;
					}
				else
					goto SIMPLE;
			default:
	SIMPLE:
				toktype = *word++ = c;
				++line;
				break;
			}
		}

OUT:

	*ttype = toktype;
	*word = 0;
	if (plen != NULL)
		*plen = word - sword;
	return line;


TWOCHAR:

	*word++ = c;
	*word++ = c1;
	line += 2;
	goto OUT;

THREECHAR:

	*word++ = c;
	*word++ = c1;
	*word++ = c2;
	line += 3;
	goto OUT;

}
