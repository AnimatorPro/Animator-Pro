#include <string.h>
#include "jimk.h"
#include "resource.h"
#include "scroller.h"
#include "wildlist.h"

bool req_resource_name(char *result, char *pat, char *hailing)

/* assumes a name to pre load in the requestor is input */
{
char opath[PATH_SIZE];
char justname[16];
bool ok = false;
Names *l;
static SHORT ipos = 0;
Errcode err;

	strcpy(justname,pj_get_path_name(result));
	err = build_wild_list(&l, resource_dir, pat, false);
	if (err >= Success)
	{
		if(l != NULL)
		{
			if(qscroller(justname, hailing, l, 10, &ipos))
			{
				ok = true;
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
