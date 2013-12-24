/*****************************************************************************
 * chopper.c - Routines to read lines and manipultate strings in lines.
 *
 *	po_get_csource_line()		- The source of lines for the preprocessor
 *	po_chop_to()		- Chop chars from line into word, up to specified char
 *	po_skip_space() - Return pointer to char following leading whitespace
 *
 * MAINTENANCE
 *	09/01/90	(Ian)
 *				The check for line-too-long is now only done if there is no
 *				\n at EOL, since by definition if we got \n it's a good line.
 *
 *				Moved the setting of t->line_buf to the top of the function,
 *				so that when we report an error such as 'NL in string' it
 *				displays the line on which the error occurred.
 *	09/07/90	(Ian)
 *				Single-quoted strings are now handled like double-quoted:
 *				they must be terminated on the same line they start on.
 *				Also, fixed bug that was causing quotes to be checked inside
 *				of comments, yielding bogus 'NL in string' errors.
 *	10/01/90	(Ian)
 *				Fixed a bug that was causing bogus 'line too long' errors in
 *				big multiline comments.  Inserted 'icount = 0;' just before
 *				the continue statement in the read loop.
 ****************************************************************************/

#include <ctype.h>
#include <string.h>
#include "poco.h"

#ifdef DEADWOOD

static Boolean po_is_all_white(register char *buf)
/*****************************************************************************
 * is_all_white - Decide whether string is all whitespace.
 ****************************************************************************/
{
while (isspace(*buf))
	++buf;
return(*buf == '\0');
}
#endif

