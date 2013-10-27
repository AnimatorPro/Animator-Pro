
#include <stdio.h>
FILE *DUMP_FILE;

dumptofile(name, pt, count)
char *name;
unsigned char *pt;
int count;
{
FILE *f;
int i;

if ((f = fopen(name, "w")) == NULL)
	{
/*	cant_create(name); */
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

OPENDUMP()
{

if ((DUMP_FILE = fopen("dump1", "w")) != NULL)
	return(1);
else	return(0);
}


DUMPINT(strng,val)
char *strng;
int val;
{
fprintf(DUMP_FILE,"%s;  %d\n",strng,val);
}

CLOSEDUMP()
{
fclose(DUMP_FILE);
}

