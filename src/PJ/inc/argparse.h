#ifndef ARGPARSE_H
#define ARGPARSE_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

/* to return number of extra arguments consumed or 0 if only the current 
 * one is consumed. < 0 will cause parser to abort and return the value */

struct argparse_list;

typedef int (*Do_aparse)(struct argparse_list *ap, /* what's calling this */
					int argc,  /* count left including current argument */
				 	char **argv, /* positioned at current argument */
				 	int position); /* position in original: argv[position] */

typedef struct argparse_list {
	void *next;
	char *text;
	Do_aparse doit;
} Argparse_list;

#define APLAST -1
#define ARGP(lst,idx,txt,doit) \
	{ idx==APLAST?NULL:(void *)(&(lst[idx])+1),txt,doit }

Errcode parse_args(Argparse_list *switches, 
		   		   Do_aparse do_others,
				   int argc, char **argv);

#endif
