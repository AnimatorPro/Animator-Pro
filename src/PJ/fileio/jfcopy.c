#include "errcodes.h"
#include "jimk.h"
#include "jfile.h"

Errcode pj_copyfile(char *source, char *dest)
/* Make a copy of a (closed) file. Report errors except for
   source file not existing. */
{
Errcode err;
char *errfile;

	err = pj_cpfile(source,dest,&errfile);
	if (err != Success && err != Err_no_file)
		err = errline(err, "%s", errfile);
	return(err);
}
