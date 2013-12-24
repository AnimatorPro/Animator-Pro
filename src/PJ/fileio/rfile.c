/** rfile.c - an internal ram-disk that can be shrunk to fit only the
 ** parts that are in use.   This is the implementation of the #: device.
 **
 ** These are the major parts to this file:
 **		o - Setting the maximum amount of memory the ramdisk is allowed
 **			to use and finding how much free space is left on ramdisk
 **			given these and memory constraints.
 **		o - Implementation of the ramdisk including the file operations
 **			open, close, read, write, seek, delete, rename.
 **		o - Compacting the ramdisk.  That is rearranging the sections of
 **			RAM that it uses to a minimum number of blocks,  and freeing
 **			the remaining blocks so that other systems may use them.
 **			(Currently a block is 256K.)
 **		o - Providing a directory of files in the ram-disk.  (This part
 **			is pretty easy.)
 **/

#include <stdio.h>
#include "stdtypes.h"
#include "ptrmacro.h"
#include "jfile.h"
#include "memory.h"
#include "linklist.h"
#define RFILE_C
#include "rfile.h"
#include "errcodes.h"

void lo_freemem(long *pt);
void *lo_askmem(long nbytes);


/*********** Basic constants, data structures, and static variables **********/

/* Temp ram-disk constants */
/* sector size as a power of 2 */
#define TRD_SSHIFT 12
#define TRD_BLOCK 64	/* # of  sectors to alloc at once */
/* TRD_SSHIFT values with TRD_BLOCK at 64
		 ==  8 sector size =  256 max disk size   1 Meg. Block size  16K 
		 ==  9 sector size =  512 max disk size   8 Meg. Block size  32K
		 == 10 sector size = 1024 max disk size  64 Meg. Block size  64K
		 == 11 sector size = 2048 max disk size 512 Meg. Block size 128K
		 == 12 sector size = 4096 max disk size   4 Gig. Block size 256K
   This has been tested with TRD_SSHIFT as low as 6.  With a 16 bit
   int size TRD_SSHIFT should go no higher than 11.  With a 32
   bit int & pointer size 12 is max. */

	/* # of pointers a sector can hold as a power of 2 */
#define TRD_PSHIFT (TRD_SSHIFT-2)		/* assume sizeof(void *) == 4 */
	/* size of data block */
#define TRD_SECTOR (1<<TRD_SSHIFT)
	/* # of blocks in logical track */
#define TRD_TRACK  (TRD_SECTOR/sizeof(void *))
	/* # of tracks in logical platter */
#define TRD_PLATTER (TRD_SECTOR/sizeof(void *))

/* actual size of block allocated in bytes */
#define TRD_BLOCK_BYTES ((long)TRD_BLOCK*(long)TRD_SECTOR)

/* estimate of size of actual data space available in a track and block */ 
#define TRD_SECTOR_DATA (TRD_SECTOR-sizeof(void*))   
#define TRD_BLOCK_DATA ((TRD_BLOCK*TRD_SECTOR_DATA)-(TRD_BLOCK*sizeof(void *)))

/* Each ram disk file is made up of platters which are composed of tracks which
 * are composed of sectors.   Not being a physical device but just a ram
 * disk,  we can do simple arithmetic to find out the platter, track, and
 * sector given an absolute offset into the file.
 */
#define get_platter(offset) ((int)((offset)>>(TRD_SSHIFT+TRD_PSHIFT)))
#define get_track(offset) ((int)(((offset)>>(TRD_SSHIFT))&((1<<TRD_PSHIFT)-1)))
#define get_sector(offset) ((int)((offset)&((1<<TRD_SSHIFT)-1)))


#define TFNAME_SIZE 16		/* The max length of a file name. */

#define MAX_TEMPS 32		/* maximum number of temp  files */
typedef struct temp_file
/* This is our very simple minded directory structure and ramdisk file
 * handle.  It's statically allocated.  To find a 
 * free directory entry for a new file, we just linearly
 * scan the directory array for an entry without the TF_USED 
 * bit set in the flags field. */
	{
	char ***platters;
	long filep;
	long size;
	char name[TFNAME_SIZE];
	short flags;
	short omode;
	} Temp_file;
/* defines  for temp_file->flags */
#define TF_READ 1
#define TF_WRITE 2
#define TF_OPEN 4
#define TF_USED 8
#define TF_USABLE (TF_OPEN|TF_USED)
static Temp_file glo_tpf[MAX_TEMPS];	/* Directory array */

