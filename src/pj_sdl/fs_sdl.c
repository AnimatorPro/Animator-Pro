/* fs_unix.c */

#include "aaconfig.h"
#include "errcodes.h"
#include "filepath.h"
#include "jimk.h"
#include "memory.h"
#include "textutil.h"
#include "tfile.h"
#include "wildlist.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <SDL3/SDL_filesystem.h>
#include "pj_sdl.h"

/* TODO: do we need current_device? */
#include "msfile.h"

// from pj_sdl.c
extern const char* SEP;

long pj_ddfree(int device)
{
	(void)device;
	return 32 * 1024 * 1024 * 1024L;
}

int pj_get_devices(UBYTE* devices)
{
	(void)devices;
	return 0;
}

Errcode current_device(char* dstr)
{
	*dstr++ = 'C';
	*dstr	= '\0';

	return Success;
}

bool pj_dmake_dir(const char* path)
{
	(void)path;
	return false;
}

Errcode pj_dget_dir(int drive, char* dir)
{
	(void)drive;
	(void)dir;
	return Success;
}

/*--------------------------------------------------------------*/
Errcode get_full_path(const char* path, char* fullpath)
{
	char resolved_path[PATH_SIZE];

	if (path == NULL || path[0] == '\0') {
		path = ".";
	}

	// Handle TFILEs-- a lot of basic functionality requires writing to disk
	if (is_tdrive(path)) {
		path += 2;
		snprintf(resolved_path, PATH_SIZE, "%s%s%s", vconfg.temp_path, SEP, path);
	}
	else {
		strlcpy(resolved_path, path, PATH_SIZE);
	}

	//!TODO: fix this with a proper abspath function
//	if (realpath(resolved_path, resolved_path) == NULL) {
//		char* errStr = strerror(errno);
//		printf("%s >> %s\n", fullpath, resolved_path);
//		printf("error string: %s\n", errStr);
//		return Err_no_path;
//	}

	size_t result = strlcpy(fullpath, resolved_path, PATH_SIZE);
	return result == strnlen(resolved_path, PATH_SIZE) ? Success : Err_corrupted;
}

Errcode make_good_dir(char* path)
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

static Errcode add_wild(Names** pwild_list, const char* path, bool is_directory)
{
	const char* prefix = is_directory ? DIR_DELIM_STR : "";
	const char* name;
	Wild_entry* next;
	size_t len;

	name = strrchr(path, DIR_DELIM);
	if (name == NULL) {
		name = path;
	} else {
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
	*pwild_list	   = &(next->hdr);
	return Success;
}

static Errcode alloc_wild_list(Names** pwild_list,
							   const char* search_folder,
							   const char* wild,
							   bool get_dirs)
{
	Errcode err;

	if (!pj_assert(pwild_list != NULL))
		return Err_bad_input;
	if (!pj_assert(search_folder != NULL))
		return Err_bad_input;
	if (!pj_assert(wild != NULL))
		return Err_bad_input;

	if (wild[0] == '#' && wild[1] == ':') {
		return Err_nogood;
	}

	struct stat s;
	char pat[PATH_MAX];
	size_t i;
	size_t dir_len;

	dir_len = strlen(search_folder);
	if (dir_len <= 0) {
		pat[0] = '\0';
	} else if (search_folder[dir_len - 1] == DIR_DELIM) {
		snprintf(pat, sizeof(pat), "%s", search_folder);
	} else {
		snprintf(pat, sizeof(pat), "%s" DIR_DELIM_STR, search_folder);
		dir_len++;
	}

	int count = 0;
	SDL_PathInfo info;
	char full_path[1024];
	char** files = SDL_GlobDirectory(search_folder, "*", SDL_GLOB_CASEINSENSITIVE, &count);

	for (i = 0; i < count; i++) {
		snprintf(full_path, 1024, "%s/%s", search_folder, files[i]);
		if (SDL_GetPathInfo(full_path, &info) != 0) {
			fprintf(stderr, "-- Error on %s: %s\n", files[i], SDL_GetError());
			continue;
		}

		if (info.type == SDL_PATHTYPE_NONE || info.type == SDL_PATHTYPE_OTHER) {
			// file can't be accessed
			continue;
		}

		if (info.type == SDL_PATHTYPE_DIRECTORY && (!get_dirs)) {
			continue;
		}

		err = add_wild(pwild_list, files[i],
					   info.type == SDL_PATHTYPE_DIRECTORY);
		if (err < Success) {
			free_wild_list(pwild_list);
			return err;
		}
	}

	SDL_free(files);
	return Success;
}

Errcode build_wild_list(Names** pwild_list, const char* drawer, const char* pat,
						bool get_dirs)
{
	Errcode err;

	if (!pj_assert(pwild_list != NULL))
		return Err_bad_input;
	if (!pj_assert(drawer != NULL))
		return Err_bad_input;
	if (!pj_assert(pat != NULL))
		return Err_bad_input;

	*pwild_list = NULL;

	err = alloc_wild_list(pwild_list, drawer, pat, get_dirs);
	if (err < Success) {
		return err;
	}

	/*
	struct names* node = pwild_list;
	while (node) {
		if (node->name) {
			fprintf(stderr, "Wild: %s\n", node->name);
		}
		else {
			fprintf(stderr, "-- No name.\n");
		}
		node = node->next;
	}
	*/

	*pwild_list = sort_names(*pwild_list);
	return Success;
}

bool pj_is_directory(const char *path)
{
	SDL_PathInfo info;

	if (SDL_GetPathInfo(path, &info) != 0) {
		return false;
	}

	return info.type == SDL_PATHTYPE_DIRECTORY;
}