char *po_get_csource_line(Poco_cb *pcb)
/*****************************************************************************
 * po_get_csource_line - Like fgets(), but understands the C concept of a 'line'.
 ****************************************************************************/
{
Boolean 	splice		= FALSE;			/* Are we splicing lines?		*/
Boolean 	mlcomment	= FALSE;			/* Are we doing ml comment? 	*/
int 		buflen		= SZTOKE-1; 		/* Max logical line size.		*/
int 		icount		= 0;				/* Significant character counter*/
Token		*t			= &pcb->t;			/* -> Token struct in pcb		*/
File_stack	*fstack 	= t->file_stack;	/* -> File_stack struct in pcb	*/
char		*lbuf		= t->line_b1;		/* -> Line buffer in Token		*/
char		*buf		= lbuf; 			/* -> Cur I/O location in buf	*/
char		*subbuf;						/* -> Cur scan location in buf	*/
char		*endcomment;					/* -> End of inline comment 	*/
char		c;

t->line_buf = lbuf; 		/* So that errors can display the current line	*/

/*
 * if we report an error, make sure it carries the right line
 * number in the report.  if we don't report an error, this'll
 * get zeroed before we return to the caller.
 */

pcb->error_line_number = fstack->line_count;

/*----------------------------------------------------------------------------
 * Loop to assemble a significant logical line.
 *	This loop will continue until a non-blank logical line is built.  A
 *	logical line has all comments stripped from it, and has had continuation
 *	lines spliced together.
 *--------------------------------------------------------------------------*/

do	{

	/*------------------------------------------------------------------------
	 * Loop to splice together physical lines that are related.
	 *	If the outer level loop has set the 'mlcomment' (multi-line comment)
	 *	flag, we stay in this loop until we read a line that has the closing
	 *	comment delimiter on it.
	 *	This loop handles splicing of physical lines ending in '\' into a
	 *	single logical line.
	 *----------------------------------------------------------------------*/

	icount = 0;

	do	{


		if (NULL == fgets(buf+icount, buflen-icount, fstack->source.file))
			{
			if (mlcomment)					/* EOF in multi-line comment	*/
				po_say_fatal(pcb, "EOF in comment");
			else if (splice)				/* EOF instead of continuation	*/
				po_expecting_got_str(pcb, "EOF", "continuation line");
			else							/* Normal EOF					*/
				{
				lbuf = NULL;
				goto NORMAL_EXIT;
				}
			}
		else								/* Not EOF, we got a line....	*/
			{

			pcb->error_line_number = ++fstack->line_count; /* Count the line*/

			icount = strlen(buf) - 1;		/* Size of string, less \n char.*/

			if ('\n' == buf[icount])        /* If the last char is a \n,    */
				{							/* truncate at the \n; count has*/
				buf[icount] = '\0';         /* already been adjusted above. */
				}
			else
				{
				if (buflen == 2 + icount)			/* If there is no \n at */
					{								/* EOL, check for buffer*/
					po_say_fatal(pcb,"line too long"); /* overflow. If not  */
					}								/* it means last line	*/
				++icount;							/* has no CRLF; adjust	*/
				}									/* the count to match.	*/

			if (icount == 0)
				continue;							/* Empty line-try again.*/

			if (mlcomment)					/* If doing multi-line comment	*/
				{
				if (NULL == strstr(buf, "*/"))
					{
					icount = 0; 			/* no significant chars on line */
					continue;				/* No delim, read next physline */
					}
				else
					mlcomment = FALSE;		/* Found delim, end of ml state */
				}

			if ('\\' == buf[icount-1])      /* If the last char on the line */
				{							/* is a backslash, splice the	*/
				--icount;					/* next physical line onto this */
				splice = TRUE;				/* line, overlaying the \ char. */
				}
			else
				splice = FALSE;
			}

		} while (splice || mlcomment || icount == 0);

	/*------------------------------------------------------------------------
	 * Loop to process double-quotes and comments.
	 *----------------------------------------------------------------------*/

	subbuf = lbuf;
	while (NULL != (subbuf = po_cmatch_scan(subbuf)))
		{

		/*--------------------------------------------------------------------
		 * Handle quotes --
		 *	 We have to ignore everything inside the quotes, so we scan for
		 *	 a closing quote, which must be on the same line as the opener.
		 *	 We also watch out for escaped quotes (\") and ignore them.
		 *------------------------------------------------------------------*/

		c = *subbuf;

		if (c == '"' || c == '\'')
			{
			Boolean found_end = FALSE;

			while (found_end == FALSE)
				{
				++subbuf;
				if (NULL == (subbuf = strchr(subbuf, c)))
					po_say_fatal(pcb, "strings cannot span lines without continuation (\\)");
				if (!('\\' == subbuf[-1] && '\\' != subbuf[-2]))
					{
					++subbuf;
					found_end = TRUE;
					}
				} /* END while (found_end == FALSE) */

			} /* END handling for a double-quote character */

		else	/* Not a quote, must've been a slash... */

			/*----------------------------------------------------------------
			 * Handle comments --
			 *	 The C++ type comment (//) is allowed because it's easy; we
			 *	 just truncate the line at that point.
			 *	 When we encounter a normal C comment, we scan the remainder
			 *	 of the line to see if it's closed on the same line.  If so,
			 *	 we wipe the comment and continue.	If the comment is not
			 *	 closed on the same line, we set the multi-line comment flag
			 *	 and loop back to perform what is essentially a splice
			 *	 operation.  When the jgetline() routine sees the mlcomment
			 *	 flag, it knows not to return until a line containing the
			 *	 closing comment delimiter is seen.
			 *--------------------------------------------------------------*/

			{
			switch (subbuf[1])
				{
				case '/':                       /* C++ comment (//)...      */

					*subbuf = '\0';
					break;

				case '*':                       /* Normal C comment...     */

					subbuf +=2;
					if (NULL == (endcomment = strstr(subbuf, "*/")))
						{
						*subbuf = '\0';         /* We have a multi-line     */
						buflen -= strlen(buf);	/* comment, set up to splice*/
						buf = subbuf;			/* more physical lines onto */
						mlcomment = TRUE;		/* current logical line.	*/
						goto ENDLOOP;
						}
					else
						{
						subbuf -=2;
						*(++endcomment) = ' ';          /* In-line comment, */
						strcpy(subbuf, endcomment); 	/* replace w/space. */
						}
					break;

				default:						/* Not a comment at all...	*/

					++subbuf;
					break;

				} /* END switch (subbuf[1]) */

			} /* END handling for a slash character */

		} /* END while slash or quote found in buffer */
ENDLOOP:
	;
	} while (TRUE == mlcomment || NULL == po_skip_space(buf = lbuf));

#ifdef DEBUG_JGETS
	printf("po_get_csource_line: '%s'\n",lbuf);
#endif

NORMAL_EXIT:

	pcb->error_line_number = 0; /* no error, clear this */

	return lbuf;
}

#ifdef DEADWOOD

char *po_skip_space(register char *line)
/*****************************************************************************
 * po_skip_space - Skip leading whitespace.
 ****************************************************************************/
{
register char c;

if (!line)
	return(NULL);
for (;;)
	{
	if (0 == (c = *line++))
		return(NULL);
	if (!isspace(c))
		break;
	}
return(--line);
}

#endif

char *po_chop_to(char *line, char *word, char letter)
/*****************************************************************************
 * po_chop_to - Copy chars from line to word until specified delim is found.
 ****************************************************************************/
{
register char c;

for (;;)
	{
	if (0 == (c = *line++))
		break;
	if (c == letter)
		break;
	*word++ = c;
	}
*word = '\0';
return(--line);
}
