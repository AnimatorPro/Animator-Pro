#include "stdtypes.h"
#include "errcodes.h"
#include "progids.h"

static char secret_code[] = "$$$-$$$$$$$$";


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
	if(secret_code[0] == '$')
		sprintf(buf, "%.7s", secret_code);
	else
		strcpy(buf,secret_code);
}
LONG get_userid()
{
#define SECRET_OSET 1008
static char *psecret_code = secret_code + SECRET_OSET;
char *usernum;

	usernum = psecret_code - SECRET_OSET;
	if(usernum[0] == '$')
		return *((LONG *)"BETA");
	else
		return(atol(usernum));
}
