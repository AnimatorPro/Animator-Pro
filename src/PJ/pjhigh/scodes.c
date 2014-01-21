#include <string.h>
#include "stdtypes.h"
#include "errcodes.h"
#include "progids.h"

static char secret_code[] = " ";

Errcode init_scodes()
/* this should do some security verify ??? */
{
	return(Success);
}
void get_relvers(char *buf)
/* loads buffer with string representing compound version number */
{
	strcpy(buf,"1.3");
}
void get_userid_string(char *buf)
{
	strcpy(buf, secret_code);
}
