
/* debug.c - not normally linked.  Handy hex dump to file routine.
   Whoopie what a marvel! */

#include <stdio.h>

dumptofile(name, pt, count)
char *name;
unsigned char *pt;
int count;
{
FILE *f;
int i;

if ((f = fopen(name, "w")) == NULL)
	{
	cant_create(name);
	return;
	}
for (i=0; i<count; i++)
	{
	fprintf(f, "%x, ", *pt++);
	if ((i%15) == 14)
		fprintf(f, "\n");
	}
fprintf(f, "\n");
fclose(f);
}
