
/* memory.c - Heap maintainer.  Gets memory straight from ms-dos.  Has
   problems with some Turbo C library routines (like fwrite...).  Does
   handle allocations greater than 64K, and does some simple error checking
   during free. */

#include "jimk.h"
#include "memory.str"
#include "peekpok_.h"
#include "ptr.h"

#ifdef NEVER
#define CHECKIT	/* slow but sure heap */
#endif /* NEVER */

#ifdef CHECKIT
#define mem_magic 0x9702
#endif /* CHECKIT */

extern void *lmalloc();
static int blocks_out;

struct mblock
	{
	struct mblock *next;
	unsigned size;	/* in paragraphs... */
	};

static struct mblock *mfree_list;
unsigned mem_free;

#ifdef LATER
frags()
{
register unsigned i = 0;
register struct mblock *n;

n = mfree_list;
while (n)
	{
	i++;
	n = n->next;
	}
return (i);
}
#endif /* LATER */

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

outta_memory()
{
continu_line(memory_100 /* "Out of Memory" */);
}

#ifdef CHECKIT
static char lomem[4];
#endif /* CHECKIT */

static void *
_lalloc(psize)
unsigned psize;
{
register struct mblock *mb, *nb, *lb;

#ifdef DEBUG
printf("_lalloc(%u)\n", psize);
#endif /* DEBUG */

if ((mb = mfree_list) != NULL)
	{
	if (mb->size == psize)
		{
#ifdef DEBUG1
		printf("alloc first exact %lx\n", mb);
#endif /* DEBUG1 */
		mfree_list = mb->next;
		mem_free -= psize;
		return((void *)mb);
		}
	else if (mb->size > psize)
		{
#ifdef DEBUG1
		printf("alloc first %lx\n", mb);
#endif /* DEBUG1 */
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
#ifdef DEBUG1
		printf("alloc exact %lx\n", mb);
#endif /* DEBUG1 */
		lb->next = mb->next;
		mem_free -= psize;
		return((long *)mb);
		}
	else if (mb->size > psize)
		{
#ifdef DEBUG1
		printf("alloc middle %lx\n", mb);
#endif /* DEBUG1 */
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

#ifdef DEBUG
printf("mfree(%lx, %d)\n", nb, amount);
#endif /* DEBUG */

mem_free += amount;
#ifdef CHECKIT
mem_to_magic(nb, amount);
early_magic(nb);
#endif /* CHECKIT */

if ( (mb = mfree_list) == NULL)
	{
#ifdef DEBUG1
	printf("new free_list\n");
#endif /* DEBUG1 */
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
#ifdef CHECKIT
		cruffty_words(mem_magic, mb, 3);
#endif /* CHECKIT */
#ifdef DEBUG1
		printf("coalescing into first chunk\n");
#endif /* DEBUG1 */
		}		
#ifdef DEBUG1
	else
		printf("new first chunk\n");	
#endif /* DEBUG1 */
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
#ifdef CHECKIT
			cruffty_words(mem_magic, mb, 3);
#endif /* CHECKIT */
#ifdef DEBUG1
			printf("coalescing both sides\n");
#endif /* DEBUG1 */
			}
#ifdef DEBUG1
		else
			printf("coalescing into previous block\n");
#endif /* DEBUG1 */
		goto adjust_rd_alloc;
		}
	if (ptr_seg(nb)+amount == ptr_seg(mb))	/*coalesce into next block*/
		{
		nb->size = mb->size + amount;
		nb->next = mb->next;
#ifdef CHECKIT
		cruffty_words(mem_magic, mb, 3);
#endif /* CHECKIT */
		lb->next = nb;
#ifdef DEBUG1
		printf("coalescing into next block\n");
#endif /* DEBUG1 */
		goto adjust_rd_alloc;
		}
	if (ptr_seg(nb) < ptr_seg(mb))
		{
#ifdef DEBUG1
		printf("adding block in middle\n");
#endif /* DEBUG1 */
		nb->next = mb;
		lb->next = nb;
		nb->size = amount;
		goto adjust_rd_alloc;
		}
	}
if (ptr_seg(nb)-lb->size == ptr_seg(lb))	/*a rare case ... */
	{
#ifdef DEBUG1
	printf("coalescing into end of last block\n");
#endif /* DEBUG1 */
	lb->size += amount;
	goto adjust_rd_alloc;
	}
