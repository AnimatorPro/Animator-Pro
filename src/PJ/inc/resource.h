#ifndef RESOURCE_H
#define RESOURCE_H

extern char resource_dir[];

extern Errcode no_resource(Errcode err);

extern Errcode init_menu_resource(char *menu_file);
extern void cleanup_menu_resource(void);
extern Errcode init_resource_path(char *path);

extern char *make_resource_path(char *dir, char *name, char *path_buf);
extern char *make_resource_name(char *name, char *path_buf);

extern Boolean resource_exists(char *name);
extern Boolean req_resource_name(char *result, char *pat, char *hailing);

#endif /* RESOURCE_H */
