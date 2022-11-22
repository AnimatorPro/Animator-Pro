/* menu to select a pdr from the resource directory */

#include <stdio.h>
#include <string.h>
#include "jimk.h"
#include "errcodes.h"
#include "ftextf.h"
#include "jfile.h"
#include "memory.h"
#include "menus.h"
#include "picdrive.h"
#include "pjbasics.h"
#include "resource.h"
#include "ftextf.h"
#include "rexlib.h"
#include "scroller.h"
#include "softmenu.h"
#include "util.h"
#include "wildlist.h"

typedef struct pdr_entry {
	Names hdr;
	char *pdr_name;
	char name_buf[50];
	char suffi[PDR_SUFFI_SIZE];
	UBYTE does_read;
	UBYTE does_write;
} Pdr_entry;

static Errcode build_pdr_list(Names *wildlist, Pdr_entry **pdrlist,
							  Pdr_entry **current,
							  char *curname,
							  int rwmode, /* 0 = any,
										   * 1 = must read,
										   * 2 = must write
										   * 3 = must both */

							  Boolean multiframe_only)

/* while in resource directory builds list of pdr info name strings for use
 * in the driver menu */
{
Errcode err = Err_nogood;
char *pdr_name;
Pdr_entry *entry;
Pdr_entry **pentry;
Pdr *pd = NULL;
char rw_char;
char *tit_start;

	pentry = pdrlist;
	*current = NULL;

	for(;;)
	{
		for(;;)
		{
			if(wildlist == NULL)
				goto done;

			pdr_name = wildlist->name;
			wildlist = (Names *)(wildlist->next);

			free_pdr(&pd);
			if((err = load_pdr(pdr_name, &pd)) >= Success)
			{
				if(multiframe_only
				   && pd->max_read_frames <= 1
				   && pd->max_write_frames <= 1)
				{
					continue;
				}

				if(   ((rwmode & 1) && pd->max_read_frames	< 1)
				   || ((rwmode & 2) && pd->max_write_frames < 1) )
				{
					continue;
				}
				break;
			}
			cant_query_driver(err,pdr_name);
		}

		if((*pentry = pj_malloc(sizeof(Pdr_entry))) == NULL)
		{
			err = Err_no_memory;
			goto error;
		}
		entry = *pentry;

		pdr_get_suffi(pd, entry->suffi);
		entry->does_read = pd->max_read_frames != 0;
		entry->does_write = pd->max_write_frames != 0;
		entry->pdr_name = pdr_name;
		entry->hdr.name = entry->name_buf;

#define PDE_TITLE_OSET 11

		if(!entry->does_write)
			rw_char = 'R'; /* read only */
		else if(!entry->does_read)
			rw_char = 'W'; /* write only */
		else
			rw_char = '-'; /* does both */

		tit_start = entry->name_buf +
						sprintf(entry->name_buf,"%c %-9.*s",  rw_char,
								 path_prefix_len(pdr_name), pdr_name );

		pdr_get_title(pd, tit_start, 35);

		/* if we found the current one set current pointer */

		if(!txtcmp(pdr_name,curname))
			*current = entry;

		entry->hdr.next = NULL;
		pentry = (Pdr_entry **)&(entry->hdr.next);
	}
error:
done:
	free_pdr(&pd);
	return(err);
}
static Boolean pdr_info_box(Names *entry,void *dat)
{
Errcode err;
Pdr_entry *pdentry = (Pdr_entry *)entry;
Pdr *pd = NULL;
char pdr_path[PATH_SIZE];
char read_text[34];
char write_text[34];
char title_text[35];
char *info_text;
char *soft_text;
LONG read_frames, write_frames;

void *sctbuf = NULL;

static Smu_name_scats scts[] = {

#define write_yes scts[0].toload.s
	{ "write_yes" },
#define write_no scts[1].toload.s
	{ "write_no" },
#define read_yes scts[2].toload.s
	{ "read_yes" },
#define read_no scts[3].toload.s
	{ "read_no" },
#define box_text scts[4].toload.s
	{ "box_text" },

};
(void)dat;

	if(!pdentry)
		return FALSE;

	hide_mp();

	make_resource_name(pdentry->pdr_name,pdr_path);
	if((err = load_pdr(pdr_path, &pd)) < Success)
	{
		cant_use_module(err,pdr_path);
		goto error;
	}
	if((err = soft_name_scatters("pdr_info", scts, Array_els(scts),
								  &sctbuf, SCT_DIRECT)) < Success)
	{
		goto error;
	}

	read_frames = pd->max_read_frames;
	write_frames = pd->max_write_frames;
	info_text = pdr_alloc_info(pd);
	pdr_get_title(pd,title_text,sizeof(title_text));

	snftextf(read_text, sizeof(read_text), "!%d",
			 read_frames?read_yes:read_no, read_frames);

	snftextf(write_text, sizeof(write_text), "!%d",
			write_frames?write_yes:write_no, write_frames);

	/* note this will handle a NULL pointer for info_text */

	if( ((soft_text = rex_key_or_text(info_text,&info_text)) != NULL)
		&& (smu_load_name_text(&smu_sm,"picdrive_texts",
							   soft_text, &soft_text) >= Success))
	{
		info_text = soft_text;
	}

	continu_box("!%s%s%s%s%s", box_text,
				pj_get_path_name(pdr_path),
				title_text,
				read_text,
				write_text,
				info_text );

	smu_free_text(&soft_text);
	pdr_free_info(info_text);
	free_pdr(&pd);
error:

	smu_free_scatters(&sctbuf);
	show_mp();
	return FALSE;

#undef write_yes
#undef write_no
#undef read_yes
#undef read_no
#undef box_text
}

struct pick_dat {
	char *name;
	char *suffi;
};

static Errcode pick_pdr(Names *entry,void *dat)
{
struct pick_dat *pdat = dat;
Pdr_entry *pdentry = (Pdr_entry *)entry;

	strcpy(pdat->name,pdentry->pdr_name);
	if(pdat->suffi)
		strcpy(pdat->suffi,pdentry->suffi);
	return(Success);
}
Errcode go_pdr_menu( char *header,	   /* header text for menu */

					 char *name_buf,   /* name of pdr selected, pre loaded with
										* name of current entry */

					 char *suffi_buf,  /* suffi from pdr selected may be null
										* output only */

					 Names *local_names, /* local pdr names to add to
										  * head of disk wildlist. This will
										  * modifiy the "next" pointer on the
										  * tail of this list and will make it
										  * non-NULL */

					 int rwmode, /* 0 = list any found ,
								  * 1 = list only those that can read,
								  * 2 = list only those that can write
								  * 3 = list only those that can both */

					 Boolean multiframe_only /* if true must read or write > 1
											  * frame */ )
{
struct pick_dat pdat;
Errcode err;
Names *namelist;
Pdr_entry *pdrlist = NULL;
Pdr_entry *current;
(void)local_names;

	hide_mp();

	pdat.name = name_buf;
	pdat.suffi = suffi_buf;

	/* Use the entire local_pdr list instead of the two Config_pdrs
	 * and any PDRs found in the resource directory.
	 */
	namelist = (Names *)local_pdrs;

	if((err = build_pdr_list(namelist,&pdrlist,&current,name_buf,
							 rwmode,multiframe_only)) < Success)
	{
		goto error;
	}


	err = go_driver_scroller( header, (Names *)pdrlist, (Names *)current,
							  pick_pdr, pdr_info_box, &pdat, NULL);

error:
	free_wild_list((Names **)&(pdrlist));
	err = softerr(err, "screen_menu");
	show_mp();
	return(err);
}
