#include "jimk.h"
#include "softmenu.h"

/* calculate coordinates for pull-down */
void scale_pull(Menuhdr *mh, int subspace)
{
	Rectangle crect;

	crect.width = 6;
	crect.height = 8;
	scale_rect(&vb.screen->menu_scale, &crect, &crect);
	pullfmt(mh, subspace, crect.width, crect.height, (Rectangle *)(&vb.screen->wndo.RECTSTART));
}

/* Load pulldown from default soft-menu file.  Scale it to screen size.
 * Attatch it to main screen. */
Errcode load_soft_pull(Menuhdr *mh,  /* where to load pull */
					   int subspace, /* # of spaces to indent first leaf */
					   char *name,   /* symbolic name of pulldown */
					   int muid,     /* Peter's menu id (???) */
					   void (*selit)(Menuhdr *mh, SHORT menuhit), /* selection function */
					   int (*enableit)(Menuhdr *mh)) /* Function to grey out items and do
													  * asterisk setting etc.  Called when
													  * cursor goes into menu bar.
													  * May be NULL.  If present function should
													  * finish by returning menu_dopull(mh) */
{
	/* load in the pull */
	Errcode err = smu_load_pull(&smu_sm, name, mh);

	if (err == Success) {
		/* scale it to current screen size */
		scale_pull(mh, subspace);

		/* install the 'feeler' */
		mh->flags |= MENU_NORESCALE;
		mh->dodata = selit;
		mh->wndoid = muid;
		if (enableit == NULL) {
			mh->domenu = menu_dopull;
		} else {
			mh->domenu = enableit;
		}
	}

	return softerr(err, "!%s", "pull_load", name);
}
