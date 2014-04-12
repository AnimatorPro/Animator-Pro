/** tfile.c - Temporary file system.
 ** This implements two devices =: and >: where files may be stored.
 ** Files on the >: device reside in a directory somewhere on the
 ** temporary file path.   Files on the =: device will reside on
 ** internal ramdisk (#:) if there is free ram,  or somewhere on
 ** the temporary path otherwise.   Files in the =: may be moved
 ** out of #: and onto hard-disk if the system is low on memory.
 **
 **/

#include <stdio.h>
#include <string.h>
#define TFILE_C
#include "jimk.h"
#include "errcodes.h"
#include "jfile.h"
#include "linklist.h"
#include "memory.h"
#include "msfile.h"
#include "ptrmacro.h"
#include "rfile.h"
#include "tfile.h"
#include "util.h"
#include "wildlist.h"

static char temp_path[2*PATH_SIZE] = {TRD_CHAR, ':', 0};
static int tfile_lockmem = 0;

char *get_temp_path(void)
{
return(temp_path+3);
}

Boolean is_tdrive(char *device)
/*
 * Returns true if first letter of device is one of those non-alphabetic
 * temporary or ram devices.
 */
{
	switch(*device++)
	{
		case TDEV_MED:
		case TDEV_LO:
		case TRD_CHAR:
			return(*device == ':');
		default:
			return(0);
	}
}
static int terr;

typedef struct path_part
/* A single directory in the temporary path */
	{
	Dlnode node; /* Link to list of parts */
	char *path;	 /* Device/directory as a string */
	short device;	 /* 1 for A:,  2 for B: ... 26 for Z:  27 for #: */
	} Path_part;

struct tfl
	{
	Dlnode tnod;
	Jfile jhandle;
	char name[FILE_NAME_SIZE+2];
	char path[PATH_SIZE];
		/* Next three variables are only used when changing temp path 
		 * or moving files. */
	short omode;	/* Open mode */
	long opos;		/* Open file position */
	short omoved;	/* Is it moved to a new dir? */
	};

static Dlheader tlist = DLHEADER_INIT(tlist);

#ifdef SLUFFED
static void print_parts(char *print_head, Dlheader *list)
{
register Dlnode *node;
register Dlnode *next;
Path_part *p;

for(node = list->head;
	NULL != (next = node->next);
	node = next)
	{
	p = (Path_part *)node;
	boxf("%s\ndevice %d path %s\n", 
		print_head, p->device, p->path);
	}
}
#endif /* SLUFFED */

static Errcode init_part(Path_part **ppart, char *pname)
/*
 * Allocate memory for a path_part struct and initialize it.  Return
 * result in *ppart.
 */
{
int len;
Path_part *p;

len = strlen(pname)+1;
if ((*ppart = p = trd_askcmem(sizeof(*p)+len)) == NULL)
	return((terr = Err_no_memory));
p->path = (char *)(p+1);
strcpy(p->path, pname);
return(Success);
}


static void free_parts(Dlheader *pparts)
/*
 * Free a doubly linked list of path parts
 */
{
Dlnode *node;

while ((node = get_head(pparts)) != NULL)
	trd_freemem(node);
}

static int letter_to_device(char letter)
/*
 * Convert from ascii to numerical representation of device
 */
{
if (letter == TRD_CHAR)
	return(TDEV_ID);
else
	return(letter - 'A');
}

static Errcode path_to_parts(char *path, Dlheader *pparts)
/*
 * Chop up a string of multiple directories separated by semicolons
 * into a doubly linked list of "path_parts".
 */
{
Path_part *newpart;
char pbuf[PATH_SIZE*2];
char device[4];
char dir[PATH_SIZE];
Errcode err = Success;


init_list(pparts);
while (parse_to_semi(&path, pbuf,sizeof(pbuf)))
	{
	if ((err = split_path(pbuf, device, dir)) < Success)
		goto BADOUT;
	sprintf(pbuf, "%s%s", device, dir);
	if ((err = init_part(&newpart, pbuf)) < Success)
		goto BADOUT;
	newpart->device = letter_to_device(device[0]);
	add_tail(pparts, &newpart->node);
	}
return(Success);
BADOUT:
free_parts(pparts);
return(terr = err);
}

