
/* jfile.c - ask MS-DOS to read,write, and seek without any buffering.
   Arguments are MS-DOS file handle (int), file-name (char *),
   buffer space pointers (32 bit large model pointers), and
   byte counts/offsets (32 bit long integers).
   */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "jimk.h"
#include "jfile.h"
#include "jfile.str"

int
read_gulp(const char *name, char *buf, long size)
{
FILE *f;

if ((f = jopen(name,0))==0)
	{
	cant_find(name);
	return(0);
	}
if (jread(f, buf, size) < size)
	{
	truncated(name);
	jclose(f);
	return(0);
	}
jclose(f);
return(1);
}

int
write_gulp(const char *name, const char *buf, long size)
{
FILE *f;

if ((f = jcreate(name))==0)
	{
	cant_create(name);
	return(0);
	}
if (jwrite(f, buf, size) < size)
	{
	truncated(name);
	jclose(f);
	jdelete(name);
	return(0);
	}
jclose(f);
return(1);
}

cant_create(name)
char *name;
{
char *bufs[3];

bufs[0] = jfile_100 /* "Sorry Converter can't create:" */,
bufs[1] = name;
bufs[2] = NULL;
continu_box(bufs);
}

cant_find(name)
char *name;
{
char *bufs[3];

bufs[0] = jfile_101 /* "Sorry Converter can't find:" */,
bufs[1] = name;
bufs[2] = NULL;
continu_box(bufs);
}

truncated(filename)
char *filename;
{
char *bufs[3];

if (filename == NULL)
	filename = "";
bufs[0] = filename;
bufs[1] = jfile_103 /* "File truncated!" */;
bufs[2] = NULL;
continu_box(bufs);
}

overwrite_old(name)
char *name;
{
char bud[90];
char *bufs[3];
int len;

if (!jexists(name) )
	return(1);
bufs[0] = jfile_104 /* "Overwrite old" */;
sprintf(bud, "%s?", name);
len = strlen(bud);	/* chop long file name */
if (len > 40)
	bufs[1] = bud+len-40;
else
	bufs[1] = bud;
bufs[2] = NULL;
return(yes_no_box(bufs));
}

mangled(name)
char *name;
{
char *bufs[3];

bufs[0] = jfile_106 /* "File mangled:" */;
bufs[1] = name;
bufs[2] = NULL;
continu_box(bufs);
}


#ifdef SLUFFED
check_abort(frame, of)
unsigned frame, of;
{
char *bufs[5];
char buf[40];

check_input();
if (key_hit || RJSTDN)
	{
	bufs[0] = "Abort multiple-frame rendering";
	sprintf(buf, "on frame %u of %u?", frame, of);
	bufs[1] = buf;
	bufs[2] = NULL;
	return(yes_no_box(bufs) );
	}
else
	return(0);
}
#endif /* SLUFFED */
