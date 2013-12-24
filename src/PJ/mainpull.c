
/* undone */
/* mainpull.c - The data structures for the top level drop-down menus up
   along the top.  Also a routine to disable menu options we cant' deal
   with yet. */

#include "linklist.h"
#include "jimk.h"
#include "menus.h"
#include "homepul.h"
#include "fli.h"
#include "errcodes.h"
#include "resource.h"
#include "filepath.h"


Boolean do_mainpull(Menuhdr *mh)
/* set disable flags and goes to do the pull */
{
static SHORT cel_pulltab[] = {
	CEL_REL_PUL,
	CEL_TUR_PUL,
	CEL_STR_PUL,
	CEL_MOV_PUL,
	CEL_PAS_PUL,
};
static SHORT anim_pulltab[] = {
	TRA_INS_PUL,
	TRA_LOO_PUL,
	TRA_SEG_PUL,
	TRA_FLI_PUL,
	TRA_NEX_PUL,
	TRA_REP_PUL,
};
static SHORT alt_pulltab[] = {
	SWA_REL_PUL,
	SWA_VIE_PUL,
	SWA_PAS_PUL,
	SWA_TRA_PUL,
};

	set_pultab_disable(mh,cel_pulltab,Array_els(cel_pulltab),
		(thecel == NULL));
	set_pultab_disable(mh,anim_pulltab,Array_els(anim_pulltab),
		(flix.hdr.frame_count < 2));
	set_pultab_disable(mh,alt_pulltab,Array_els(alt_pulltab),
		(vl.alt_cel == NULL));
	return(menu_dopull(mh));
}

/* some stuff to deal with poco-pull-down which is partially made
   up during run-time */


int new_pull_list(		/* Returns Errcode or # of pull selections made */
	Pull **ppull, 					/* Put resulting Pulls here */
	Names *nlist,  
	int ncount, 		/* Input names */
	int startid)
/* Make a list of pulls linked along the next thread corresponding
   to the first ncount names on nlist.   */
{
Pull *list = NULL;
Pull *new;
void *data;
int count;
Errcode err = Success;

for (count = 0; count < ncount; count++)
	{
	if (nlist == NULL)
		break;
	if ((err = new_pull(&new, nlist->name)) < Success)
		goto OUT;
	data = new->data;
	new->key2 = ((char *)(new->data))[0];
	new->next = list;
	new->id = ++startid;
	list = new;
	nlist = nlist->next;
	}
OUT:
if (err < Success)
	{
	free_wild_list((Names **)&list);
	*ppull = NULL;
	return(err);
	}
*ppull = reverse_slist(list);
return(count);
}

static void prep_poc_list(Names *list)
{
char *p;
char c;

while (list != NULL)
	{
	p = list->name;
	if ((c = *p) != 0)		/* make all but 1st letter lower case */
		{
		while ((c = *(++p)) != '.')
			*p = tolower(c);
		*p = 0;
		}
	list = list->next;
	}
}

Errcode init_poco_pull(Menuhdr *mh, SHORT prev_id, SHORT root_id)
{
Errcode err;
int count;
Names *pocs = NULL;
Pull *prev = id_to_pull(mh, prev_id);

if((err = change_dir(resource_dir)) < Success)
	{
	err = no_resource(err);
	goto OUT;
	}
build_wild_list(&pocs, "*.POC", FALSE);
prep_poc_list(pocs);
if ((count = new_pull_list(&prev->next,
		pocs, 10, prev_id)) < Success)
	{
	err = count;
	goto OUT;
	}
id_to_pull(mh, root_id)->children->height += prev->height*count;
OUT:
free_wild_list(&pocs);
change_dir(vb.init_drawer);
return(softerr(err,"poco_leaf"));
}


static void poco_pull_path(Menuhdr *mh, int id, char *buf)
{
make_resource_name(id_to_pull(mh,id)->data, buf);
strcat(buf, ".POC");
}

Errcode run_pull_poco(Menuhdr *mh, SHORT id)
{
char ppath[PATH_SIZE];

poco_pull_path(mh,id,ppath);
return(qrun_pocofile(ppath,FALSE));
}