#if defined(__WATCOMC__)
Errcode set_temp_path(char *tp)
/*
 * Input is a semicolon separated list of directories.   This function
 * will assign this list to the temp_path,  and create any directories
 * contained within it that don't already exist.
 */
{
Dlheader pp;
Path_part *p, *next;
Errcode err;
Errcode aerr = Success;
char buf[PATH_SIZE];
int slen;
int goodlen;
int gooduns = 0;

	/* first put ram-device in path */
	sprintf(temp_path, "%c:;", TRD_CHAR);
	path_to_parts(tp, &pp);
	for ( p = (Path_part *)(pp.head);
		(next = (Path_part *)(p->node.next)) != NULL;
		p = next)
	{
		strcpy(buf,p->path);
		slen = strlen(buf);
		if((goodlen = strcspn(buf,".\"/[]|<>+=,?*")) != slen)
		{
			aerr = softerr(Err_bad_input, "!%c%s", "tdev_badchar", 
						   buf[goodlen], buf);
			continue;
		}
		if (slen < 2 || buf[1] != ':')
		{
			aerr = softerr(Err_no_message, "!%s", "tdev_device", buf);
			continue;
		}
		if(pj_pathdev_is_fixed(buf) != 1)
		{
			aerr = softerr(Err_no_message, "!%s", "tdev_floppy", buf);
			continue;
		}
		if (slen < 3 || buf[2] != DIR_DELIM)
		{
			aerr = softerr(Err_no_message, "!%s", "tdev_delim", buf);
			continue;
		}
		if (slen < 4)
		{
			aerr = softerr(Err_no_message, "!%s", "tdev_root", buf);
			continue;
		}
		if (slen > 3 && buf[slen-1] == DIR_DELIM)
			buf[slen-1] = 0;		/* chop off trailing \ */
		/* make directory (if it doesn't exist) */
		err = pj_dmake_dir(buf);
		if (err < Success)
		{
			if (pj_dget_err() != 5)	/* already exists */
			{
				aerr = softerr(err, "!%s", "tdev_exist", buf);
				continue;
			}
		}
		strcat(temp_path, buf);
		strcat(temp_path, ";");
		gooduns+=1;
	}
	if (gooduns < 1)
		return(errline(Err_no_temp_devs, tp));
	free_parts(&pp);
	return(aerr);
}
#else /* __WATCOMC__ */
Errcode set_temp_path(char *tp)
{
	(void)tp;
	return Success;
}
#endif /* __WATCOMC__ */

#ifdef SLUFFED
void get_temp_path_head(char *head)
/*
 * Return the first directory in path (excluding the internal ramdisk
 * into head.
 */
{
char *path = temp_path;

parse_to_semi(&path, head, PATH_SIZE);	/* skip #: */
parse_to_semi(&path, head, PATH_SIZE);
}
#endif /* SLUFFED */

static void make_tname(char *result, const char *name, const char *ppref)
/*
 * Input temp name including device (eg >:aatemp.flx) and
 * path prefix (eg c:\paat\).
 * Output ms-dos name (eg c:\paat\aatemp.flx)
 */
{
sprintf(result, "%s%s", ppref, name+2);
}

static Errcode tfind(const char *name, char *pname)
/*
 * Find a file that's somewhere on the temp-path.  
 * Return ms-dos file-name in pname[]
 */
{
Dlheader pp;
Path_part *p, *next;
Errcode err;

path_to_parts(temp_path, &pp);
for ( p = (Path_part *)(pp.head);
	(next = (Path_part *)(p->node.next)) != NULL;
	p = next)
	{
	make_tname(pname, name, p->path);
	if (pj_exists(pname))
		{
		err = Success;
		goto GOODOUT;
		}
	}
terr = err = Err_no_file;
GOODOUT:
free_parts(&pp);
return(err);
}