typedef struct sector_list
/* Structure to keep the list of free sectors. */
	{
	struct sector_list *next;
	} Sector_list;
static Sector_list *tfree_list;	 /* Linked list of free sectors. */
static long talloc_size; /* Number of sectors allocated */
static long tfree_size;	 /* Number of sectors currently free */

typedef  struct block_list
/* Structure to keep the list of blocks allocated to ram-disk */
	{
	struct block_list *next;
	} Block_list;

static int rerr;		/* stores last error code */
static Block_list *tblock_list;	 /* Linked list of blocks allocated. */
static int tblock_count;		/* Count of blocks allocated. */

#define MAXUSE TRD_PLATTER

#ifdef TOTHEMAX
	static long tmax_blocks = MAXUSE;
#else
	static long tmax_blocks = 2000000000L/TRD_BLOCK_BYTES;
#endif /* TOTHEMAX */

/************ Stuff for finding how much free space in ram disk *************
			  and setting how much ram to use */

void rstats(long *alloc, long *free)
/* Find out the number of bytes allocated to ram-disk, and the amount
 * currently in use. */
{
	*alloc = talloc_size*TRD_SECTOR;
	*free = tfree_size*TRD_SECTOR;
}


static int new_blocks_avail(int max)
/* See how many blocks we can allocate up to max then free them.  Return
 * the # of blocks we allocated. */
{
int bcount = 0;
Block_list *blist, *bl;

	blist = NULL;
	while(--max >= 0)
	{
		if((bl = trd_laskmem(sizeof(*bl)+TRD_BLOCK_BYTES)) == NULL)
			break;
		bl->next = blist;
		blist = bl;
		++bcount;
	}
	while((bl = blist) != NULL)
	{
		blist = bl->next;
		trd_freemem(bl);
	}
	return(bcount);
}

void rdisk_set_max_to_avail()
/* set to max ram available for disk */
{
	tmax_blocks = tblock_count + new_blocks_avail(MAXUSE-tblock_count);
}

static long div_next_up(long p, long q)
/* This returns the next integer higher or equal to p/q */
{
long result = p/q;
if (p%q != 0)
	result += 1;
return(result);
}

long rdos_dfree()
/* Hopefully returns amount of ram disk available for a new file. */
{
long sectors_avail;

		/* get count of potentially available sectors */
	sectors_avail = (tmax_blocks-tblock_count)*TRD_BLOCK + tfree_size;
		/* One sector is used for the pointers to platters  array */
	sectors_avail -= 1;	
		/* Subtract number of platters we could possibly fill */
	sectors_avail -= div_next_up(sectors_avail, TRD_PLATTER*TRD_TRACK);
		/* Subtract number of tracks we could possibly fill */
	sectors_avail -= div_next_up(sectors_avail, TRD_TRACK);
	return(sectors_avail*TRD_SECTOR);
}

/***************** Ramdisk read/write/open/close routines ******************/

Errcode rerror()
/* Return the last ram-disk error */
{
return(rerr);
}

static tfree_sector(void *v)
/* Add a sector to the free list */
{
Sector_list *sl;

tfree_size += 1;
sl = v;
sl->next = tfree_list;
tfree_list = sl;
}

static Boolean tmore_sectors()
/* Allocate a block of memory,  divide it into sectors, and put the
 * sectors on the free list.   
 */
{
Block_list *bl;
char *pt;
int i;

	if(tblock_count >= tmax_blocks)		/* Don't exceed maximum # of blocks */
		goto outta_space;
	if ((bl = trd_laskmem(sizeof(*bl)+TRD_BLOCK_BYTES)) == NULL)
		goto outta_space;

	bl->next = tblock_list;
	tblock_list = bl;
	tblock_count+=1;
	pt = (char *)(bl+1);
	talloc_size += TRD_BLOCK;
	i = TRD_BLOCK;
	while (--i >= 0)
	{
		tfree_sector(pt);
		pt += TRD_SECTOR;
	}
	return(Success);
outta_space:
	return(rerr = Err_no_memory);
}

static void *tget_sector()
/* Return an unused sector.   Search free list first.  If empty allocate
 * another block.   If block alloc fails return NULL */
{
Sector_list *sl;

if ((sl = tfree_list) != NULL)
	{
	tfree_list = sl->next;
	tfree_size -= 1;
	}
else
	{
	if (tmore_sectors()>=Success)
		sl  = tget_sector();
	}
return(sl);
}

