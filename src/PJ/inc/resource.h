#ifndef RESOURCE_H
#define RESOURCE_H

#ifndef ARGPARSE_H
#include "argparse.h"
#endif

extern char resource_dir[];

/* cleanup.c */
extern void close_downto_screen(void);
extern void cleanup_all(Errcode err);

/* resource.c */
extern Errcode init_pj_resources(void);
extern void cleanup_resources(void);

/* pjhigh/resource.c */
extern Errcode no_resource(Errcode err);

extern Errcode init_menu_resource(char *menu_file);
extern void cleanup_menu_resource(void);
extern Errcode init_resource_path(char *path);

extern char *make_resource_path(char *dir, char *name, char *path_buf);
extern char *make_resource_name(char *name, char *path_buf);

extern Boolean resource_exists(char *name);
extern Boolean req_resource_name(char *result, char *pat, char *hailing);

/* Platform specific. */
extern Errcode
init_pj_startup(Argparse_list *more_args, Do_aparse do_others,
		int argc, char **argv, char *help_key, char *menufile_name);

extern void cleanup_startup(void);

#endif
