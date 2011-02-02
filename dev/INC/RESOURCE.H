
#ifndef RESOURCE_H
#define RESOURCE_H

char *make_resource_name(char *name, char *path_buf);
Errcode init_resource_dir(char *path);
Errcode init_resources(void);
Boolean req_resource_name(char *result, char *pat, char *hailing);
extern char resource_dir[];

#endif /* RESOURCE_H */
