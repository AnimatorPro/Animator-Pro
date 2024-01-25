
#include "stdtypes.h"
#include "ptrmacro.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "rfile.h"
#include "jfile.h"
#include "linklist.h"

void *tflush_alloc(long size)
{
void *pt;

if ((pt = trd_laskmem(size)) == NULL)
	{
	trd_compact(size);			/* try shrinking ram-disk */
	pt = trd_laskmem(size);
	}
return(pt);
}

/* magic numbers tagged at beginning and end of allocated memory blocks */
#define START_COOKIE (0x41f38327)
#define END_COOKIE (0x15998327)

old_video(){}

static long lastsize;
static long sizegot;
long pj_mem_last_fail;
long pj_mem_used;


void pj_free(void *p)
{
register ULONG *pt = p;
ULONG psize;
ULONG *endcookie;
static char callus[] =  "Internal error.  Gronk.";

if (!(*(--pt) == START_COOKIE) )
	{
	old_video();
	printf("%x bad START_COOKIE %lx\n", *pt, pt+1);
	puts(callus);
	exit(0);	/* okok... */
	}
*pt = 0;
psize = *(--pt);
pj_mem_used -= psize;
endcookie = OPTR(pt, psize-sizeof(LONG));
if (*endcookie != END_COOKIE)
	{
	old_video();
	printf("%x bad END_COOKIE %lx\n", *pt, pt+2);
	puts(callus);
	exit(0);	/* okok... */
	}
*endcookie = 0;
free(pt);	/* psize is size of block to free here if you need it... */
}

void *
pj_malloc(long size)
{
ULONG *endcookie;
ULONG *pt;

	if (size <= 0)
		return(NULL);
	lastsize = size;
	size += 3*sizeof(ULONG);	/* ask for cookie space too */
	if ((pt = tflush_alloc(size)) == NULL)
		{
		pj_mem_last_fail = lastsize;
		return(NULL);
		}
	*pt++ = size;
	pj_mem_used += size;
	*pt++ = START_COOKIE;
	endcookie = OPTR(pt, size-3*sizeof(ULONG));
	*endcookie = END_COOKIE;
	return(pt);
}



static short err_trans_table[] = {
	Success,
	Err_nogood,
	Err_no_file,
	Err_no_path,
	Err_too_many_files,
	Err_access,
	Err_file_not_open,
	Err_sys_mem_bad,
	Err_no_memory,
	Err_bad_address,
	Err_bad_env,
	Err_disk_format,
	Err_file_access,
	Err_data,
	Err_nogood,
	Err_no_device,
	Err_nogood,
	Err_nogood,
	Err_no_more_files,
	Err_write_protected,
	Err_no_device,
	Err_disk_not_ready,
	Err_nogood,
	Err_disk_data,
	Err_nogood,
	Err_seek,
	Err_not_dos_disk,
	Err_sector,
	Err_no_paper,
	Err_read,
	Err_write,
	Err_general_failure,
	Err_file_share,
	Err_file_lock,
	Err_disk_change,
	Err_nogood, /* "No FCB available", */
	Err_nogood, /* "Sharing buffer overflow", */
};

static short err2_trans_table[] =  {
Err_network,
Err_no_remote,
Err_network,
Err_network,
Err_network_busy,
Err_network,
Err_network,
Err_network,
Err_network,
Err_network,
Err_network,
Err_nogood, /* "Print queue full", */
Err_nogood, /* "Not enough space for print file", */
Err_nogood, /* "Print file was deleted", */
Err_network,
Err_access,
Err_network,
Err_network,
Err_network,
Err_network,
Err_share_pause,
Err_network,
Err_redirect_pause,	/* #72 */
Err_nogood,Err_nogood,Err_nogood,Err_nogood,Err_nogood,Err_nogood,Err_nogood,
								/* #73-79 */
Err_extant,
Err_nogood,
Err_directory_entry,
Err_critical,		/* critical error */
Err_network,
Err_network,
Err_nogood, 	/* "Invalid password", */
Err_nogood, 	/* "Invalid parameter", */
Err_network,
};

int pj_mserror(void)
/* return last Jfileio error from dos_open(), dos_create(), or dos_readrwite() */
{
int err = pj_dget_err();

if (err == 83)	/* critical error */
	err = Err_critical;
else if (err >= 0 && err <= Array_els(err_trans_table))
	err = err_trans_table[err];
else if (err >= 50 &&  err <= 88)
	err = err2_trans_table[err-50];
else
	err = Err_nogood;
return(err);
}