static void *tget_clear()
/* Return an unused sector initialized to all zeroes. */
{
void *pt;

if ((pt = tget_sector()) != NULL)
	clear_mem(pt, TRD_SECTOR);
return(pt);
}

long rwrite(Rfile tpf, void *buf, long count)
/* Write a buffer to an open ram-disk file. */
{
char ***platter;
char **track;
char *data;
int i;
int ssize, so;
int flags;
long filept;
long  written;
long new_size;

flags = tpf->flags;
if (!(flags&TF_OPEN))
	{
	rerr = Err_file_not_open;
	return(0);
	}
if (!(flags&TF_WRITE))
	{
	rerr = Err_write;
	return(0);
	}
if (count <= 0)
	return(0);

	data = buf;
	written = 0;
	filept = tpf->filep;
	new_size = filept + count;
	platter = tpf->platters + get_platter(filept);

	for (;;)
	{
		if ((track = *platter) == NULL)
			if ((*platter = track = tget_clear()) == NULL)
				goto OUT;
		i = get_track(filept);
		track += i;
		for ( ; i< TRD_TRACK; i++)
		{
			if (*track == NULL)
				if ((*track = tget_sector()) == NULL)
					goto OUT;
			so = get_sector(filept);
			ssize = TRD_SECTOR - so;
			if (count > ssize)
			{
				memcpy(*track + so, data, ssize);
				written += ssize;
				data +=  ssize;
				count -= ssize;
				filept += ssize;
			}
			else
			{
				memcpy(*track + so, data, (size_t)count);
				written += count;
				filept += count;
				goto OUT;
			}
			track++;
		}
		platter++;
	}
OUT:
	if (tpf->size < (tpf->filep = filept))
		tpf->size = filept;
	return(written);
}

long rread(Rfile tpf, void *buf, long count)
/* Read a buffer from an open ram-disk file. */
{
char ***platter;
char **track;
char *data;
int i;
int ssize, so;
int flags;
long filept;
long  readin;
long fsize;

flags = tpf->flags;
if (!(flags&TF_OPEN))
	{
	rerr = Err_file_not_open;
	return(0);
	}
if (!(flags&TF_READ))
	{
	rerr = Err_read;
	return(0);
	}
data = buf;
readin = 0;
filept = tpf->filep;
fsize  = tpf->size;
if ((tpf->filep = filept+count) > fsize)
	{
	rerr = Err_eof;
	if ((count = fsize-filept) <= 0)
		return(0);
	}
platter = tpf->platters + get_platter(filept);
for (;;)
	{
	i = get_track(filept);
	track = *platter  +  i;
	for ( ; i< TRD_TRACK; i++)
		{
		so = get_sector(filept);
		ssize = TRD_SECTOR - so;
		if (count > ssize)
			{
			memcpy(data, *track + so, ssize);
			readin += ssize;
			data +=  ssize;
			count -= ssize;
			filept += ssize;
			}
		else
			{
			memcpy(data,  *track + so, (size_t)count);
			return(readin+count);
			}
		track++;
		}
	platter++;
	}
}

static Rfile find_empty_slot()
/* Find an empty directory slot. */
{
int tf;
Temp_file *tpf;

for (tf=0; tf<MAX_TEMPS; tf++)
	if (!(glo_tpf[tf].flags&TF_USED))	/* got a free slot */
		{
		tpf = &glo_tpf[tf];

		if ((tpf->platters = tget_clear()) == NULL)
			return(NULL);
		tpf->filep = tpf->size = 0;
		return(tpf);
		}
rerr = Err_too_many_files;
return(NULL);
}

static Rfile find_named(char *name)
/* Return directory entry of a named file if it exists, NULL otherwise. */
{
int tf;
Temp_file *tpf;

for (tf=0; tf<MAX_TEMPS; tf++)
	{
	tpf = &glo_tpf[tf];
	if (tpf->flags&TF_USED)
		{
		if (txtcmp(tpf->name, name) == 0)
			return(tpf);
		}
	}
rerr = Err_no_file;
return(NULL);
}

Errcode rexists(char *name)
/* Return Success if a file is on the ram-disk. */
{
if (find_named(name) == NULL)
	return(rerr);
return(Success);
}

