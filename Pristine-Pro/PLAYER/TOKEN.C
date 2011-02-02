#include "player.h"
#include "gfx.h"
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

static void skip_leading_white()
{
int inchar;

	for(;;)
	{
		switch(inchar = fgetc(pcb.scr_file))
		{
			case EOF:
				return;
			case '\n':
				++pcb.cn->line;
				break;
			default:
				if(!isspace(inchar))
				{
					ungetc(inchar,pcb.scr_file);
					return;
				}
				break;
		}	
	}
}
static void strip_to_newline()
/* Reads and dumps all chars until a newline.  Puts newline back 
 * used to strip comments */
{
int inchar;

	while((inchar = fgetc(pcb.scr_file)) != '\n')
	{
		if(inchar == EOF)
			return;
	}
	ungetc('\n',pcb.scr_file);
}
int get_token()
/* get a token up to 128 (127\0) chars in length from current script file 
 * it will stop at a '(' or ',' or any whitespace char returns size of token 
 * or Err_eof */
{
int inchar;
int lastchar;
char *outchar;
char *maxchar;

	pcb.toklen = pcb.reuse_tok;
	pcb.reuse_tok = 0;
	if(pcb.toklen)
		return(pcb.toklen);

start_over: /* this is where you get sent after comment stripping */

	skip_leading_white();
	outchar = pcb.token;
	maxchar = outchar + (sizeof(pcb.token - 1));
	lastchar = 0;

	for(;;)
	{	
		if(outchar >= maxchar)
			goto done;

		inchar = fgetc(pcb.scr_file);	
		*outchar = inchar;
		switch(inchar)
		{
			case EOF:
				*outchar = 0; 
				if(outchar == &pcb.token[0])
					return(Err_eof);
				goto kill_last;
			case '(':
			case ')':
			case ',':
				if(outchar == &pcb.token[0])
					goto done;
			case '\n':
				ungetc(inchar,pcb.scr_file); /* put back for line counter */
				goto kill_last;
			case '/':
				if(lastchar == '/') /* start of comment '//' */
				{
					strip_to_newline();

					/* if the first char of this token is the comment start we
					 * start fresh */

					if(--outchar == pcb.token) 
						goto start_over; /* this is back to the top above */
					goto kill_last;
				}
				break;
			default:
				if(isspace(inchar))
					goto kill_last;
				break;
		}
		lastchar = inchar;
		++outchar;
	}

done:
	++outchar;
kill_last:
	*outchar = 0;
	pcb.toklen = (outchar - &pcb.token[0]);
	return(Success);
}
void reuse_token()
{
	pcb.reuse_tok = pcb.toklen;
}
Errcode scan_number(char type, void *pnum)
/* gets a number from current token. Types are scanf types 
 * ie: 'u' = unsigned, 'f' = float, 'd' = signed decimal, 'x' = hex etc */
{
static char typefmt[] = "%?%*c";

	typefmt[1] = type;
	if(sscanf(pcb.token,typefmt,pnum) != 1)
		return(Err_not_found);
	return(Success);
}
Errcode get_number(char type, void *pnum)
/* will get a token and scan it, will not do a reuse token if an error */
{
Errcode err;

	if((err = get_token()) < Success)
		return(err);
	if(scan_number(type,pnum) < Success)
		return(Err_syntax);
}
Errcode getuint(int *pint)
/* gets an unsigned integer from current script file */
{
	return(get_number('u',pint));
}
