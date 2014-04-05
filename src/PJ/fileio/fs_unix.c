/* fs_unix.c */

#include <assert.h>
#include <glob.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "jimk.h"
#include "errcodes.h"
#include "filepath.h"
#include "memory.h"
#include "wildlist.h"

/* TODO: do we need pj_change_device and current_device? */
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
pj_change_device(char *name)
{
	(void)name;
	return Success;
}

Errcode
current_device(char *dstr)
{
	*dstr++ = 'C';
	*dstr = '\0';

	return Success;
}

Errcode
change_dir(char *name)
{
	(void)name;
	return Success;
}

Boolean
pj_dmake_dir(char *path)
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

Errcode
make_good_dir(char *path)
{
	(void)path;
	return Success;
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
	if ((name[0] == '.') && (name[1] == '\0' || name[1] == '.'))
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
alloc_wild_list(Names **pwild_list, const char *wild, Boolean get_dirs)
{
	Errcode err;

	*pwild_list = NULL;

	if (wild[0] == '#' && wild[1] == ':') {
		return Err_nogood;
	}
	else {
		const char *drawer = ".";
		struct stat s;
		char pat[1024];
		glob_t g;
		size_t nmatches;
		size_t i;
		size_t dir_len;

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
		nmatches = g.gl_pathc;

		if (get_dirs) {
			snprintf(pat + dir_len, sizeof(pat) - dir_len, "*");
			glob(pat, GLOB_APPEND | GLOB_TILDE | GLOB_ONLYDIR, NULL, &g);
		}

		for (i = 0; i < nmatches; i++) {
			if (stat(g.gl_pathv[i], &s) != 0)
				continue;

			if (!S_ISREG(s.st_mode))
				continue;

			if ((err = add_wild(pwild_list, g.gl_pathv[i], FALSE)) != Success)
				goto error;
		}

		for (i = nmatches; i < g.gl_pathc; i++) {
			if (stat(g.gl_pathv[i], &s) != 0)
				continue;

			if (!S_ISDIR(s.st_mode))
				continue;

			if ((err = add_wild(pwild_list, g.gl_pathv[i], TRUE)) != Success)
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
build_wild_list(Names **pwild_list, const char *pat, Boolean get_dirs)
{
	Errcode err;

	if ((err = alloc_wild_list(pwild_list, pat, get_dirs)) >= Success) {
		*pwild_list = sort_names(*pwild_list);
	}

	return err;
}