static int flags_for_mode(int mode)
/* Calculate the temp_file.flags value for a given msdos/jfile file open/create
 * mode. */
{
int flags;
switch (mode&MSOPEN_MODES)
	{
	case JREADONLY:
		flags = TF_USED|TF_OPEN|TF_READ;
		break;
	case JWRITEONLY:
		flags = TF_USED|TF_OPEN|TF_WRITE;
		break;
	case JREADWRITE:
		flags = TF_USED|TF_OPEN|TF_READ|TF_WRITE;
		break;
	default:
		return(rerr = Err_file_access);
	}
return(flags);
}

Rfile ropen(char *name, int mode)
/* Open a file on ram-disk */
{
Temp_file *tpf;
int flags;

if ((flags = flags_for_mode(mode)) < Success)
	return(NULL);

if ((tpf = find_named(name)) != NULL)
	{
	if (tpf->flags & TF_OPEN)
		{
		rerr = Err_in_use;
		return(NULL);
		}
	tpf->filep = 0;
	tpf->flags = flags;
	tpf->omode = mode;
	return(tpf);
	}
if ((flags&TF_READ))
	{
	rerr = Err_no_file;
	return(NULL);
	}
if ((tpf = find_empty_slot()) != NULL)
	{
	strncpy(tpf->name, name, TFNAME_SIZE-1);
	tpf->flags = flags;
	tpf->omode = mode;
	}
return(tpf);
}

long rtell(Rfile tpf)
/* Return current file position on ram-disk. */
{
if (tpf->flags & TF_OPEN)
	return(tpf->filep);
else
	return(rerr = Err_file_not_open);
}

long rseek(Rfile tpf, long offset, int mode)
/* Seek to a position in ram-disk */
{
long filep;

if (!(tpf->flags&TF_OPEN))
	return(rerr=Err_file_not_open);
filep = tpf->filep;
switch (mode)
	{
	case JSEEK_START:
		filep = offset;
		break;
	case JSEEK_REL:
		filep += offset;
		break;
	case JSEEK_END:
		filep = tpf->size-offset;
		break;
	default:
		return(rerr = Err_file_access);
	}
if (filep < 0 || filep > tpf->size)
	return(rerr = Err_seek);
return(tpf->filep = filep);
}

Rfile rcreate(char *name,int mode)
/* Create a file on ram-disk. */
{
Temp_file *tpf;
int flags;

if ((flags = flags_for_mode(mode)) < Success)
	return(NULL);
if ((tpf = find_named(name)) != NULL)
	if (rdelete(name) < Success)
		return(NULL);
if ((tpf  = find_empty_slot()) == NULL)
	return(NULL);
strncpy(tpf->name, name, TFNAME_SIZE-1);
tpf->flags = flags;
tpf->omode = mode;
return(tpf);
}

rclose(Rfile tpf)
/* Close an open ram-disk file */
{
if (!(tpf->flags&TF_OPEN))
	return(rerr=Err_file_not_open);
tpf->flags &= ~(TF_OPEN|TF_READ|TF_WRITE);
return(Success);
}

rdelete(char *name)
/* Delete a (closed) file from ram-disk. */
{
Temp_file *tpf;
char ***platters;
char **tracks;
char *sectors;
int i;

if ((tpf = find_named(name)) == NULL)
	return(NULL);
if (tpf->flags & TF_OPEN)
	{
	return(rerr = Err_in_use);
	}
platters = tpf->platters;
while ((tracks = *platters) != NULL)
	{
	i = TRD_TRACK;
	while (--i >= 0 && (sectors = *tracks++) != NULL)
		tfree_sector(sectors);
	tfree_sector(*platters++);
	}
tfree_sector(tpf->platters);
tpf->flags = 0;
return(Success);
}

Errcode rrename(char *old, char *new)
/* Rename a file on ram-disk */
{
Temp_file *tpf;

if ((tpf = find_named(new)) != NULL)
	return(rerr=Err_extant);
if ((tpf =  find_named(old))  == NULL)
	return(rerr);
strncpy(tpf->name, new, TFNAME_SIZE-1);
return(Success);
}

/******************** Ramdisk compaction routines ********************/

#ifdef __TURBOC__
extern long i86_ptr_to_long(void *pt);
#define pt_to_long(pt) i86_ptr_to_long(pt)
#else
#define pt_to_long(pt) ((long)(pt))
#endif /* __TURBOC__ */

