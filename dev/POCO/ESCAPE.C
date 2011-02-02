
#error You shouldn't try to compile this module, see comments in the file.

NOTE: this routine is no longer part of poco or pj.  its functionality is
	  now built into the tokenize_word routine in TOKEN.C.

THIS ROUTINE IS BROKEN -- IT DOESN'T HANDLE OCTAL OR HEX CONSTANTS RIGHT!


/*****************************************************************************
 *
 * escape.c	- Routine to convert C escapes in a string to actual values.
 *
 ****************************************************************************/

int translate_escapes(register unsigned char *inbuf,
					  unsigned char *outbuf)
/*****************************************************************************
 * translate all escape codes in the input string to the output string.
 * the input and output buffers can be the same.
 ****************************************************************************/
{
unsigned char *routbuf = outbuf;
register unsigned char inchar;
int scape_count = 0;
int inhex = 0;
unsigned char binary_val;


	for(;;)
	{
		inchar = *inbuf++;

		if(!scape_count)
		{
			switch(inchar)
			{
				case 0:
					goto done;
				case '\\':
					scape_count = 1;
					break;
				default:
					*routbuf++ = inchar;
			}
			continue;
		}
		else
		{
			if(scape_count == 1)
			{
				switch(inchar)
				{
					case 0:
						goto bad_escape;
#if 0
					case '0':
						inchar = '\0';
						goto load_inchar;
#endif
					case 'a':
						inchar = '\a';
						goto load_inchar;
					case 'b':
						inchar = '\b';
						goto load_inchar;
					case 'f':
						inchar = '\f';
						goto load_inchar;
					case 'n':
						inchar = '\n';
						goto load_inchar;
					case 'r':
						inchar = '\r';
						goto load_inchar;
					case 't':
						inchar = '\t';
						goto load_inchar;
					case 'v':
						inchar = '\v';
						goto load_inchar;
					case '\\':
						inchar = '\\';
						goto load_inchar;
					case '?':
						inchar = '\?';
						goto load_inchar;
					case '\'':
						inchar = '\'';
						goto load_inchar;
					case '\"':
						inchar = '\"';
						goto load_inchar;
					case '\x':
						inhex = 1;
						break;
					default:
					{
						if(inchar < '0' || inchar > '7')
							goto load_inchar;

						/* it's an octal digit */

						inchar -= '0';
						inchar <<= 6;
						binary_val = inchar;
						inhex = 0;
					}
				}
				++scape_count;
				continue;
			}

			++scape_count;    /* counts for chars will be /2345 */

			if(inhex)
			{

				if(inchar < '0')
					goto bad_escape;
				else if(inchar <= '9')
					inchar -= '0';
				else if(inchar < 'A')
					goto bad_escape;
				else if(inchar <= 'F')
					inchar -= ('A' - 10);
				else if(inchar < 'a')
					goto bad_escape;
				else if(inchar <= 'f')
					inchar -= ('a' - 10);
				else
					goto bad_escape;


				if(scape_count == 3) /* "/x34" */
				{
					binary_val = inchar <<= 4;
					continue;
				}
				else /* last of two hex digits */
				{
					inchar += binary_val;
					goto load_inchar;
				}
			}
			else /* octal */
			{
				if(inchar < '0' || inchar > '7' )
					goto bad_escape;

				inchar -= '0';

				if(scape_count == 3) /* "/d34" */
				{
					inchar <<= 3;
					binary_val += inchar;
					continue;
				}
				inchar += binary_val;
				goto load_inchar;
			}

bad_escape:
			while(scape_count > 0)
			{
				*routbuf++ = *(inbuf - scape_count);
				--scape_count;
			}

			if(inchar == 0)
			{
				--routbuf; /* for length count */
				goto done;
			}
			continue;

load_inchar:
			*routbuf++ = inchar;
			scape_count = 0;
		}
	}
done:
	*routbuf = 0;
	return(routbuf - outbuf);
}

