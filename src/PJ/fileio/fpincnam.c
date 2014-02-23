#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "filepath.h"

int pj_inc_filename(char *path)
{
char *pt;
char *name;
char suffix[6];
char numbuf[10];
int numend, digilen;
char c;
long number;

	name = pj_get_path_name(path);
	pt = fix_suffix(name);
	strcpy(suffix,pt);
	*pt = 0;
	digilen = 0;

	while(pt >= name)
	{
		c = pt[-1];
		if(!isdigit(c))
			break;
		--pt;
		++digilen;
	}

	numend = 8 - (pt - name); /* what is left for number */
	if(digilen < 2)
		digilen = 2;
	number = atol(pt)+1;
	if(number > 9999999)  /* just for safety */
		number = 9999999;

	numend -= sprintf(numbuf,"%0*ld",digilen, number);
	if(numend < 0)
		pt += numend;

	sprintf(pt,"%s%s", numbuf, suffix );
	return(number);
}
