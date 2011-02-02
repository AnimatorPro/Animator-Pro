/* leading comment */

#include <stdio.h>
#include <ctype.h>

struct bigcb {
	char *sfilename;  /* source file name */
	FILE *sfile;      /* source file */
	char *ofilename;  /* output file name */
	FILE *ofile;      /* output file */ 
	int isasm;
	char buff[1024];
};

static struct bigcb bcb;

/******************************************/
main(argv,argc)

char **argv;
int argc;
{
	if(getparms(argv,argc))
		exit(-1);

	if(NULL == (bcb.sfile = fopen(bcb.sfilename, "r" )))
	{
		printf("unable to open \"%s\" for input\n", bcb.sfilename );
		goto error;
	}

	if(NULL == (bcb.ofile = fopen(bcb.ofilename,"a" )))
	{
		printf("unable to open \"%s\" for output\n", bcb.ofilename  );
		goto error;
	}

	fprintf(bcb.ofile,
	        "\n---------************* %s *************-------\n\n",
			bcb.sfilename );

	for(;;)
	{
		if(NULL == fgets(bcb.buff,sizeof(bcb.buff),bcb.sfile))
			break;

		if(bcb.isasm)
		{

			if( bcb.buff[0] != ';'
				&& bcb.buff[0] > 0
				&& !(iscntrl(bcb.buff[0]))
				&& !(isspace(bcb.buff[0])))
			{
				break;
			}

			if(!(strncmp("\tpublic", bcb.buff, 7 )))
				break;

			if(!(strncmp("\tTITLE", bcb.buff, 6 )))
				break;

			if(!(strncmp("\ttitle", bcb.buff, 6 )))
				break;
		}
		else if(!(strncmp("#include", bcb.buff, 8 )))
			break;

		fputs(bcb.buff,bcb.ofile);
	}

	closeup();
	exit(0);
error:
	closeup();
	exit(-1);
}
closeup()
{
	if(bcb.sfile)
		fclose(bcb.sfile);
	if(bcb.ofile)
		fclose(bcb.ofile);
}
int getparms(argc,argv)

char **argv;
int argc;
{
int namelen;

	if(argc != 2)
	{
		printf("no file name !!!");
		goto error;
	}

	bcb.sfilename = argv[1];

	namelen = strlen(bcb.sfilename);

	if(namelen < 3)
	{
		printf("bad source filename\n");
		goto error;
	}

	if(!strcmp(".c", &bcb.sfilename[namelen - 2]))
		bcb.isasm = 0;
	else if(!strcmp(".asm", &bcb.sfilename[namelen - 4]))
		bcb.isasm = 1;
	else
	{
		printf("must be .c or .asm file\n");
		goto error;
	}


	bcb.ofilename = "pullcoms.out";

	return(0);
error:
	return(1);
}