#ifdef DEBUG1
printf("adding last block\n");
#endif /* DEBUG1 */
lb->next = nb;
nb->next = NULL;
nb->size = amount;
adjust_rd_alloc:
return;
}


#ifdef CHECKIT
early_magic(pt)
register UWORD *pt;
{
*pt++ = mem_magic;
*pt++ = mem_magic;
*pt++ = mem_magic;
}
#endif /* CHECKIT */

#ifdef CHECKIT
mem_to_magic(pt, amount)
register UWORD *pt;
UWORD amount;
{
pt += 3;	/* skip pointer to next */
*pt++ = mem_magic;
*pt++ = mem_magic;
*pt++ = mem_magic;
*pt++ = mem_magic;
*pt++ = mem_magic;
amount -= 1;
while (amount != 0)
	{
	*pt++ = mem_magic;
	*pt++ = mem_magic;
	*pt++ = mem_magic;
	*pt++ = mem_magic;

	*pt++ = mem_magic;
	*pt++ = mem_magic;
	*pt++ = mem_magic;
	*pt++ = mem_magic;
	pt = norm_pointer(pt);
	amount -= 1;
	}
}
#endif /* CHECKIT */

#ifdef CHECKIT
check_mem_magic(p, amount)
WORD *p;
unsigned amount;
{
register WORD *pt;
int bad_magic;

pt = p+3;
bad_magic = 0;
bad_magic |=  *pt++ - mem_magic;
bad_magic |=  *pt++ - mem_magic;
bad_magic |=  *pt++ - mem_magic;
bad_magic |=  *pt++ - mem_magic;
bad_magic |=  *pt++ - mem_magic;
if (!bad_magic)
	{
	amount -= 1;
	while (amount != 0)
		{
		bad_magic |=  *pt++ - mem_magic;
		bad_magic |=  *pt++ - mem_magic;
		bad_magic |=  *pt++ - mem_magic;
		bad_magic |=  *pt++ - mem_magic;
		bad_magic |=  *pt++ - mem_magic;
		bad_magic |=  *pt++ - mem_magic;
		bad_magic |=  *pt++ - mem_magic;
		bad_magic |=  *pt++ - mem_magic;
		pt = norm_pointer(pt);
		amount -= 1;
		if (bad_magic)
			break;
		}
	if (!bad_magic)
		return(1);
	}
return(0);
}
#endif /* CHECKIT */

/* magic numbers tagged at beginning and end of allocated memory blocks */
#define START_COOKIE (0x41f3)
#define END_COOKIE (0x1599)

freememory(pt)
UWORD *pt;
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

gentle_freemem(pt)
void *pt;
{
if (pt != NULL)
	freemem(pt);
}

#ifdef CHECKIT
ck_block(pt)
UWORD *pt;
{
long size;
unsigned bsize;
#define MAXB (16*1024)

size = pt[2];
size <<= 3;
size -= 3;
pt += 3;
while (size > 0)
	{
	if (size < MAXB)
		bsize = size;
	else
		bsize = MAXB;
	if (pt[0] != mem_magic)
		return(0);
	if (fsame(pt, bsize) != bsize)
		return(0);
	pt = norm_pointer(pt+bsize);
	size -= bsize;
	}
return(1);
}
#endif /* CHECKIT */

#ifdef CHECKIT
ck_heap()
{
struct mblock *pt;

pt = mfree_list;
while (pt != NULL)
	{
	if (!ck_block(pt))
		return(0);
	pt = pt->next;
	}
return(1);
}
#endif /* CHECKIT */

#ifdef CHECKIT
check_heap()
{
if (!ck_heap())
	{
	old_video();
	puts("Heap in a bad way.");
	unconfig_ints();
	exit(0);
	}
if (bcompare(NULL, lomem, 4) != 4)
	{
	old_video();
	puts("Someone stepped on lo memory!");
	unconfig_ints();
	exit(0);
	};
}
#endif /* CHECKIT */

#ifdef CHECKIT
seal_heap()
{
struct mblock *h;

h = mfree_list;
while (h != NULL)
	{
	mem_to_magic(h, h->size);
	h = h->next;
	}
}
#endif /* CHECKIT */
