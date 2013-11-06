#ifndef FS_H
#define FS_H

#if defined(__TURBOC__)
#define DIR_SEPARATOR_CHAR '\\'
#define DIR_SEPARATOR_STR  "\\"
#else
#define DIR_SEPARATOR_CHAR '/'
#define DIR_SEPARATOR_STR  "/"
#endif

enum FileType {
	FILETYPE_DIRECTORY,
	FILETYPE_REGULAR
};

struct file_list
{
	struct file_list *next;
	char *name;
	enum FileType type;
};
typedef struct file_list File_list;

extern struct file_list *wild_lst;

/* Function: rtrm */
extern void rtrm(char *s, int i);

/* Function: suffix_in
 *
 *  See if string ends with suff.
 */
extern int suffix_in(const char *string, const char *suff);

/* Function: change_dev
 *
 *  newdev - (1) A:, (2) B:, ...
 */
extern int change_dev(int newdev);

/* Function: change_dir */
extern int change_dir(const char *name);

/* Function: get_devices */
extern void get_devices(void);

/* Function: make_current_drawer */
extern int make_current_drawer(void);

/* Function: make_path_name */
extern void
make_path_name(const char *drawer, char *file, const char *suffix,
		char *path);

/* Function: fs_go_rootdir */
extern void fs_go_rootdir(void);

/* Function: fs_go_updir */
extern void fs_go_updir(void);

/* Function: add_wild */
extern void add_wild(const char *path, enum FileType type);

/* Function: fs_build_wild_list */
extern void fs_build_wild_list(const char *wild);

#endif
