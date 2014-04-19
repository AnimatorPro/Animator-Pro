/* fs_unix.c */

#include <assert.h>
#include <glob.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "jimk.h"
#include "errcodes.h"
#include "filepath.h"
#include "memory.h"
#include "wildlist.h"
#include "textutil.h"

/* TODO: do we need current_device? */
#include "msfile.h"

long
pj_ddfree(int device)
{
	(void)device;
	return 32 * 1024 * 1024 * 1024L;
}

int
pj_get_devices(UBYTE *devices)
{
	(void)devices;
	return 0;
}

Errcode
current_device(char *dstr)
{
	*dstr++ = 'C';
	*dstr = '\0';

	return Success;
}

Boolean
pj_dmake_dir(const char *path)
{
	(void)path;
	return FALSE;
}

Errcode
pj_dget_dir(int drive, char *dir)
{
	(void)drive;
	(void)dir;
	return Success;
}

/*--------------------------------------------------------------*/

Errcode
get_full_path(const char *path, char *fullpath)
{
	char resolved_path[PATH_MAX];

	if (path == NULL || path[0] == '\0')
		path = ".";

	/* TODO: need to handle TFILE. */

	if (realpath(path, resolved_path) == NULL)
		return Err_no_path;

	return text_ncopy(fullpath, resolved_path, PATH_SIZE);
}

Errcode
make_good_dir(char *path)
{
	if (get_full_path(path, path) >= Success)
		return Success;

	if (get_full_path(".", path) >= Success)
		return Success;

	return Err_no_path;
}

/*--------------------------------------------------------------*/
/* Wild list.                                                   */
/*--------------------------------------------------------------*/

static Errcode
add_wild(Names **pwild_list, const char *path, Boolean is_directory)
{
	const char *prefix = is_directory ? DIR_DELIM_STR : "";
	const char *name;
	Wild_entry *next;
	size_t len;

	name = strrchr(path, DIR_DELIM);
	if (name == NULL) {
		name = path;
	}
	else {
		name++;
	}

	/* Filter out '.' and '..' */
	if (strncmp(name, ".", 2) == 0 || strncmp(name, "..", 3) == 0)
		return Success;

	/* Note: Wild_entry contains space for a \0. */
	len = strlen(prefix) + strlen(name);
	if ((next = pj_malloc(sizeof(Wild_entry) + len)) == NULL)
		return Err_no_memory;

	snprintf(next->name_buf, len + 1, "%s%s", prefix, name);

	next->hdr.name = next->name_buf;
	next->hdr.next = *pwild_list;
	*pwild_list = &(next->hdr);
	return Success;
}

static Errcode
alloc_wild_list(Names **pwild_list,
		const char *drawer, const char *wild, Boolean get_dirs)
{
	Errcode err;

	if (!pj_assert(pwild_list != NULL)) return Err_bad_input;
	if (!pj_assert(drawer != NULL)) return Err_bad_input;
	if (!pj_assert(wild != NULL)) return Err_bad_input;

	if (wild[0] == '#' && wild[1] == ':') {
		return Err_nogood;
	}
	else {
		struct stat s;
		char pat[PATH_MAX];
		glob_t g;
		size_t i;
		size_t dir_len;
		Boolean is_reg;
		Boolean is_dir;

		dir_len = strlen(drawer);
		if (dir_len <= 0) {
			pat[0] = '\0';
		}
		else if (drawer[dir_len - 1] == DIR_DELIM) {
			snprintf(pat, sizeof(pat), "%s", drawer);
		}
		else {
			snprintf(pat, sizeof(pat), "%s" DIR_DELIM_STR, drawer);
			dir_len++;
		}

		snprintf(pat + dir_len, sizeof(pat) - dir_len, "%s", wild);
		glob(pat, GLOB_TILDE, NULL, &g);

		if (get_dirs) {
			snprintf(pat + dir_len, sizeof(pat) - dir_len, "*");
			glob(pat, GLOB_APPEND | GLOB_TILDE | GLOB_ONLYDIR, NULL, &g);
		}

		for (i = 0; i < g.gl_pathc; i++) {
			if (stat(g.gl_pathv[i], &s) != 0)
				continue;

			is_reg = S_ISREG(s.st_mode);
			is_dir = S_ISDIR(s.st_mode);
			if ((!is_reg && !is_dir) || (is_dir && !get_dirs))
				continue;

			err = add_wild(pwild_list, g.gl_pathv[i], is_dir);
			if (err < Success)
				goto error;
		}

		globfree(&g);
	}

	return Success;

error:
	free_wild_list(pwild_list);
	return err;
}

Errcode
build_wild_list(Names **pwild_list,
		const char *drawer, const char *pat, Boolean get_dirs)
{
	Errcode err;

	if (!pj_assert(pwild_list != NULL)) return Err_bad_input;
	if (!pj_assert(drawer != NULL)) return Err_bad_input;
	if (!pj_assert(pat != NULL)) return Err_bad_input;

	*pwild_list = NULL;

	err = alloc_wild_list(pwild_list, drawer, pat, get_dirs);
	if (err >= Success)
		*pwild_list = sort_names(*pwild_list);

	return err;
}
