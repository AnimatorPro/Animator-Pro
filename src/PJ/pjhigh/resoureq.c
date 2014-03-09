#include <string.h>
#include "jimk.h"
#include "resource.h"
#include "scroller.h"
#include "wildlist.h"

Boolean req_resource_name(char *result, char *pat, char *hailing)

/* assumes a name to pre load in the requestor is input */
{
char opath[PATH_SIZE];
char justname[16];
Boolean ok = FALSE;
Names *l;
static SHORT ipos = 0;
Errcode err;

	get_dir(opath);
	strcpy(justname,pj_get_path_name(result));
	if ((err = change_dir(resource_dir)) >= Success)
	{
		build_wild_list(&l, pat, FALSE);
		change_dir(opath);

		if(l != NULL)
		{
			if(qscroller(justname, hailing, l, 10, &ipos))
			{
				ok = TRUE;
				strcpy(result, justname);
			}
			free_wild_list(&l);
		}
		else
			cant_find(make_resource_name(pat, opath));
	}
	else
		no_resource(err);
	return(ok);
}