int txtcmp(register char *as, register char *bs)
{
register char a, b;

	for(;;)
	{
		a = *as++;
		b = *bs++;
		if(a == 0)
			break;

		if (isupper(a))
			a = _tolower(a);
		if (isupper(b))
			b = _tolower(b);
		if (a != b)
			break;
	}
	return(a-b);
}

Errcode errline(int err, char *s, ...)
{
printf("Error %d %s\n", err, s);
return(err);
}

tprint_stats()
{
long talloc, tfree;

rstats(&talloc, &tfree);
printf("talloc %ld  tfree %ld  tused %ld\n", talloc, tfree,  
	talloc - tfree);
}

copy_to_tempfile(char *inname, char *outname)
{
FILE *in;
Rfile out;
char buf[300];
long size;

if ((in = fopen(inname, "r")) == NULL)
	return(Err_no_file);
if ((out = rcreate(outname, JWRITEONLY)) == 0)
	return(rerror());
for (;;)
	{
	size = fread(buf, 1, 300, in);
	rwrite(out, buf, size);
	if (size < 300)
		break;
	}
fclose(in);
rclose(out);
}

print_tempfile(char  *name)
{
Rfile t;
char buf[300];
int len;

if ((t = ropen(name, JREADONLY)) == 0)
	return(rerror());
for (;;)
	{
	len = rread(t,buf,300);
	fwrite(buf,1,len,stdout);
	if (len < 300)
		break;
	}
rclose(t);
}

err_peep(char *name, int mode)
{
Rfile t;

t = ropen(name, mode);
printf("%s mode %d t %d err %d closeerr %d\n", 
	name, mode,  t, rerror(), rclose(t));
}

test_open_err()
{
err_peep("empty.foo", JREADONLY);
err_peep("empty.foo", JWRITEONLY);
err_peep("empty.foo", JREADWRITE);
err_peep("empty.foo", -10);
err_peep("empty.foo", JREADONLY);
puts("Deleting");
rdelete("empty.foo");
err_peep("empty.foo", JREADONLY);
}

make_rshort(char *name, short size)
{
short i;
Rfile t;
short inv;

if ((t = rcreate(name, JWRITEONLY)) != 0)
	{
	for (i=0; i<size; i++)
		{
		inv = size-i;
		rwrite(t, &inv, (long)sizeof(inv));
		}
	rclose(t);
	}
else
	{
	errline(rerror(), name);
	}
}

make_jshort(char *name, short size, int pri, int losable)
{
short i;
Jfile t;
short inv;
int lose_mask;


lose_mask = (losable ? JMEM_CAN_LOSE : 0);
if ((t = pj_create(name, JWRITEONLY|(1<<JSP_SHIFT)|lose_mask)) != 0)
	{
	for (i=0; i<size; i++)
		{
		inv = size-i;
		pj_write(t, &inv, (long)sizeof(inv));
		}
	pj_close(t);
	}
else
	{
	errline(pj_ioerr(), name);
	}
}

test_tempfile()
{
Errcode err;
char *fname;

tprint_stats();
fname = "main.c";
if ((err = copy_to_tempfile(fname, "tempfile.foo")) < Success)
	{
	errline(err, fname);
	return(err);
	}
tprint_stats();
fname = "makefile";
if ((err = copy_to_tempfile(fname, "makefile.foo")) < Success)
	{
	errline(err, fname);
	return(err);
	}
tprint_stats();
fname = "makefile";
if ((err = copy_to_tempfile(fname, "tempfile.foo")) < Success)
	{
	errline(err, fname);
	return(err);
	}
puts("After copying over tempfile.foo");
tprint_stats();
trd_compact();
make_rshort("2000.foo",2000);
puts("After make short 2000");
tprint_stats();
make_rshort("4000.foo",4000);
puts("After make short 4000");
tprint_stats();
trd_compact();
puts("After compact");
tprint_stats();
rdelete("2000.foo");
trd_compact();
puts("After compact");
tprint_stats();
return(Success);
}

testit()
{
make_rshort("2000.foo", 2000);
make_rshort("4000.foo", 4000);
copy_to_tempfile("makefile", "makefile.foo");
rdelete("2000.foo");
tprint_stats();
trd_compact();
tprint_stats();
make_rshort("3000.foo", 3000);
rdelete("3000.foo");
tprint_stats();
trd_compact();
tprint_stats();
print_tempfile("makefile.foo");
}

test_seek()
{
Rfile t;
int i;
short data;
long pos;

make_rshort("100.foo", 100);
t = ropen("100.foo", JREADWRITE);
/* for (i=99; i>=0; --i) */
rseek(t, 100L, JSEEK_START);
for (i=1; i<=101; i++)
	{
	if ((pos = rseek(t, (long)-4, JSEEK_REL)) <  Success)
		{
		errline((Errcode)pos, "100.foo");
		break;
		}
	rread(t, &data, (long)sizeof(data));
	printf("%d\n", data);
	}
rclose(t);
}

