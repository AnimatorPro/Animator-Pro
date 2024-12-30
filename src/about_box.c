/* about_box.c - display continue alert box with program info */

#include "jimk.h"
#include "progids.h"


void about(void)
{
	static char date[] = __DATE__;
	char relnum[16];
	char idtext[32];

	get_pj_version(relnum);
	get_userid_string(idtext);
	soft_continu_box("!%s", "about", relnum);
}

