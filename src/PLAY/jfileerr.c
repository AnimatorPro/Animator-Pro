#include "jimk.h"
#include "jfile.str"

cant_find(name)
char *name;
{
char *bufs[3];

bufs[0] = jfile_100 /* "Sorry AAPlay can't find:" */,
bufs[1] = name;
bufs[2] = NULL;
continu_box(bufs);
}

truncated(filename)
char *filename;
{
char *bufs[3];

bufs[0] = (filename ? filename : "");
bufs[1] = jfile_102 /* "File truncated!" */;
bufs[2] = NULL;
continu_box(bufs);
}

mangled(name)
char *name;
{
char *bufs[2];

bufs[0] = jfile_103 /* "File mangled:" */;
bufs[1] = name;
continu_line(bufs);
}
