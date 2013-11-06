/* fs_unix.c */

#include <assert.h>
#include <dirent.h>
#include <glob.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "jimk.h"
#include "fs.h"

char devices[26];
int dev_count;

extern char drawer[71];

int
change_dev(int newdev)
{
	return 0;
}

int
change_dir(const char *name)
{
	struct stat s;

	if (stat(name, &s) == 0) {
		return S_ISDIR(s.st_mode);
	}

	return 0;
}

void
get_devices(void)
{
	dev_count = 0;
	devices[0] = 0;
}

int
make_current_drawer(void)
{
	getcwd(drawer, sizeof(drawer));
	return 0;
}

void
make_path_name(const char *drawer, char *file, const char *suffix,
		char *path)
{
	int len;

	len = strlen(drawer);
	if (len == 0) {
		path[0] = '\0';
	}
	else {
		strcpy(path, drawer);
		if (path[len-1] != DIR_SEPARATOR_CHAR) {
			path[len++] = DIR_SEPARATOR_CHAR;
			path[len] = '\0';
		}
	}

	rtrm(file, strlen(file));
	strcat(path, file);
	if (suffix[0] == '.' && suffix[1] != '*') {
		if (!suffix_in(file, suffix))
			strcat(path, suffix);
	}
}

void
fs_go_rootdir(void)
{
	snprintf(drawer, sizeof(drawer), DIR_SEPARATOR_STR);
}

void
fs_go_updir(void)
{
	int len = strlen(drawer);
	char *d = drawer;

	if (len > 0) {
		if (d[0] == DIR_SEPARATOR_CHAR) {
			d++;
			len--;
		}
	}

	while (--len >= 0) {
		char c = d[len];
		d[len] = '\0';
		if (c == DIR_SEPARATOR_CHAR)
			break;
	}
}

void
fs_build_wild_list(const char *wild)
{
	char pat[1024];
	glob_t g;
	size_t ndirs;
	size_t i;
	size_t dir_len;

	dir_len = strlen(drawer);
	if (dir_len <= 0) {
		pat[0] = '\0';
	}
	else if (drawer[dir_len - 1] == DIR_SEPARATOR_CHAR) {
		snprintf(pat, sizeof(pat), "%s", drawer);
	}
	else {
		snprintf(pat, sizeof(pat), "%s" DIR_SEPARATOR_STR, drawer);
		dir_len++;
	}

	snprintf(pat + dir_len, sizeof(pat) - dir_len, "*");
	glob(pat, GLOB_TILDE | GLOB_ONLYDIR, NULL, &g);
	ndirs = g.gl_pathc;

	snprintf(pat + dir_len, sizeof(pat) - dir_len, "%s", wild);
	glob(pat, GLOB_APPEND | GLOB_TILDE, NULL, &g);

	for (i = 0; i < ndirs; i++) {
		struct stat s;

		if (stat(g.gl_pathv[i], &s) == 0) {
			if (S_ISDIR(s.st_mode))
				add_wild(g.gl_pathv[i], FILETYPE_DIRECTORY);
		}
	}

	for (i = ndirs; i < g.gl_pathc; i++)
		add_wild(g.gl_pathv[i], FILETYPE_REGULAR);

	globfree(&g);
}
