
/* memory.c - Heap maintainer.  Gets memory straight from ms-dos.  Has
   problems with some Turbo C library routines (like fwrite...).  Does
   handle allocations greater than 64K, and does some simple error checking
   during free. */

#include "jimk.h"
#include "memory.h"
#include "memory.str"
#include "peekpok_.h"
#include "ptr.h"

unsigned mem_free;

void
outta_memory(void)
{
continu_line(memory_100 /* "Out of Memory" */);
}

#ifdef USE_MEMORY_MANAGEMENT
extern void *lmalloc();
static int blocks_out;

struct mblock
	{
	struct mblock *next;
	unsigned size;	/* in paragraphs... */
	};

static struct mblock *mfree_list;

unsigned
largest_frag()
{
unsigned i = 0;

register struct mblock *n;

n = mfree_list;
while (n)
	{
	if (i < n->size)
		i = n->size;
	n = n->next;
	}
return (i);
}

static void *
_lalloc(psize)
unsigned psize;
{
register struct mblock *mb, *nb, *lb;

if ((mb = mfree_list) != NULL)
	{
	if (mb->size == psize)
		{
		mfree_list = mb->next;
		mem_free -= psize;
		return((void *)mb);
		}
	else if (mb->size > psize)
		{
		nb = make_ptr(0, ptr_seg(mb)+psize);
		nb->next = mb->next;
		nb->size = mb->size - psize;
		mfree_list = nb;
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

#define cruffty_words(a,b,c)  stuff_words(a,b,c)

mfree(nb, amount)
register struct mblock *nb;
register unsigned amount;
{
register struct mblock *mb;
register struct mblock *lb;

mem_free += amount;

if ( (mb = mfree_list) == NULL)
	{
	mb = mfree_list = nb;
	mb->next = NULL;
	mb->size = amount;
	goto adjust_rd_alloc;
	}
if ( ptr_seg(nb) < ptr_seg(mb))
	{
	mfree_list = nb;
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
freememory(UWORD *pt)
{
long psize;
WORD *endcookie;
static char callus[] = 
  memory_101 /* "Internal error.  Please see Appendix B of Autodesk Animator Reference Manual." */;

if (!(*(--pt) == START_COOKIE) )
	{
	old_video();
	printf(memory_102 /* "%x bad START_COOKIE %lx\n" */, *pt, pt+1);
	puts(callus);
	unconfig_ints();
	exit(0);	/* okok... */
	}
psize = *(--pt);
endcookie = long_to_pt(pt_to_long(pt)+(psize<<4L)-2);
if (*endcookie != END_COOKIE)
	{
	old_video();
	printf(memory_103 /* "%x bad END_COOKIE %lx\n" */, *pt, pt+2);
	puts(callus);
	unconfig_ints();
	exit(0);	/* okok... */
	}
mfree(pt, psize);
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
	continu_line(memory_104 /* "Zero memory request!" */);
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

void *
begmemc(size)
unsigned size;
{
void *pt;

if ((pt = begmem(size)) != NULL)
	zero_structure(pt, size);
return(pt);
}

void
gentle_freemem(void *pt)
{
if (pt != NULL)
	freemem(pt);
}
#endif /* USE_MEMORY_MANAGEMENT */