static Tfile alloc_tfile(const char *name, const char *ppref)
/*
 * Allocate and initialize a temporary file structure.
 */
{
Tfile tfl;

if ((tfl = trd_askcmem(sizeof(*tfl))) == NULL)
	{
	terr = Err_no_memory;
	return(NULL);
	}
strcpy(tfl->name, name);
strcpy(tfl->path, ppref);
add_head(&tlist, &tfl->tnod);
return(tfl);
}

#if defined(__WATCOMC__)
Tfile tcreate(const char *name, int mode)
/*
 * Create a new temporary file.   If it's a =: file most likely it'll
 * end up in internal ram-disk unless real tight for memory.  Otherwise
 * on first directory in temp path that has room to create a file.
 */
{
Tfile result = NULL;
Jfile handle;
Errcode err;
Path_part *p, *next;
Dlheader pp;
char pname[PATH_SIZE];

tdelete(name);
path_to_parts(temp_path, &pp);
for ( p = (Path_part *)(pp.head);
	(next = (Path_part *)(p->node.next)) != NULL;
	p = next)
	{
			/* Skip the #: device if it's a >: file.  */
	if (p->device == TDEV_ID && name[0] == TDEV_LO)
		continue;
	make_tname(pname, name, p->path);
	if ((handle = pj_create(pname,mode)) != JNONE)
		{
		if ((result = alloc_tfile(name, p->path)) != NULL)
			result->jhandle = handle;
		goto OUT;
		}
	else
		err = pj_ioerr();
	}
terr = err;
OUT:
free_parts(&pp);
return(result);
}
#else /* __WATCOMC__ */
Tfile tcreate(const char *name, int mode)
{
	Tfile result = NULL;
	Jfile handle;
	char pname[PATH_SIZE];

	tdelete(name);

	make_tname(pname, name, "./");
	if ((handle = pj_create(pname, mode)) != JNONE) {
		if ((result = alloc_tfile(name, "./")) != NULL)
			result->jhandle = handle;
		return result;
	}
	return NULL;
}
#endif /* __WATCOMC__ */

#if defined(__WATCOMC__)
Tfile topen(const char *name, int mode)
/*
 * Open a file on the temp device.  This will loop through the temp path
 * checking each directory for the file. 
 */
{
Tfile result = NULL;
Jfile handle;
Errcode err;
Path_part *p, *next;
Dlheader pp;
char pname[PATH_SIZE];

path_to_parts(temp_path, &pp);
p = (Path_part *)(pp.head);
for ( p = (Path_part *)(pp.head);
	(next = (Path_part *)(p->node.next)) != NULL;
	p = next)
	{
	if (p->device == TDEV_ID && name[0] == TDEV_LO)
		continue;
	make_tname(pname, name, p->path);
	if ((handle = pj_open(pname,mode)) != JNONE)
		{
		if ((result = alloc_tfile(name, p->path)) != NULL)
			result->jhandle = handle;
		goto OUT;
		}
	else
		err = pj_ioerr();
	}
terr = err;
OUT:
free_parts(&pp);
return(result);
}
#else /* __WATCOMC__ */
Tfile topen(const char *name, int mode)
{
	Tfile result = NULL;
	char pname[PATH_SIZE];
	Jfile handle;

	make_tname(pname, name, "./");
	if ((handle = pj_open(pname, mode)) != JNONE) {
		if ((result = alloc_tfile(name, "./")) != NULL)
			result->jhandle = handle;
		return result;
	}
	return NULL;
}
#endif /* __WATCOMC__ */

Errcode tclose(Tfile t)
/*
 * Close a temporary file.
 */
{
Errcode err;

if (t == NULL)
	return(terr = Err_null_ref);
err = pj_close(t->jhandle);
rem_node(&t->tnod);
trd_freemem(t);
return(err);
}

