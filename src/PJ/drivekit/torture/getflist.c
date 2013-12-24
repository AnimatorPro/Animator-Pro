#include "torture.h"
#include "animinfo.h"
#include <stdio.h>
#include <io.h>


#define exists(f) (!access((f),0))

static char *skip_space(char *src)
/*****************************************************************************
 *
 ****************************************************************************/
{
	while (*src && *src <= ' ')     // skip leading whitespace
		++src;
	return src;
}

static char *copy_next_word(char *dest, char *src, Boolean is_path)
/*****************************************************************************
 * then copy a whitespace-delimited word.
 * if the word is a pathname, force a trailing '\' onto it.
 ****************************************************************************/
{
	char	lastc = 0;

	src = skip_space(src);			// skip leading whitespace

	while (*src > ' ') {            // copy until next whitespace
		if (*src == '/')            // if char is comment delimiter,
			*src = 0;				// nuke the rest of the input line.
		else
			*dest++ = lastc = *src++;
	}

	if (is_path && lastc != 0 && lastc != '\\')
		*dest++ = '\\';

	*dest = 0;
	return src;
}

static Errcode maybe_add_flic(char *filename, int xoffset, int yoffset)
/*****************************************************************************
 *
 ****************************************************************************/
{
	Errcode 	err;
	FlicList	*thisflic;
	FlicList	**cur;
	Anim_info	ainfo;

	if (Success > (err = get_flic_info(filename, &ainfo))) {
		log_progress("  Cannot load  '%s', bypassed.\n", filename);
		return FALSE;
	}

	if (ainfo.width  > tcb.display_raster.width
	||	ainfo.height > tcb.display_raster.height) {
		log_progress("  Skipped flic '%s' (%dx%d), too big for screen.\n",
			filename, ainfo.width, ainfo.height);
		return Success;
	}

	if (NULL == (thisflic = malloc(sizeof(*thisflic)))) {
		log_error("Out of memory\n");
		return Err_no_memory;
	}

	log_progress("  Added flic   '%s' (%dx%d) ", filename, ainfo.width, ainfo.height);
	strcpy(thisflic->name, filename);
	thisflic->width  = ainfo.width;
	thisflic->height = ainfo.height;

	if (xoffset < 0)
		xoffset = 0;

	if (yoffset < 0)
		yoffset = 0;

	if ((xoffset+ainfo.width) <= tcb.display_raster.width) {
		thisflic->xoffset = xoffset;
		log_progress("at x=%d, ", thisflic->xoffset);
	} else {
		thisflic->xoffset = tcb.display_raster.width-ainfo.width;
		log_progress("at (adjusted) x=%d, ", thisflic->xoffset);
	}

	if ((yoffset+ainfo.width) <= tcb.display_raster.width) {
		thisflic->yoffset = yoffset;
		log_progress("y=%d.\n", thisflic->yoffset);
	} else {
		thisflic->yoffset = tcb.display_raster.height-ainfo.height;
		log_progress("(adjusted) y=%d.\n", thisflic->yoffset);
	}

	for (cur = &tcb.fliclist; *cur != NULL; cur = &((*cur)->next))
		continue;	// walk to end of list

	thisflic->next = NULL;
	*cur = thisflic;

	return Success;
}

Errcode get_flic_names(void)
/*****************************************************************************
 *
 ****************************************************************************/
{
	Errcode err;
	int 	xoffset, yoffset;
	FILE	*listfile;
	char	*line;
	char	linebuf[256];
	char	defaultpath[256];
	char	workbuf[256];
	char	fullpath[256];
	Anim_info dummy;

	if (tcb.fliclist != NULL)	// already have names,
		return Success; 		// just return success.

	defaultpath[0] = 0;

	log_progress("Building list of flic files that fit %dx%d screen...\n",
		tcb.display_raster.width, tcb.display_raster.height);

	/*------------------------------------------------------------------------
	 * try to read the list file as if it were a flic file.  if this works,
	 * it means the command line /pFILENAME option was used to name a single
	 * flic file instead of a file containing a list of flic files.
	 *----------------------------------------------------------------------*/

	if (Success <= get_flic_info(tcb.list_file_name, &dummy)) {
		return maybe_add_flic(tcb.list_file_name, 0, 0);
	}

	/*------------------------------------------------------------------------
	 * if it wasn't a flic file, it must be a file containing a list of flic
	 * files.  Open it as a normal text file.
	 *----------------------------------------------------------------------*/

	if (NULL == (listfile = fopen(tcb.list_file_name, "r"))) {
		log_error("Cannot open list of test flics (file '%s')\n", tcb.list_file_name);
		return Err_nogood;
	}

	/*------------------------------------------------------------------------
	 * read each line in the list file, ignoring comments and remembering
	 * changes in the default path specified with path=.  For each entry
	 * that looks like a flic name, get the next two words and treat them
	 * as numbers which specify an x/y offset for playback.  Pass each
	 * name/x/y set to the maybe_add_flic() which will adjust the x/y to
	 * fit the screen (if needed) and add the flic to the internal list of
	 * test flics (if it will fit on the screeen.)
	 *----------------------------------------------------------------------*/

	for (;;) {
		if (NULL == fgets(linebuf, sizeof(linebuf), listfile))
			break;

		line = skip_space(linebuf);

		if (strnicmp(line, "path=", 5) == 0) {
			copy_next_word(defaultpath, &line[5], TRUE);
			continue;
		}

		line = copy_next_word(workbuf, line, FALSE);

		if (workbuf[0] == '\0')                       // if comment or no name
			continue;								  // ignore the entry.

		if (NULL != strchr(workbuf, '\\'))            // if name contains path
			strcpy(fullpath, workbuf);				  // go with it, else tack
		else										  // on default path.
			sprintf(fullpath, "%s%s", defaultpath, workbuf);

		line = copy_next_word(workbuf, line, FALSE);
		xoffset = strtol(workbuf, NULL, 10);
		line = copy_next_word(workbuf, line, FALSE);
		yoffset = strtol(workbuf, NULL, 10);

		if (Success != (err = maybe_add_flic(fullpath, xoffset, yoffset)))
			goto ERROR_EXIT;
	}

ERROR_EXIT:

	fclose(listfile);

	if (err == Success)
		log_progress("...file list built.\n\n");

	return err;
}

void free_flic_names(void)
/*****************************************************************************
 * free the items in the flic names list.
 ****************************************************************************/
{
	FlicList *cur, *next;

	for (cur = tcb.fliclist; cur != NULL; cur = next) {
		next = cur->next;
		free(cur->name);
		free(cur);
	}
}
