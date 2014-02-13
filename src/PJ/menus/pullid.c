/* pullid.c - routines to find a pull given it's ID.  Also routines for
 * enabling and disabling pulls from their ID's */

#include "menus.h"
#include "errcodes.h"

Pull *id_to_pull(Menuhdr *mh, SHORT id)
/* find a pull with the appropriate id */
{
Pull *p, *ip;

p = mh->mbs;
while (p != NULL)
	{
	if (p->id == id)
		return(p);
	ip = p->children->children;
	while (ip != NULL)
		{
		if (ip->id == id)
			return(ip);
		ip = ip->next;
		}
	p = p->next;
	}
/* shouldn't happen unless resource file is bad... */
errline(Err_not_found, "id_to_pull(%d)\n", id);
return(NULL);
}

void set_pul_disable(Menuhdr *mh, SHORT id, Boolean disable)
/* Disable/enable Pull item depending on disable */
{
Pull *p = id_to_pull(mh,id);

if(disable)
	p->flags |= PULL_DISABLED;
else
	p->flags &= ~(PULL_DISABLED);
}

void set_pultab_disable(Menuhdr *mh, SHORT *ids, int id_count, Boolean disable)
/* Disable/enable Pulls depending on disable */
{
while (--id_count >= 0)
	set_pul_disable(mh, *ids++, disable);
}

void set_leaf_disable(Menuhdr *mh, SHORT leafid, Boolean disable)
/* Disable/enable entire leaf of a pulldown */
{
Pull *p = id_to_pull(mh, leafid)->children->children;

while (p != NULL)
	{
	if(disable)
		p->flags |= PULL_DISABLED;
	else
		p->flags &= ~(PULL_DISABLED);
	p = p->next;
	}
}

void pul_xflag(Menuhdr *mh, SHORT id, Boolean xflag)
/* Put an asterisk or a space in the text area of Pull depending on xflag.
 * Xflag TRUE for asterisk. */
{
Pull *p = id_to_pull(mh,id);
char c = (xflag ? '*' : ' ');

((char *)(p->data))[0] = c;
}

void pultab_xoff(Menuhdr *mh, SHORT *ids, int id_count)
/* Wipe out any asterisks in the Pulls */
{
while (--id_count >= 0)
	pul_xflag(mh, *ids++, FALSE);
}
