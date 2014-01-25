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

struct argparse_list {
	void *next;
	char *text;
	Do_aparse doit;
};

/* Note this is NOT a typedef */
#define Argparse_list struct argparse_list 

#define APLAST -1
#define ARGP(lst,idx,txt,doit) \
	{ idx==APLAST?NULL:(void *)(&(lst[idx])+1),txt,doit }

Errcode parse_args(Argparse_list *switches, 
		   		   Do_aparse do_others,
				   int argc, char **argv);

#ifdef BIG_COMMENT /**********************************************/

initializing an argparse list:

static Errcode do_a1()
{
	printf("got argument -a1\n");
	return(0);
}
static Errcode do_help()
{
	printf("help_text");
	return(-1);
}

Argparse_list apl = {
	ARGP(apl,0,"-a1",do_a1),
	ARGP(apl,1,"-a2",do_a2),
	ARGP(apl,APLAST,"-?",do_help),
};

If you want to include more data in the argument item make it bigger. as in.

struct long_aparse {
	Argparse_list apl;
	void *data;
};

struct long_argparse apl = {
	{ARGP(apl,0,"-a1",do_a1),&data}
	{ARGP(apl,1,"-a2",do_a2),&data}
	{ARGP(apl,APLAST,"-?",do_help),&data}
};

/***** using it *****/

main(int argc, char **argv)
{
	ret = parse_args(apl,do_help,argc,argv);
}

#endif /* BIG_COMMENT *********************************************/


#endif /* ARGPARSE_H */
