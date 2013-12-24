/*****************************************************************************
 * FLICNAME.C - Complete a flic filename string by adding appropriate type.
 ****************************************************************************/

#include "flicglue.h"

static char *modulename = __FILE__;

static char flc_type[] = ".FLC";
static char fli_type[] = ".FLI";

Errcode pj_flic_complete_filename(char *path, AnimInfo *ainfo, Boolean force_type)
/*****************************************************************************
 * complete the filetype part of a flic path/file name, based on flic sizes.
 * - there must be something in the filename to start with (no empty strings).
 * - a 320x200 flic is a .FLI file, anything else is a .FLC file.
 * - if no filetype exists, the appropriate one will be added.
 * - if the force_type flags is true, the type will always be added/replaced.
 * - the existing string is modified in place, there must be room to add
 *	 four characters to the string, or Bad Things will happen.
 ****************************************************************************/
{
	register char	 *pcur;
	char	*last_path_delim = NULL;
	char	*last_type_delim = NULL;
	char	*thetype;

	/*------------------------------------------------------------------------
	 * validate the parms...
	 *----------------------------------------------------------------------*/

	if (NULL == path || NULL == ainfo)
		return pj_error_internal(Err_internal_pointer, modulename, __LINE__);

	if ('\0' == *path)
		return pj_error_internal(Err_internal_parameter, modulename, __LINE__);

	/*------------------------------------------------------------------------
	 * find the last path delim and last type delim in the name...
	 *----------------------------------------------------------------------*/

	pcur = path;
	while (*pcur) {
		switch (*pcur) {
		  case '\\':
			last_path_delim = pcur;
			break;
		  case '.':
			last_type_delim = pcur;
			break;
		}
		++pcur;
	}

	/*------------------------------------------------------------------------
	 * if there was no type delim found, or the only type delim found was
	 * in a path node (not on the filename), set the last type delim to
	 * point at the nullterm char at the end of the string.
	 *----------------------------------------------------------------------*/

	if (NULL == last_type_delim || last_path_delim > last_type_delim)
		last_type_delim = pcur;

	/*------------------------------------------------------------------------
	 * if it's a 320x200 flic, it's a .FLI file, else it's a .FLC file...
	 *----------------------------------------------------------------------*/

	if (320 == ainfo->width && 200 == ainfo->height)
		thetype = fli_type;
	else
		thetype = flc_type;

	/*------------------------------------------------------------------------
	 * if there was no type, or if we're forcing the type, put the type in...
	 *----------------------------------------------------------------------*/

	if (force_type || '\0' == *last_type_delim)
		strcpy(last_type_delim, thetype);

	return Success;
}
