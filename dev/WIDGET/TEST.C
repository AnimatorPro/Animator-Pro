#include "memory.h"
#include "widget.h"

extern Errcode to_be_bracketed(int a, int b, int c, int d, int e);

Boolean widge_ask_retry()
{
static char *keys[] = { "ask", "r", "c", NULL };

	return(soft_multi_box(keys, "widge_retry") == 1);
}

#include "test.h"

SHORT program_version, program_id;

main(int argc, char **argv)
{
Errcode err;
int elapsed;

	if((err = init_pj_startup(NULL,NULL,1,argv,
						      NULL,"aa.mu")) < Success)
	{
		exit(0);
	}

	init_widget(NULL);

	printf("\n\n");

	while(argc >= 2)
	{
		++argv;
		--argc;
		printf( "{ \"%s\", 0x%08x, },\n", *argv, widge_query(*argv));
	}

	boxf("get_ready");

	printf("\n");

	if((err = to_be_bracketed(1,2,3,4,5)) < Success)
		goto error;
	if((err = to_be_bracketed(6,7,8,9,0)) < Success)
		goto error;
	if((err = to_be_bracketed(9,8,7,6,5)) < Success)
		goto error;

	printf("\n");

error:
	boxf("ret = %d", err );
	errline(err,NULL);

	cleanup_widget();
	cleanup_startup();
}

