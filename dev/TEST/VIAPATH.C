#include <stdlib.h>
#include <stdio.h>
#include "stdtypes.h"
#include "filepath.h"


/* this little program will take the input list of names and output a list
 * of files found by searching the dos path method to find the files it will
 * not extend a suffix and the suffix must be provided */

static FILE *infile = NULL;
static int arg_c;
static char **argvs;

static char *get_arg()
{
char *out;
static char nbuf[PATH_SIZE];

	for(;;)
	{
		if(infile == NULL)
		{
			if(--arg_c <= 0)
				return(NULL);
			++argvs;
			out = *argvs;
			if(*out != '@')
				return(out);
			++out;
			if((infile = fopen(out,"r")) == NULL)
			{
				printf("\nError: can't open \"%s\" for input\n", out);
				exit(1);
			}
		}
		if(1 != fscanf(infile,"%79s", nbuf))
		{
			fclose(infile);
			infile = NULL;
			continue;
		}
		return(nbuf);
	}
}

main(int argc, char **argv)
{
char ckpath[PATH_SIZE];
char *dpath;
char *paths;
char *arg;
char *suffix;

	if(argc < 2)
	{
		printf(
		"viapath name1 [name2] [name3] @file_with_names_in_it [name4] ...\n\n"
		"Prints paths of where all names are found via the dos path\n"
		"environment variable. If no suffix is provided then the search\n"
		"is done in .COM .EXE .BAT and (no suffix) order.\n" );
		exit(1);
	}



	argvs = argv;
	arg_c = argc;

	if((dpath = getenv("PATH")) == NULL)
	{
		printf("no path found!!!");
		exit(1);
	}

	while((arg = get_arg()) != NULL)
	{
		ckpath[0] = 0;
		paths = dpath;

		strcpy(ckpath,arg);

		for(;;)
		{
			suffix = pj_get_path_suffix(ckpath);
			if(*suffix == 0)
			{
				strcpy(suffix,".COM");
				if(pj_exists(ckpath))
					break;
				strcpy(suffix,".EXE");
				if(pj_exists(ckpath))
					break;
				strcpy(suffix,".BAT");
				if(pj_exists(ckpath))
					break;
				*suffix = 0;
			}
			if(pj_exists(ckpath))
				break;

			if(paths == dpath)
			{
				if(pj_get_path_name(arg) != arg)
					goto cant_find_it;
			}
			if(parse_to_semi(&paths, ckpath,sizeof(ckpath)) == 0)
				goto cant_find_it;

			add_subpath(ckpath, arg, ckpath);
		}
		printf("%s\n", ckpath );
		continue;

	cant_find_it:
		printf("Error: can't find \"%s\"", arg );
		continue;
	}
}