long tread(Tfile t, void *buf, long size)
/*
 * Read a block of data from an open temporary file.
 */
{
	if (t != NULL)
		return pj_read(t->jhandle, buf, size);
	return 0;
}

static Errcode oopen_seek(Jfile *phandle, char *name, int mode, long pos)
/*
 * Open a file and seek to a particular position.
 */
{
long serr;

if ((*phandle = pj_open(name, mode)) == JNONE)
	return(pj_ioerr());
if ((serr = pj_seek(*phandle, pos, JSEEK_START)) < Success)
	return(serr);
return(Success);
}

static long tfree(int dev)
/*
 * Return the amount of memory left on a device.
 */
{
long free;

if (dev == TDEV_ID)
	free = rdos_dfree();
else
	free = pj_ddfree(dev+1);
return(free);
}


static Errcode copy_tfile(char *opname, char *pname, Tfile t, long space,
	Boolean substitute_result)
/* Close down file and copy it somewhere where there's room for it plus
 * some extra space.  On failure leave things as they were.  On Success
 * if substitute_result is true reattatch temp file to copy and delete
 * old one.
 */
{
Errcode err, cerr;
Path_part *p, *next;
Dlheader pp;
long fsize;
long new_size;
int cur_dev;
long pj_ddfree;

make_tname(opname, t->name, t->path);
t->omode = get_jmode(t->jhandle);
path_to_parts(temp_path, &pp);
t->opos = tseek(t, 0L, JSEEK_REL);
fsize = tseek(t, 0L, JSEEK_END);
new_size = t->opos+space;
if (new_size < fsize)
	new_size = fsize;
cur_dev = letter_to_device(t->path[0]);
for ( p = (Path_part *)(pp.head);
	(next = (Path_part *)(p->node.next)) != NULL;
	p = next)
	{
	if (p->device == TDEV_ID && t->name[0] == TDEV_LO)
		continue;
	if (p->device != cur_dev)
		{
		if ((pj_ddfree = tfree(p->device)) >= new_size)
			{
			make_tname(pname, t->name, p->path);
			soft_put_wait_box("!%s%s", "tfile_move", opname, pname );
			pj_close(t->jhandle);
			if ((err = cerr = pj_copyfile(opname, pname)) < Success || 
				!substitute_result)
				{
				if ((err = oopen_seek(&t->jhandle, opname,t->omode,t->opos))
							< Success)
					cerr = Err_swap;
				if (cerr < Success)
					{
					pj_delete(pname);
					continue;
					}
				}
			else
				{
				pj_delete(opname);
				strcpy(t->path, p->path);
				if ((err = oopen_seek(&t->jhandle, pname,t->omode,t->opos))
					< Success)
					err = Err_swap;
				}
			cleanup_wait_box();
			goto OUT;
			}
		}
	}
terr = err = Err_no_temp_space;
OUT:
free_parts(&pp);
return(err);
}

static Errcode move_tfile(Tfile t, long space)
/*
 * Attempt to move a tfile requiring space more bytes than it has to
 * another device. 
 */
{
char opname[PATH_SIZE];
char pname[PATH_SIZE];

return(copy_tfile(opname, pname, t, space, TRUE));
}

long twrite(Tfile t, void *buf, long size)
/*
 * Write a block of data to an open temporary file.
 * First write it to current underlying file.   If the write fails
 * then try swapping file to new device.  If swap fails return
 * length that did manage to get written.
 */
{
long len;
long left;

	len = pj_write(t->jhandle, buf, size);
	if (len < size)
	{
		left = size-len;
		if (move_tfile(t,left) < Success)
			return(len);
		else
			return(len+twrite(t, OPTR(buf,len), left));
	}
	return(len);
}

long tseek(Tfile t, long offset, int mode)
/*
 * Seek to a position in a temporary file
 */
{
return(pj_seek(t->jhandle,offset,mode));
}

long ttell(Tfile t)
/* 
 * Query current position in a temporary file
 */
{
return(pj_tell(t->jhandle));
}

