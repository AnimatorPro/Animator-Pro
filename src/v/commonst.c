/* commonst.c - contains read only strings used in more than one module */

#include "commonst.h"
#include "commonst.str"
#include "fs.h"

char cst_cancel[] = commonst_100 /* "Cancel" */;
char cst__cancel[] = commonst_102 /* " Cancel" */;
char cst_ok[]	= commonst_101 /* "Ok" */;
char cst_[] = "";
char cst_space[] = " ";
char cst_fsep[] = DIR_SEPARATOR_STR;
char cst_wild_all[] = "*.*";
