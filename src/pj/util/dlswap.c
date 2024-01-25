#include "linklist.h"

void swap_dl_list(Dlheader *a, Dlheader *b)
{
Dlheader swapper;

init_list(&swapper);
list_totail(a,&swapper);
list_totail(b,a);
list_totail(&swapper,b);
}
