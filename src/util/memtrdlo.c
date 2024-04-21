/* lo level memory stuff. */
#include <stdlib.h>
#include "stdtypes.h"
#include "errcodes.h"
#include "memory.h"
#include "asm.h"
#include "dpmiutil.h"

/* note that size must immediately precede memory !!!! */

struct mblock	/* structure to keep track of free memory */
	{
	struct mblock *next;
	long size;
	};

static struct mblock *free_list = NULL;
long mem_free;
long init_mem_free;

#if defined(__WATCOMC__)
static long find_dos_free_amount(void)
/*
 * Figure out how much memory to allocate leaving DOS a little room.
 */
{
/* leave 64K for system. */
return dos_mem_free() - 64*1024;
}

static long find_free_amount(void)
/*
 * Figure out how much memory to allocate.  Check for DPMI virtual memory
 * manager (aka Windows) and refrain from allocating all virtual memory.
 */
{
Boolean have_dpmi;
DPMIMemoryInfo info;
long amount;

/* Check for dpmi virtual memory.  If it's present just use
 * the unlocked pages of physical memory to avoid a good bit of
 * swapping/thrashing. */
have_dpmi = pj_dpmi_present();
if (have_dpmi)
	have_dpmi = pj_dpmi_inquire_memory(&info);
if (have_dpmi && (info.dpmi_flags & DPMIFLAG_VMM))
	{
	amount = info.bytes_per_page * info.total_unlocked_pages - 0x50000;
	/* The 0x50000 is approximately the size of PJ that shouldn't really
	 * be swapped out for performance reasons.  It's just a guess based
	 * on the code size and data size in the link map. */
	 if (amount <  500000)
	 	{
		/* If they're real low on memory though go ahead and let it
		 * all be virtual. */
		amount = find_dos_free_amount();
		}
	}
else
	{
	amount = find_dos_free_amount();
	}
return amount;
}
#else /* __WATCOMC__ */
static long find_free_amount(void)
{
#if (32 * 1024 * 1024 * 1024L < LONG_MAX)
	return 32 * 1024 * 1024 * 1024L;
#else
	return LONG_MAX;
#endif
}
#endif /* __WATCOMC__ */

Errcode init_mem(long max_mem)
/*
 * Allocate memory for our-selves.
 * Pass in amount of memory to allocate.  If zero then
 * figure out how to grab pretty much all physical memory.
 */
{
register long size;
char *pool;

if (max_mem <= 0)
	size = find_free_amount();
else
	size = max_mem;
for(;;)
	{
	if (size <= 128*1024L)
		return Err_no_memory;
	if((pool = (char *)malloc(size)) != NULL)
		break;
	size -= 32L*1024L;
	}
free_list = (struct mblock *)pool;
free_list->next = NULL;
init_mem_free = mem_free = free_list->size = size;
return Success;
}

#ifdef SLUFFED
void cleanup_mem()
{
	print_alloclist();
}
#endif /* SLUFFED */

#ifdef SLUFFED
long largest_frag()
{
register struct mblock *mb= free_list;
register long longest;
register long size;

longest = 0;
while (mb)
	{
	size = mb->size;
	if (size > longest)
		longest = size;
	mb = mb->next;
	}
return(longest);
}
#endif /* SLUFFED */

static long *lalloc(long nbytes)
{
register struct mblock *mb, *nb, *lb;

#ifdef DEBUG
printf("alloc(%d)\n", nbytes);
#endif /* DEBUG */

if ((mb = free_list) != NULL)
	{
	if (mb->size == nbytes)
		{
#ifdef DEBUG1
		printf("alloc first exact %lx\n", mb);
#endif /* DEBUG1 */
		free_list = mb->next;
		return((long *)mb);
		}
	else if (mb->size > nbytes)
		{
#ifdef DEBUG1
		printf("alloc first %lx\n", mb);
#endif /* DEBUG1 */
		nb = (struct mblock *)(((char *)mb)+nbytes);
		nb->next = mb->next;
		nb->size = mb->size - (long)nbytes;
		free_list = nb;
		return((long *)mb);
		}
	else
		{
		lb = mb;
		mb = mb->next;
		}
	}

while (mb)
	{
	if (mb->size == nbytes)
		{
#ifdef DEBUG1
		printf("alloc exact %lx\n", mb);
#endif /* DEBUG1 */
		lb->next = mb->next;
		return((long *)mb);
		}
	else if (mb->size > nbytes)
		{
#ifdef DEBUG1
		printf("alloc middle %lx\n", mb);
#endif /* DEBUG1 */
		nb = (struct mblock *)(((char *)mb)+nbytes);
		nb->next = mb->next;
		nb->size = mb->size - (long)nbytes;
		lb->next = nb;
		return((long *)mb);
		}
	else
		{
		lb = mb;
		mb = mb->next;
		}
	}
return(NULL);
}


void *lo_askmem(long nbytes)
{
long *pt;

/* Store the size before the buffer. */
nbytes += sizeof(intptr_t);

/* Round up to nearest multiple of 8. */
nbytes = (nbytes + 7) & (~0x07);

if ((pt = lalloc( nbytes )) == NULL)
	return(NULL);
*pt++ = nbytes;
mem_free -= nbytes;
return(pt);
}

long lo_freemem(void *p)
/* returns size freed (size allocd with) */
{
long *pt = p;
register struct mblock *mb;
register struct mblock *lb;
register struct mblock *nb;
register long amount;

#ifdef DEBUG
printf("mfree(%lx, %d)\n", nb, amount);
#endif /* DEBUG */

amount = *(--pt);
nb = (struct mblock *)pt;
mem_free += amount;

if ( (mb = free_list) == NULL)
	{
#ifdef DEBUG1
	printf("new free_list\n");
#endif /* DEBUG1 */
	mb = free_list = nb;
	mb->next = NULL;
	mb->size = amount;
	goto adjust_rd_alloc;
	}
if ( nb < mb)
	{
	free_list = nb;
	nb->next = mb;
	nb->size = amount;
	if ( (char *)nb+amount == (char *)mb)	/*coalesce into first block*/
		{
		nb->next = mb->next;
		nb->size += mb->size;
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
	if ((char *)nb - lb->size == (char *)lb)	/*coalesce into previous block*/
		{
		lb->size += amount;
		if ((char *)nb + amount == (char *)mb)
			{
			lb->size += mb->size;
			lb->next = mb->next;
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
	if ((char *)nb+amount == (char *)mb)	/*coalesce into next block*/
		{
		nb->size = mb->size + amount;
		nb->next = mb->next;
		lb->next = nb;
#ifdef DEBUG1
		printf("coalescing into next block\n");
#endif /* DEBUG1 */
		goto adjust_rd_alloc;
		}
	if (nb < mb)
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
if ((char *)nb-lb->size == (char *)lb)	/*a rare case ... */
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
return(amount - sizeof(long));
}


