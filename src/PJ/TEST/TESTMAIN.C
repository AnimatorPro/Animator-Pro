#include "lstdio.h"
#include "ffile.h"
#include "filepath.h"
#include "textedit.h"
#include "memory.h"

SHORT program_version, program_id;

main(int argc, char **argv)
{
Errcode err;

/*	if((err = init_pj_startup(NULL,NULL,argc,argv,
						      NULL,"aa.mu")) < Success)
	{
		exit(0);
	} */
	test(argc, argv);
/* 	cleanup_startup(); */
}
