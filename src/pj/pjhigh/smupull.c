#include "jimk.h"
#include "softmenu.h"

void scale_pull( Menuhdr *mh, int subspace)
/* calculate coordinates for pull-down */
{
Rectangle crect;

crect.width = 6;
crect.height = 8;
scale_rect(&vb.screen->menu_scale, &crect, &crect);
pullfmt(mh,subspace,crect.width,crect.height,
	(Rectangle *)(&vb.screen->wndo.RECTSTART));
}

Errcode load_soft_pull(
	Menuhdr *mh, 	/* where to load pull */
	int subspace,	/* # of spaces to indent first leaf */
	char *name, 	/* symbolic name of pulldown */
	int muid,		/* Peter's menu id (???) */
	void (*selit)(Menuhdr *mh, SHORT menuhit),/* selection function */
	int (*enableit)(Menuhdr *mh)) /* Function to grey out items and do
								   * asterisk setting etc.  Called when 
								   * cursor goes into menu bar.
								   * May be NULL.  If present function should 
								   * finish by returning menu_dopull(mh) */
/* Load pulldown from default soft-menu file.  Scale it to screen size. 
 * Attatch it to main screen. */
{
Errcode err;

/* load in the pull */
if ((err = smu_load_pull(&smu_sm,name,mh)) < Success)
	goto OUT;

/* scale it to current screen size */
scale_pull(mh, subspace);

/* install the 'feeler' */
mh->flags |= MENU_NORESCALE;
mh->dodata = selit;
mh->wndoid = muid;
if (enableit == NULL)
	mh->domenu = menu_dopull;
else
	mh->domenu = enableit;
OUT:
return(softerr(err, "!%s", "pull_load",
	name));
}


