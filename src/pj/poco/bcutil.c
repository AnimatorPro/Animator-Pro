#include <ctype.h>
#include "token.h"

#ifndef iscsym
  #define iscsym(c) ( (c) == '_' || isalnum((c)) )
#endif


/*****************************************************************************
 * chop the next C symbol into the word buffer, up to maxlen chars.
 ****************************************************************************/
char *po_chop_csym(char *line, char *word, int maxlen, char **wordnext)
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
