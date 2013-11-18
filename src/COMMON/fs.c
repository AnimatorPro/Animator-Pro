/* fs.c */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jimk.h"
#include "fs.h"
#include "memory.h"

char devices[26];
int dev_count;
struct file_list *wild_lst;

/* compare two strings ignoring case */
static int
ustrcmp(const char *as, const char *bs)
{
	for (;;) {
		char a = tolower(*as++);
		char b = tolower(*bs++);
		if (a != b)
			return a-b;
		if (a == '\0')
			return 0;
	}
}

static void
rtrm(char *s, int i)
{
	i--;
	while (i >= 0 && s[i] == ' ')
		s[i--] = '\0';
}

int
suffix_in(const char *string, const char *suff)
{
	string += strlen(string) - strlen(suff);
	return (ustrcmp(string, suff) == 0);
}

int
valid_device(int d)
{
	int i;

	for (i = 0; i < dev_count; i++) {
		if (devices[i] == d)
			return 1;
	}
	return 0;
}

void
make_path_name_suffix(const char *drawer, char *file, const char *suffix,
		char *path)
{
	if (!(suffix[0] == '.' && suffix[1] == '*')) { /* dont add in suffix if wild card */
		if (!suffix_in(file, suffix)) { /* add in suffix ... ldg */
			int flen;

			rtrm(file, flen = strlen(file));
			if (file[flen-1] == '.')
				file[--flen] = '\0'; /* remove dot */

			if (strlen(suffix) < (80-flen))
				strcat(file, suffix); /* 80 is hard coded len */
		}
	}

	make_path_name(drawer, file, path);
}

void
add_wild(const char *path, enum FileType type)
{
	struct file_list *next;
	const char *name;

	name = strrchr(path, DIR_SEPARATOR_CHAR);
	if (name == NULL) {
		name = path;
	}
	else {
		name++;
	}

	if (name[0] == '.') { /* filter out '.' and '..' */
		if (name[1] == '\0' || name[1] == '.')
			return;
	}

	next = (struct file_list *)askmem(sizeof(*next));
	if (next == NULL)
		return;

	next->type = type;
	if (type == FILETYPE_DIRECTORY) {
		int len = strlen(name) + 2;
		next->name = askmem(len);
		if (next->name != NULL) {
			sprintf(next->name, "%c%s", DIR_SEPARATOR_CHAR, name);
		}
	}
	else {
		next->name = clone_string(name);
	}

	if (next->name == NULL) {
		freemem(next);
		return;
	}
	next->next = wild_lst;
	wild_lst = next;
}
