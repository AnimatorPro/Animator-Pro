
/* memory.c - Heap maintainer.  Gets memory straight from ms-dos.  Has
   problems with some Turbo C library routines (like fwrite...).  Does
   handle allocations greater than 64K, and does some simple error checking
   during free. */

#include <stdio.h>
#include <stdlib.h>
#include "jimk.h"
#include "memory.h"
#include "memory.str"
#include "peekpok_.h"
#include "ptr.h"

#if defined(__TURBOC__)
#define USE_MEMORY_MANAGEMENT
#endif

void
outta_memory(void)
{
continu_line(memory_100 /* "Out of Memory" */);
}

#ifdef USE_MEMORY_MANAGEMENT
int blocks_out;

struct mblock
	{
	struct mblock *next;
	unsigned size;	/* in paragraphs... */
	};

static struct mblock *free_list;
static unsigned mem_free;

static void *
_lalloc(psize)
unsigned psize;
{
register struct mblock *mb, *nb, *lb;

if ((mb = free_list) != NULL)
	{
	if (mb->size == psize)
		{
		free_list = mb->next;
		mem_free -= psize;
		return((void *)mb);
		}
	else if (mb->size > psize)
		{
		nb = make_ptr(0, ptr_seg(mb)+psize);
		nb->next = mb->next;
		nb->size = mb->size - psize;
		free_list = nb;
		mem_free -= psize;
		return((void *)mb);
		}
	else
		{
		lb = mb;
		mb = mb->next;
		}
	}

while (mb)
	{
	if (mb->size == psize)
		{
		lb->next = mb->next;
		mem_free -= psize;
		return((long *)mb);
		}
	else if (mb->size > psize)
		{
		nb = make_ptr(0, ptr_seg(mb)+psize);
		nb->next = mb->next;
		nb->size = mb->size - psize;
		lb->next = nb;
		mem_free -= psize;
		return((void *)mb);
		}
	else
		{
		lb = mb;
		mb = mb->next;
		}
	}
nomem_exit:
return(NULL);
}

static void
mfree(nb, amount)
register struct mblock *nb;
register unsigned amount;
{
register struct mblock *mb;
register struct mblock *lb;

mem_free += amount;

if ( (mb = free_list) == NULL)
	{
	mb = free_list = nb;
	mb->next = NULL;
	mb->size = amount;
	goto adjust_rd_alloc;
	}
if ( ptr_seg(nb) < ptr_seg(mb))
	{
	free_list = nb;
	nb->next = mb;
	nb->size = amount;
	if ( ptr_seg(nb)+amount == ptr_seg(mb))	/*coalesce into first block*/
		{
		nb->next = mb->next;
		nb->size += mb->size;
		}		
	goto adjust_rd_alloc;
	}
for (;;)
	{
	lb = mb;
	if ( (mb = mb->next) == NULL)
		break;
	if (ptr_seg(nb) - lb->size == ptr_seg(lb)) /*coalesce into previous block*/
		{
		lb->size += amount;
		if (ptr_seg(nb) + amount == ptr_seg(mb))
			{
			lb->size += mb->size;
			lb->next = mb->next;
			}
		goto adjust_rd_alloc;
		}
	if (ptr_seg(nb)+amount == ptr_seg(mb))	/*coalesce into next block*/
		{
		nb->size = mb->size + amount;
		nb->next = mb->next;
		lb->next = nb;
		goto adjust_rd_alloc;
		}
	if (ptr_seg(nb) < ptr_seg(mb))
		{
		nb->next = mb;
		lb->next = nb;
		nb->size = amount;
		goto adjust_rd_alloc;
		}
	}
if (ptr_seg(nb)-lb->size == ptr_seg(lb))	/*a rare case ... */
	{
	lb->size += amount;
	goto adjust_rd_alloc;
	}
lb->next = nb;
nb->next = NULL;
nb->size = amount;
adjust_rd_alloc:
return;
}

/* magic numbers tagged at beginning and end of allocated memory blocks */
#define START_COOKIE (0x41f3)
#define END_COOKIE (0x1599)

void
freemem(UWORD *pt)
{
long psize;
WORD *endcookie;

if (!(*(--pt) == START_COOKIE) )
	{
	old_video();
	printf(memory_101 /* "%x bad START_COOKIE %lx\n" */, *pt, pt+1);
	unconfig_ints();
	exit(0);	/* okok... */
	}
psize = *(--pt);
endcookie = long_to_pt(pt_to_long(pt)+(psize<<4L)-2);
if (*endcookie != END_COOKIE)
	{
	old_video();
	printf(memory_102 /* "%x bad END_COOKIE %lx\n" */, *pt, pt+2);
	unconfig_ints();
	exit(0);	/* okok... */
	}
mfree((struct mblock *)pt, psize);
blocks_out-=1;
}

void *
laskmem(size)
long size;
{
WORD *endcookie;
WORD *pt;
long psize;

if (size > 0xffff0)
	return(0);
psize = ((size+6+15)>>4);
if ((pt = _lalloc(psize)) != NULL)
	{
#ifdef CHECKIT
	if (!check_mem_magic(pt, psize))
		{
		old_video();
		puts("Someone's stomping free memory!");
		unconfig_ints();
		exit(0);	/* okok... */
		}
#endif /* CHECKIT */
	blocks_out++;
	*pt++ = psize;
	*pt++ = START_COOKIE;
	endcookie = long_to_pt(pt_to_long(pt-2)+(psize<<4L)-2);
	*endcookie = END_COOKIE;
	}
return(pt);
}

void *
askmem(size)
unsigned size;
{
return(laskmem((long)size));
}

void *
lbegmem(size)
long size;
{
void *pt;

if (size == 0L)
	{
	continu_line(memory_103 /* "Zero memory request!" */);
	return(NULL);
	}
if ((pt = laskmem(size)) == NULL)
	{
	outta_memory();
	return(NULL);
	}
return(pt);
}

void *
begmem(size)
unsigned size;
{
return(lbegmem((long)size));
}

void
gentle_freemem(pt)
UWORD *pt;
{
if (pt != NULL)
	freemem(pt);
}

int
init_mem(void)
{
register unsigned size;
unsigned err;
union regs r;
char *pool;
extern	unsigned char cdecl _osmajor;

r.w.bx = 0xffff;	/* ask for a meg.... */
r.b.ah = 0x48;
sysint(0x21,&r,&r);
if (_osmajor >= 5)		/* IBM PC DOS 5.0 wants a little more free memory */
	size = r.w.bx-512;		/* leave 8k for DOS 5.0+*/
else
	size = r.w.bx-256;		/* leave 4K for DOS */
for (;;)
	{
	if (size < 9000)
		{
		early_err(memory_104 /* "Not enough memory for Converter, sorry" */);
		return(0);
		}
	r.w.bx = size;
	r.b.ah = 0x48;
	if (!(sysint(0x21,&r,&r) & 1))	/* no error, we got it! */
		break;
	size = size*15L/16;
	}
mfree(make_ptr(0,r.w.ax), size);
return(1);
}
#endif /* USE_MEMORY_MANAGEMENT */
