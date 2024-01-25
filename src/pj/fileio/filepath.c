/* filepath.c
 *
 * Common path string manipulation routines.
 */

#include <string.h>
#include "pjassert.h"
#include "errcodes.h"
#include "filepath.h"
#include "linklist.h"
#include "memory.h"
#include "textutil.h"

enum {
	MAX_DEV_LEN = 4
};

typedef struct filepathnode {
	Dlnode dl;
	char name[1]; /* node name + NUL. */
} FilePathNode;

struct filepath {
	Dlheader dl;

	enum FilePathType type;
	size_t len;
	char name[1]; /* device name + NUL. */
};

/* Function: filepath_create_header
 *
 *  Allocates a new FilePath, and initialises the device string.
 */
static FilePath *
filepath_create_header(const char *str)
{
	FilePath *filepath;
	enum FilePathType type;
	size_t len;
	char drive;

	if (!pj_assert(str != NULL)) return NULL;

	drive = ('a' <= str[0] && str[0] <= 'z') ? (str[0] + 'A' - 'a') : str[0];

	if (str[0] == DIR_DELIM || str[0] == DIR_DELIM2) {
		type = FILEPATH_ROOT;
		len = 1;
	}
	else if (('A' <= drive && drive <= 'Z')
			&& (str[1] == DEV_DELIM)
			&& (str[2] == DIR_DELIM || str[2] == DIR_DELIM2)) {
		type = FILEPATH_DOS;
		len = 3;
	}
	else {
		type = FILEPATH_NO_PREFIX;
		len = 0;
	}

	filepath = pj_malloc(sizeof(*filepath) + len);
	if (filepath == NULL)
		return NULL;

	if (init_list(&filepath->dl) != Success)
		goto cleanup;

	filepath->type = type;
	filepath->len = len;

	if (len > 0)
		memcpy(filepath->name, str, len);

	filepath->name[len] = '\0';
	return filepath;

cleanup:
	filepath_destroy(filepath);
	return NULL;
}

/* Function: filepath_append_node
 *
 *  Appends a new FilePathNode, copying the path element name.
 *  Returns the number of characters parsed, or an error code.
 */
static Errcode
filepath_append_node(FilePath *filepath, const char *str)
{
	FilePathNode *node;
	Errcode err;
	size_t len;

	if (!pj_assert(filepath != NULL)) return Err_bad_input;
	if (!pj_assert(str != NULL)) return Err_bad_input;

	len = text_count_until_dir_delim(str);
	if (len == 0)
		return Success;

	node = pj_malloc(sizeof(*node) + len);
	if (node == NULL)
		return Err_no_memory;

	err = text_ncopy_dir_delim(node->name, str, len + 1);
	if (err > 0) {
		const Errcode err2 = add_tail(&filepath->dl, &node->dl);
		if (err2 >= Success)
			return err;

		err = err2;
	}

	pj_free(node);
	return err;
}

/* Function: filepath_create_from_string
 *
 *  Converts a path string into a FilePath structure for easy
 *  manipulation.
 */
FilePath *
filepath_create_from_string(const char *str)
{
	FilePath *filepath;
	Errcode err;

	if (!pj_assert(str != NULL)) return NULL;

	filepath = filepath_create_header(str);
	if (filepath == NULL)
		return NULL;

	str += filepath->len;
	while (*str != '\0') {
		if (*str == DIR_DELIM || *str == DIR_DELIM2) {
			str++;
			continue;
		}

		err = filepath_append_node(filepath, str);
		if (err < Success)
			goto cleanup;

		str += err;
	}

	return filepath;

cleanup:
	filepath_destroy(filepath);
	return NULL;
}

/* Function: filepath_destroy
 *
 *  Destroy a FilePath structure.
 */
Errcode
filepath_destroy(FilePath *filepath)
{
	Errcode err;

	if (!pj_assert(filepath != NULL)) return Err_bad_input;

	err = free_dl_list(&filepath->dl);
	if (err != Success)
		return err;

	pj_free(filepath);
	return Success;
}

/* Function: filepath_append
 *
 *  Appends a path element into the file path.
 *  The string shouldn't contain a directory delimiter.
 */
Errcode
filepath_append(FilePath *filepath, const char *str)
{
	return filepath_append_node(filepath, str);
}

/* Function: filepath_drop_tail
 *
 *  Remove the last path element from the file path unless it is the
 *  device.
 */
Errcode
filepath_drop_tail(FilePath *filepath)
{
	Dlnode *node;

	if (!pj_assert(filepath != NULL)) return Err_bad_input;

	node = get_tail(&filepath->dl);
	if (node != NULL)
		pj_free(node);

	return Success;
}

/* Function: filepath_to_cstr
 *
 *  Converts a FilePath structure back to a string.
 *  Returns the length of the string, or an error code.
 */
Errcode
filepath_to_cstr(const FilePath *filepath, char delim, char *str, size_t n)
{
	Errcode err;
	const Dlnode *node;
	char *start = str;

	if (!pj_assert(filepath != NULL)) return Err_bad_input;
	if (!pj_assert(filepath->dl.head != NULL)) return Err_internal_pointer;
	if (!pj_assert(str != NULL)) return Err_bad_input;
	if (!pj_assert(n > 0)) return Err_range;

	if (filepath->len > 0) {
		err = text_ncopy(str, filepath->name, n);
		if (err < Success)
			return err;

		str += err;
		n -= err;
	}

	for (node = filepath->dl.head; node->next != NULL; node = node->next) {
		const FilePathNode *curr = (const FilePathNode *)node;

		err = text_ncopy(str, curr->name, n);
		if (err < Success)
			return err;

		/* If not last element, replace NUL with delim. */
		if ((err > 0) && !is_tail(node)) {
			if (str[err-1] != DIR_DELIM && str[err-1] != DIR_DELIM2) {
				str[err] = delim;
				err++;
			}
		}

		str += err;
		n -= err;
	}

	*str = '\0';
	return str - start;
}