Errcode tdelete(const char *name)
/*
 * Delete a temporary file.
 */
{
char pbuf[PATH_SIZE];
Errcode err;

if ((err = tfind(name, pbuf)) < Success)
	{
	return(err);
	}
if ((err = pj_delete(pbuf)) < Success)
	{
	terr = err;
	}
return(err);
}

Errcode terror(void)
/*
 * Return last temporary file error.
 */
{
return(terr);
}

Errcode trename(const char *old, const char *new)
/*
 * Rename a temporary file.
 */
{
char obuf[PATH_SIZE];
char fobuf[FILE_NAME_SIZE];
char pbuf[PATH_SIZE], nbuf[PATH_SIZE];
Errcode err;

if ((err = tfind(old, obuf)) < Success)
	return(err);
split_copy_path(obuf, pbuf, fobuf);
make_tname(nbuf, new, pbuf);
return(pj_rename(obuf, nbuf));
}


static Tfile lookup_tfile(char *name, Dlheader *list)
/*
 * Find an (open) temporary file with the given name.
 */
{
Dlnode *node;
Tfile tf;

for(node = list->head;
	NULL != node->next;
	node = node->next)
	{
	tf = (Tfile)node;
	if (txtcmp(tf->name, name) == 0)
		return(tf);
	}
return(NULL);
}

static Errcode swap_open_rfile(char *name)
/*
 * Move an open temporary file from ram to disk.
 */
{
Tfile tf;

if ((tf = lookup_tfile(name, &tlist)) == NULL)
	return(Err_no_file);
return(move_tfile(tf,0L));
}

static Errcode swap_closed_rfile(char *rname)
/*
 * Move a closed temporary file from ram to disk.
 */
{
Tfile tf;
Errcode err;

if ((tf = topen(rname, JREADONLY)) == NULL)
	return(terror());
err = move_tfile(tf,0L);
tclose(tf);
return(err);
}

static Errcode try_rcompact(long need_free)
/*
 * Compact (free unused parts of) ram-disk,  and then temporarily
 * allocate "need_free" bytes.   Return whether allocation was
 * successful.
 */
{
void *pt;

rcompact();
if (need_free <= 0)
	return(Success);
if ((pt = trd_laskmem(need_free)) != NULL)
	{
	trd_freemem(pt);
	return(Success);
	}
return(Err_no_memory);
}

static Errcode swap_either(Rdir *cless)
/*
 * Swap out a temporary file in ram that is either open or closed.
 */
{
char tname[PATH_SIZE];

strcpy(tname, cless->name);
tname[0] = TDEV_MED;
if (cless->open)
	return(swap_open_rfile(tname));
else
	return(swap_closed_rfile(tname));
}

static Errcode swap_either_compact(Rdir *cless, long need_free)
/*
 * Swap out a file from ram, and then return if there's "need_free"
 * memory that can be allocated after swap.
 */
{
Errcode err;

if ((err=swap_either(cless)) < Success)
	return(err);
return(try_rcompact(need_free));
}

