#include <string.h>
#include "filepath.h"
#include "util.h"

bool suffix_in(char *path, char *suff)
{
	return(txtcmp(pj_get_path_suffix(path), suff) == 0);
}
