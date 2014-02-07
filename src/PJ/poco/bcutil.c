#include "poco.h"
#include <stddef.h>
#include <ctype.h>
#include <string.h>

#ifndef iscsym
  #define iscsym(c) ( (c) == '_' || isalnum((c)) )
#endif

#ifdef SLUFFED
char *po_skip_space(char *line)
/*****************************************************************************
 * return pointer to next non-white, or NULL if no more non-white on line.
 ****************************************************************************/
{
	if (line == NULL)	/* can this happen?  asm codes checks for it. */
		return NULL;

	while (isspace(*line))
		++line;

	return (*line) ? line : NULL;

}
#endif /* SLUFFED */

#ifdef SLUFFED
int po_hashfunc(unsigned char *s)
/*****************************************************************************
 * typical hashing function.
 ****************************************************************************/
{

int acc;
int c;

acc = *s++;
while ((c = *s++) != 0)
	acc = (acc /* <<1 */ )+c;
return(acc&(HASH_SIZE-1));

}
#endif /* SLUFFED */

char *po_chop_csym(char *line, char *word, int maxlen, char **wordnext)
/*****************************************************************************
 * chop the next C symbol into the word buffer, up to maxlen chars.
 ****************************************************************************/
{
	while (--maxlen && iscsym(*line)) {
		*word++ = *line++;
	}
	*wordnext = word;

	while (iscsym(*line)) {
		++line;
	}
	return line;
}
