#include <string.h>
#include "rexlib.h"

char *rex_key_or_text(char *key_or_text, char **name)
/* returns key or NULL, puts pointer to name in *name */
{
	if(key_or_text == NULL) /* this handles NULLs */
	{
		*name = "";
		return(NULL);
	}
	if(RL_ISKEYTEXT(key_or_text))
	{
		*name = key_or_text + strlen(key_or_text)+1;
		return(key_or_text+1);
	}
	else
	{
		*name = key_or_text;
		return(NULL);
	}
}