typedef struct tcs_block
/* This structure is used in rearranging things so as to compact the
 * ramdisk (push all used sectors onto a minimum number of allocated
 * blocks.) */
	{
	void *block;
	Sector_list *frees;
	} Tcs_block;

static int tcs_cmp(Tcs_block *a, Tcs_block *b)
/* See if the block pointer is lower or higher in memory than
 * the other block pointer.  Used in compacting ramdisk. */
{
long diff;

diff = pt_to_long(a->block) - pt_to_long(b->block);
if (diff < 0)
	return(-1);
else if (diff  > 0)
	return(1);
else
	return(0);
}

static void attatch_frees(Tcs_block *tcs, Sector_list **pfree)
/* Move free sectors within block from *pfree to "frees" list of tcs.
 * This is called repeatedly for blocks that are in ascending order
 * in memory.
 * This is one stage of compacting the ram-disk. */
{
Sector_list  *sl, *next;
Sector_list *nlist = NULL;	/* List of things left in *pfree */
Sector_list *tlist = NULL;	/* List of things moved to tcs->frees */
void *end_block;			/* Last piece of memory in block */

tlist = nlist = NULL;
end_block = OPTR(tcs->block,TRD_BLOCK_BYTES);
next = *pfree;
while ((sl=next) != NULL)
	{
	next = sl->next;
	if (pt_to_long(sl) < pt_to_long(end_block))
		{
		sl->next  = tlist;
		tlist = sl;
		}
	else
		{
		sl->next = nlist;
		nlist = sl;
		}
	}
tcs->frees = tlist;
*pfree = nlist;
}

static void merge_frees(Tcs_block *tcs, Sector_list **pfree)
/* Append tcs->frees to end of list in *pfree.
 * This is one stage of compacting the ram-disk. */
{
Sector_list  *next;
Sector_list *nlist, *tlist;

if ((tlist = tcs->frees) == NULL)
	return;
if ((nlist  = *pfree) == NULL)
	{
	*pfree = tlist;
	return;
	}
/* seek to end of merged free list */
while ((next = nlist->next) != NULL)
	nlist = next;
nlist->next = tlist;
}

static Sector_list *next_frees(Tcs_block **ptsc)
/* Return next free sector from an array of Tcs_blocks.  
 * This sector will be detatched from the tsc->frees list
 * that it is on.
 * This is one stage of compacting the ram-disk. */
{
Tcs_block *tsc;
Sector_list *sl;

tsc = *ptsc;
for (;;)
	{
	if ((sl = tsc->frees)  !=  NULL)
		{
		tsc->frees = sl->next;
		break;
		}
	tsc += 1;
	*ptsc = tsc;
	}
return(sl);
}

static void move_sec(Tcs_block **ptsc, void **psec, long cut_at)
/* If sector is at least as high as cut_at in memory,  move it to
 * a free sector in lower memory. 
 * This is one stage of compacting the ram-disk. */
{
Sector_list *newsec;

if (pt_to_long(*psec) >= cut_at)
	{
	newsec = next_frees(ptsc);
	memcpy(newsec, *psec, TRD_SECTOR);
	*psec = newsec;
	}
}


static void tsq_one(
	Tcs_block **tcs, int bcount, 		/* alloced block lists */
	long cut_at,						/* # of blocks needed */
	Temp_file *tpf)						/* temp file */
/* Move all the sectors used by one file to be below "cut_at" in
 * high memory.
 * This is one stage of compacting the ram-disk. */
{
char ***platter;
char **track;
int i;

move_sec(tcs, (void **)(&tpf->platters), cut_at);
platter = tpf->platters;
for (;;)
	{
	if (*platter == NULL)
		break;
	move_sec(tcs, (void **)platter, cut_at);
	track = *platter;
	i = TRD_TRACK;
	for (;;)
		{
		if (--i <= 0)
			break;
		if (*track == NULL)
			break;
		move_sec(tcs, (void **)track, cut_at);
		track += 1;
		}
	platter += 1;
	}
}

static void tsqueeze(
	Tcs_block *tcs, int bcount, 		/* alloced block lists */
	int needed,							/* # of blocks needed */
	Temp_file *gtpf,					/* temp file dir */
	int tpf_count)						/* temp file dir count */
