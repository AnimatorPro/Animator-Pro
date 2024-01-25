
#include <stdio.h>

main(argc, argv)
int argc;
char *argv[];
{
char *title;
FILE *f;
int r,g,b,ccount;
int i;

if (argc != 2)
	{
	puts("Usage - dumpc file.col");
	puts("   creates a C array initialization for file.col");
	exit(0);
	}
title = argv[1];
if ((f = fopen(title, "rb")) == NULL)
	{
	printf("Sorry, couldn't find %s\n", title);
	exit(-1);
	}
#ifdef LATER
ccount = getc(f);
if (ccount == 0)
#endif LATER
	ccount = 256;
for (i=0; i<ccount; i++)
	{
	r = getc(f);
	g = getc(f);
	b = getc(f);
	printf("%2d,%2d,%2d,\t", r,g,b);
	if ((i&3) == 3)
		puts("");
	}
}