Errcode trd_compact(long need_free)
/*
 * Swap temporary files to disk until can allocate a block of
 * memory "need_free" bytes long.
 */
{
Errcode err;
Rdir *rdir, *rn;
Rdir *cless;
long csize, rsize;
long talloc,tfree;

	/* no recursion beyond this point or we fail !!!! */
	if(tfile_lockmem)
		return(Err_no_memory);

	++tfile_lockmem;

	/* First see if there's any hope of satisfying this request. */
	rstats(&talloc, &tfree);	/* ask ram-disk how much memory it's using */
	if(talloc < need_free) 
		return(Err_no_memory); /* Never had that much memory to begin with */


	/* If can compact ram disk without moving any files it's easy! */
	if((err = try_rcompact(need_free)) == Success)
		goto pop_recurs;

	/* See if we can swap a single file and get the space we need.
	 * Search through directory to find smallest file bigger than the space
	 * we need */

	rget_dir(&rdir);	/* Get directory of ram-disk */
	cless = NULL;
	csize = 0x7fffffff;
	for (rn = rdir; rn != NULL; rn = rn->next)
	{
		rsize = rn->size;
		if (rsize < csize && rsize >= need_free)
		{
			cless = rn;
			csize = rsize;
		}
	}
	/* Aha - we've got the file. */
	if (cless != NULL)
	{
		err = swap_either_compact(cless, need_free);
		if (err == Err_swap || err == Success)
			goto OUT;
	}
	/* Well, if we've made it here we couldn't find the one file that would
	 * do the job.  So go through and swap out everything until there is
	 * enough room.
	 */
	rfree_dir(&rdir);  /* Rescan directory in case we swapped one that _looked_
					    * big enough but due to sector stuff wasn't. */
	rget_dir(&rdir);
	for (rn = rdir; rn != NULL; rn = rn->next)
	{
		err = swap_either_compact(rn, need_free);
		if (err == Err_swap || err == Success)
			goto OUT;
	}
	err = Err_no_memory;
OUT:
	rfree_dir(&rdir);
pop_recurs:
	--tfile_lockmem;
	return(err);
}

Errcode trd_ram_to_files(void)
/* 
 * force ram-located temp files onto ms-dos 
 */
{
Rdir *rdir, *rn;
Errcode err = Success;

rget_dir(&rdir);
for (rn = rdir; rn != NULL; rn = rn->next)
	{
	if ((err = swap_either(rn)) < Success)
		break;
	}
rfree_dir(&rdir);
return(err);
}

Errcode trd_up_to_ram(char *name)
/* 
 * If possible move a temp file up to ram. 
 * (Assumes temp file is closed.) 
 */
{
char pname[PATH_SIZE];
char tname[PATH_SIZE];
Errcode err;
char *errin;
LONG size;
LONG rfree;

	if ((err = tfind(name, pname)) < Success)
		return(err);
	if (pname[0] == TRD_CHAR)
		return(Success);
	strcpy(tname, name);
	tname[0] = TRD_CHAR;
	size = pj_file_size(pname);
	if (size >= 0 && size < (rfree = rdos_dfree()))
	{
		soft_put_wait_box("!%s%ld%ld", "tfile_toram", pname, size, rfree );

		if ((err = pj_cpfile(pname,tname,&errin)) == Success)
			pj_delete(pname);
		else
		{
			pj_delete(tname);
		}
	}
	return(err);
}

#ifdef SLUFFED
Errcode trd_move_to_ram(char *name, Jfile *pf)
/*
 * same as trd_up_to_ram but requires file handle, If file handle is 
 * JNONE it will leave file closed.  If file handle is not JNONE it may 
 * alter *pf and move file to ram,  It is possible that this may leave the 
 * file closed in a fatal error situation 
 */
{
Jfile fd;
int omode;
LONG ooset;

	fd = *pf;
	if(fd != JNONE)
	{
		ooset = pj_tell(fd);
		omode = get_jmode(fd);
		pj_close(fd);
	}
	trd_up_to_ram(name);
	if(fd != JNONE)
	{
		if((*pf = pj_open(name,omode)) == JNONE)
			return(pj_ioerr());
		pj_seek(*pf,ooset,JSEEK_START);
	}
	return(Success);
}
#endif /* SLUFFED */

static Boolean is_a_part(Dlheader *list, char *ppart)
/* Return TRUE if ppart is one of the paths in the path-parts-list. */
{
Path_part *p, *next;

for ( p = (Path_part *)(list->head);
	(next = (Path_part *)(p->node.next)) != NULL;
	p = next)
	{
	if (txtcmp(p->path,ppart) == 0)
		return(TRUE);
	}
return(FALSE);
}


static void delete_files(Names *list)
/* Delete all files named in a singly linked list. */
{
while (list != NULL)
	{
	pj_delete(list->name);
	list = list->next;
	}
}

