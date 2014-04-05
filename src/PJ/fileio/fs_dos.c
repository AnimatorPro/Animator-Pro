/* fs_dos.c */

#include <assert.h>
#include "errcodes.h"
#include "jfile.h"
#include "filepath.h"
#include "wildlist.h"

typedef struct wild_data {
	Names **plist; /* pointer to name list start */
	char prefix[4];
	int min_name_size;
	Nameload load_name;
	struct fndata fn;
} Wild_data;

/*--------------------------------------------------------------*/
/* Wild list.                                                   */
/*--------------------------------------------------------------*/

static Errcode
add_wild(Wild_data *wd)
{
	Wild_entry *next;
	char buf[16];
	int c2;
	int len;

	/* Filter out '.' and '..' */
	if (wd->fn.name[0] == '.') {
		c2 = wd->fn.name[1];
		if (c2 == '.' || c2 == 0)
			return Success;
	}
	len = sizeof(Wild_entry) + sprintf(buf, "%s%s", wd->prefix, wd->fn.name);
	if ((next = pj_malloc(Max(len, wd->min_name_size))) == NULL)
		return Err_no_memory;
	next->hdr.name = next->name_buf;
	(*(wd->load_name))(next, buf);
	next->hdr.next = *(wd->plist);
	*(wd->plist) = &(next->hdr);

	return Success;
}

/* Function: attr_wild_list
 *
 *  Will return Success if nothing is found.
 */
static Errcode
attr_wild_list(int attr, const char *pat, Wild_data *wd)
{
	Errcode err;

	/* set the 'DTA' area for directory search */
	pj_dset_dta(&wd->fn);

	/* now do the find first... */
	if (pj_dfirst(pat, attr)) {
		for (;;) {
			if ((wd->fn.attribute&16) == attr)
				if ((err = add_wild(wd)) < Success)
					return err;
			if (!pj_dnext())
				break;
		}
	}

	return Success;
}

/* Function: alloc_wild_list
 *
 *  Allocate list of files from current directory insuring that each
 *  buffer is the size requested and copying in name with input
 *  function.  Does not sort list.
 */
static Errcode
alloc_wild_list(Names **pwild_list, char *pat, Boolean get_dirs,
		int min_name_size, Nameload load_name)
{
	Errcode err;
	Wild_data wd;

	*pwild_list = NULL;
	wd.plist = pwild_list;
	wd.min_name_size = min_name_size;
	wd.load_name = load_name;

	if (pat[0] == '#' && pat[1] == ':') {
#ifdef SLUFFED
		boxf("Oooops this could crash! see \\paa\\fileio\\jdevlist.c");
#endif /* SLUFFED */
#ifdef WONT_LINK
		rget_dir(pwild_list);
#endif
	}
	else {
		/* get all directories */
		if (get_dirs) {
			wd.prefix[0] = DIR_DELIM;
			wd.prefix[1] = 0;
			if ((err = attr_wild_list(16, "*.*",&wd)) < Success)
				goto error;
		}
		/* and other files matching wild */
		wd.prefix[0] = 0;
		if ((err = attr_wild_list(0,pat,&wd)) < Success)
			goto error;
	}
	return Success;
error:
	free_wild_list(pwild_list);
	return err;
}

static void
load_wild_name(Wild_entry *entry, const char *name)
{
	entry->hdr.name = entry->name_buf;
	strcpy(entry->name_buf, name);
}

Errcode
build_wild_list(Names **pwild_list, const char *pat, Boolean get_dirs)
{
	Errcode err;

	err = alloc_wild_list(pwild_list, pat, get_dirs, 0, load_wild_name);
	if (err >= Success) {
		*pwild_list = sort_names(*pwild_list);
	}

	return err;
}