/* Having figured out how many sectors are needed to hold ram-disk,
 * move sectors used by each file into the lower memory blocks. */
{
long cut_at;

cut_at = pt_to_long(tcs[needed].block);		/* Cut at start of first block
											 * that's not needed */
while (--tpf_count >= 0)		/* Loop though all the files in dir */
	{
	if (gtpf->flags & TF_USED)	/* If file is used */
		{
		tsq_one(&tcs, bcount, cut_at, gtpf);	/* Squeeze it. */
		}
	gtpf += 1;					/* Go to next file. */
	}
}

Errcode rcompact()
/* Master routine to compact the ram-disk.  This will move files into
 * the lower blocks in memory,  and free unused blocks in high memory. */
{
Tcs_block *tcs, *tcnext;
Block_list *blist, *bl;
long i;
int blocks_needed;

/* Count up the sectors used, and use that to determine how many blocks
 * are needed by ram-disk */
blocks_needed =  (talloc_size - tfree_size + TRD_BLOCK - 1)/TRD_BLOCK;

if (blocks_needed >= tblock_count)	/* No compaction possible, using all. */
	return(Success);

 /* Allocate an array of control structures for compaction process, one
  * for each block */
if ((tcnext = tcs = trd_laskmem(tblock_count*sizeof(*tcs))) == NULL)
	return(Err_no_memory);
 /* Loop down the block list initializing the corresponding control array 
  * element's block pointers and setting the local free lists to empty. */
bl = tblock_list;
while (bl != NULL)
	{
	tcnext->block = bl;
	tcnext->frees = NULL;
	tcnext++;
	bl = bl->next;
	}
/* Sort the control array based on the address of the block, so that
 * low blocks come first. */
qsort(tcs, (int)tblock_count,  sizeof(tcs[0]), tcs_cmp);
/* Move the free sectors from the global free list to the free list
 * for the block they are part of. */
tcnext = tcs;
i = tblock_count;
while (--i >= 0)
	{
	attatch_frees(tcnext, &tfree_list);
	tcnext += 1;
	}
/* All this preparation done, call routine that will move files to
 * lower memory blocks. */
tsqueeze(tcs,tblock_count,blocks_needed,glo_tpf, MAX_TEMPS);
/* Loop through the blocks putting the local free-lists back onto the
 * global free list for the blocks that we keep,  and freeing the blocks
 * that are no longer used. */
tcnext = tcs;
blist = NULL;
for (i=0; i<tblock_count; i++)
	{
	if (i >= blocks_needed)
		{
		trd_freemem(tcnext->block);
		talloc_size -= TRD_BLOCK;
		tfree_size -= TRD_BLOCK;
		}
	else
		{
		merge_frees(tcnext, &tfree_list);
		bl = tcnext->block;
		bl->next = blist;
		blist = bl;
		}
	tcnext += 1;
	}
tblock_list = blist;
tblock_count = blocks_needed;
trd_freemem(tcs);				/* Free control structure */
return(Success);
}

/*************** Ramdisk directory routines *****************************/

#ifdef DEBUG
tprint_dir()
{
int i;

for (i=0; i<MAX_TEMPS; i++)
	{
	if (glo_tpf[i].flags&TF_USED)
		printf("%s\t%ld\n", glo_tpf[i].name, glo_tpf[i].size);
	}
}
#endif /* DEBUG */


Errcode rget_dir(Rdir **plist)
/* Return directory of ram-disk (file-names only) in a singly linked
 * list. */
{
Rdir *next;
int len;
char *name;
int i;
Temp_file *tpf;

*plist = NULL;
for (i=0; i<MAX_TEMPS; i++)
	{
	tpf = &glo_tpf[i];
	if (tpf->flags&TF_USED)
		{
		name = tpf->name;
		len = sizeof(*next) + strlen(name) + 1;
		if ((next = trd_askmem(len)) == NULL)
			return(Err_no_memory);
		next->name = (char *)(next+1);
		strcpy(next->name, name);
		next->size = tpf->size;
		next->open = ((tpf->flags&TF_OPEN) != 0);
		next->next = *plist;
		next->omode = tpf->omode;
		*plist = next;
		}
	}
return(Success);
}

void rfree_dir(Names **pdir)
/* Free up directory returned by rget_dir() */
{
Names *l, *n;

l = *pdir;
while (l != NULL)
	{
	n = l->next;
	trd_freemem(l);
	l = n;
	}
*pdir = NULL;
}