static Errcode copy_to_new_path(char *old_path, char *new_path)
/* Copy all the files in the old-path-directories to new-path-directories. */
{
Dlheader old_parts, new_parts;
Errcode err = Success;
Names *copied_list = NULL;
Names *old_list = NULL;
Names *name1;
Dlnode *node;
Tfile tf;				 /* Temp file handle */
char opname[PATH_SIZE];	 /* Full path name of temp file */
char npname[PATH_SIZE];	 /* Full path name of temp file */
char *errname;
Path_part *pp, *ppnext;
Names *wild_list = NULL;
Names *wild_pt;



++tfile_lockmem;	/* don't want to swap anything out now. */

/* Break old and new path list into their component directories. */
init_list(&old_parts);
init_list(&new_parts);
path_to_parts(old_path, &old_parts);
path_to_parts(new_path, &new_parts);


/* Go through open temp files and make copies of them in new path */
for(node = tlist.head;
	NULL != node->next;
	node = node->next)
	{
	tf = (Tfile)node;
	if ((tf->omoved = !is_a_part(&new_parts, tf->path)) != 0)
		{
		/* Get name of temp file's actual place */
		if ((err = copy_tfile(opname, npname, tf, 0, FALSE)) < Success)
			goto OUT;
		if ((err = new_name(&name1, npname, &copied_list)) < Success)
			goto OUT;
		if ((err = new_name(&name1, opname, &old_list)) < Success)
			goto OUT;
		}
	}

/* Move closed temp files */
for ( pp = (Path_part *)(old_parts.head);
	(ppnext = (Path_part *)(pp->node.next)) != NULL;
	pp = ppnext)
	{
	if (!is_a_part(&new_parts, pp->path))
		{
		if ((err = build_dir_list(&wild_list, "*.*", FALSE, pp->path)) 
			< Success)
			goto OUT;
		for (wild_pt = wild_list; wild_pt != NULL; wild_pt = wild_pt->next)
			{
			sprintf(opname, "%s%s", pp->path, wild_pt->name);
			if (name_in_list(opname, old_list) == NULL)
				{
				sprintf(npname, "<:%s", wild_pt->name);
				soft_put_wait_box("!%s%s", "tfile_move", opname, new_path );
				if ((err = pj_cpfile(opname, npname, &errname)) < Success)
					goto OUT;
				if ((err = new_name(&name1, npname, &copied_list)) < Success)
					goto OUT;
				if ((err = new_name(&name1, opname, &old_list)) < Success)
					goto OUT;
				}
			}
		free_wild_list(&wild_list);
		}
	}

/* Go through open temp files and attatch them to copies */
for(node = tlist.head;
	NULL != node->next;
	node = node->next)
	{
	tf = (Tfile)node;
	if (tf->omoved)
		{
		tfind(tf->name, npname);
		pj_close(tf->jhandle);
		if ((err = oopen_seek(&tf->jhandle, npname,tf->omode,tf->opos))
			< Success)
			err = Err_swap;
		split_copy_path(npname, tf->path, opname /* ignored */);
		}
	}

OUT:
	{
	free_wild_list(&wild_list);
	free_parts(&old_parts);
	free_parts(&new_parts);
	if (err < Success)
		delete_files(copied_list);
	else
		delete_files(old_list);
	free_slist((Slnode *)copied_list);
	free_slist((Slnode *)old_list);
	--tfile_lockmem;	/* reenable low-memory swapping. */
	return(softerr(err, "!%s", "temp_copy", new_path));
	}
}

Errcode change_temp_path(char *new_path)
{
Errcode err;
char opath[2*PATH_SIZE] = {TRD_CHAR, ':', 0};

strcpy(opath, temp_path);
if ((err = set_temp_path(new_path)) < Success)
	{
	set_temp_path(opath+3);
	return(err);
	}
if ((err = copy_to_new_path(opath,temp_path)) < Success)
	{
	set_temp_path(opath+3);
	}
return(err);
}

