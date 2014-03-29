#include <string.h>
#include "errcodes.h"
#include "reqlib.h"
#include "vsetfile.h"

char *vset_get_filename(char *prompt, char *suffi, char *button, 
				       int path_type, char *outpath, Boolean force_suffix)
/* Put up file requestor installing path from config path type. */
{
char *retp;
static Vset_path cpath;

	vset_get_pathinfo(path_type,&cpath);
	retp = pj_get_filename(prompt,suffi,button,
								cpath.path,cpath.path,force_suffix, 
								&cpath.scroller_top,cpath.wildcard);
	vset_set_pathinfo(path_type,&cpath);
	if (outpath != NULL)
		strcpy(outpath, cpath.path);
	return(retp);
}
