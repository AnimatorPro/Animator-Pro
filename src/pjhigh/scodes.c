#include <string.h>
#include "stdtypes.h"
#include "errcodes.h"
#include "progids.h"

static char secret_code[] = " ";

/* loads buffer with string representing compound version number */
void get_pj_version(char *buf)
{
	strcpy(buf,"1.4");
}


void get_userid_string(char *buf)
{
	strcpy(buf, secret_code);
}
