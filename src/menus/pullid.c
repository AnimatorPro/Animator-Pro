/* pullid.c - routines to find a pull given its ID.  Also routines for
 * enabling and disabling pulls from their ID's */

#include "errcodes.h"
#include "jimk.h"
#include "menus.h"

/* find a pull with the appropriate id */
Pull *id_to_pull(Menuhdr *mh, SHORT id)
{
	Pull *p, *ip;

	p = mh->mbs;
	while (p != NULL) {
		if (p->id == id) {
			return p;
		}
		ip = p->children->children;
		while (ip != NULL) {
			if (ip->id == id) {
				return ip;
			}
			ip = ip->next;
		}
		p = p->next;
	}

	/* shouldn't happen unless resource file is bad... */
	errline(Err_not_found, "id_to_pull(%d)\n", id);
	return NULL;
}

void set_pul_disable(Menuhdr *mh, SHORT id, bool disable)
/* Disable/enable Pull item depending on disable */
{
	Pull *p = id_to_pull(mh, id);

	if (disable) {
		p->flags |= PULL_DISABLED;
	} else {
		p->flags &= ~(PULL_DISABLED);
	}
}

/* Disable/enable Pulls depending on disable */
void set_pultab_disable(Menuhdr *mh, SHORT *ids, int id_count, bool disable)
{
	while (--id_count >= 0) {
		set_pul_disable(mh, *ids++, disable);
	}
}

/* Disable/enable entire leaf of a pulldown */
void set_leaf_disable(Menuhdr *mh, SHORT leafid, bool disable)
{
	Pull *p = id_to_pull(mh, leafid)->children->children;

	while (p != NULL) {
		if (disable) {
			p->flags |= PULL_DISABLED;
		} else {
			p->flags &= ~(PULL_DISABLED);
		}
		p = p->next;
	}
}

/* Put an asterisk or a space in the text area of Pull depending on xflag.
 * Xflag TRUE for asterisk. */
void pul_xflag(Menuhdr *mh, SHORT id, bool xflag)
{
	Pull *p = id_to_pull(mh, id);
	char c = (xflag ? '*' : ' ');

	((char *)(p->data))[0] = c;
}

/* Wipe out any asterisks in the Pulls */
void pultab_xoff(Menuhdr *mh, SHORT *ids, int id_count)
{
	while (--id_count >= 0) {
		pul_xflag(mh, *ids++, false);
	}
}
