#ifndef COMMONST_H
#define COMMONST_H

#ifndef STDTYPES_H
#include "stdtypes.h"
#endif

struct softmenu;

/* commonst.h - place where common strings like "ok", "cancel", "load",
 * "save", etc. are defined */

extern char *any_continue;
extern char *enter_choice;
extern char *continue_str;
extern char *yes_str;
extern char *no_str;
extern char *ok_str;
extern char *cancel_str;
extern char *load_str;
extern char *save_str;
extern char *unnamed_str;
extern char *hit_enter_to;
extern char *please_wait_str;

extern char space_str[];
extern char empty_str[];

extern Errcode init_common_str(struct softmenu *sm);
extern void default_common_str(void);
extern void cleanup_common_str(void);

#endif /* COMMONST_H */
