#include "softmenu.h"

int soft_name_string(char *symname,	/* name of symbol in resource file */
					char *strname,	/* name of string in symbol */
					char *strbuf,	/* place to put string */
					int bufsize)	/* length of place to put string */
{
	return(smu_name_string(&smu_sm,symname,strname,strbuf,bufsize));
}