test_lots()
{
copy_to_tempfile("tfile.map", "v.doc");
test_seek();
test_tempfile();
testit();
puts("About to delete v.doc");
rdelete("v.doc");
puts("Ok here");
rdelete("2000.foo");
rdelete("3000.foo");
rdelete("4000.foo");
rdelete("100.foo");
rdelete("empty.foo");
rdelete("makefile.foo");
tprint_stats();
trd_compact();
tprint_stats();
tprint_dir();
rdelete("tempfile.foo");
tprint_stats();
trd_compact();
tprint_stats();
tprint_dir();
}

test_rfile()
{
test_lots();
}

print_jfile(char  *name)
{
Jfile t;
char buf[300];
int len;

if ((t = pj_open(name, JREADONLY)) == 0)
	return(pj_ioerr());
for (;;)
	{
	len = pj_read(t,buf,300);
	fwrite(buf,1,len,stdout);
	if (len < 300)
		break;
	}
pj_close(t);
return(Success);
}

Errcode  test_jfile()
{
Errcode err;

if ((err  = print_jfile("makefile")) < Success)
	{
	errline(err,"makefile");
	return(err);
	}
puts("------------------------------");
if ((err = pj_copyfile("makefile", ">:makefile")) < Success)
	return(err);
if ((err = pj_copyfile("tfile.c", ">:tfile.c")) < Success)
	return(err);
if ((err = pj_copyfile("makefile.tc", ">:makefile.tc")) < Success)
	return(err);
if ((err = pj_copyfile("main.c", ">:main.c")) < Success)
	return(err);
if ((err = pj_copyfile("tfile.exe", ">:tfile.exe")) < Success)
	return(err);
if ((err = pj_copyfile(">:tfile.exe", "H:tfile.exe")) < Success)
	return(err);
if ((err = pj_copyfile(">:main.c", "H:main.c")) < Success)
	return(err);
if ((err = pj_copyfile(">:makefile.tc", "H:makefile.tc")) < Success)
	return(err);
if ((err = pj_copyfile(">:tfile.c", "H:tfile.c")) < Success)
	return(err);
if ((err = pj_copyfile(">:makefile", "H:makefile")) < Success)
	return(err);
if ((err  = print_jfile(">:makefile")) < Success)
	{
	errline(err,">:makefile");
	return(err);
	}
puts("------------------------------");
if ((err  = print_jfile(">:make2")) < Success)
	{
	errline(err,">:make2");
	return(err);
	}
puts("------------------------------");
return(err);
}

test_two()
{
test_rfile();
test_jfile();
}

test_test()
{
puts("xxxxxxxxxxxxxx");
print_jfile("makefile");
puts("xxxxxxxxxxxxxx");
pj_copyfile("makefile", ">:makefile");
print_jfile(">:makefile");
puts("xxxxxxxxxxxxxx");
}

main(int argc, char *argv[])
{
Slnode *mem_eater = NULL;
Slnode *new;
int mem_k;
unsigned csz;

printf("max at %ld\n", set_rmax(250000L));
set_temp_path("h:\\jumbo;d:\\paat;");
test_test();
make_jshort(">:bscreen", 20000, 3, TRUE);
pj_copyfile("tfile.lnk", ">:tfile.lnk");
pj_rename(">:tfile.lnk", ">:Coo.lnk");
pj_copyfile("tfile.bak", ">:tfile.bak");
pj_copyfile("e:\\v\\v.doc", ">:v.doc");
pj_copyfile("makefile.wc", ">:makefile.wc");
pj_copyfile("e:\\v\\catmouse.fli", ">:catmouse.fli");
pj_copyfile("\\paa\\doc\\pj.doc", ">:pj.doc");
puts("******* initial dir************");
tprint_dir();
for (csz = 32; csz >= 1; csz >>= 1)
	{
	for (;;)
		{
		printf("    Asking for %ld bytes\n", csz*1024L);
		if ((new = pj_malloc(csz*1024L)) == NULL)
			break;
		new->next = mem_eater;
		mem_eater = new;
		}
	printf("******** csz = %u *********\n", csz*1024);
	tprint_dir();
	}
while ((new = mem_eater) != NULL)
	{
	mem_eater = mem_eater->next;
	pj_free(new);
	}
puts("Still alive at the end.");
tprint_dir();
puts("flushing all...");
trd_ram_to_files();
tprint_dir();
}


