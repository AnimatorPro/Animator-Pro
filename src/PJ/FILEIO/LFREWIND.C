#include "lfile.ih"

void lrewind(LFILE *f)
{
f->flags &= ~(BFL_ERR);
lfseek(f, 0L, LSEEK_SET);
}
