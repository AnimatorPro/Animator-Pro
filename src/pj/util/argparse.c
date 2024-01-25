#include "argparse.h"
#include "linklist.h"

Errcode parse_args(Argparse_list *switches, Do_aparse do_others,
		   		   int argc, char **argv)
{
Do_aparse doit;
Argparse_list *apl;
int position;
Errcode ret;

	++argv;
	position = 1;
	argc -= 1;
	while(argc > 0)
	{
		if((apl = (Argparse_list *)text_in_list(*argv,
								   (Names *)switches)) != NULL)
		{
			doit = apl->doit;
		}
		else
			doit = do_others;

		if(doit != NULL)
		{
			/* note the (void*) this is needed because of some compiler bug */
			if((ret = (*doit)((void *)apl, argc, argv, position)) < 0)
				return(ret);
			++ret;
		}
		else
			ret = 1;

		argv += ret;
		position += ret;
		argc -= ret;
	}
	return(Success);
}
