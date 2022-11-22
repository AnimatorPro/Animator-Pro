/*****************************************************************************
 *
 * pocomemry.c - Memory management routines for compile-time phase of poco.
 *
 * MAINTENANCE
 *	08/27/90	(Ian)
 *				New module.
 *	08/29/90	(Ian)
 *				Removed return value checking, now covered by longjump.
 *	10/29/90	(Ian)
 *				Increased expression frame cache to 32 slots, more things
 *				(for, if, while, do) are using it now.
 ****************************************************************************/

#include <string.h>
#include "poco.h"


/*----------------------------------------------------------------------------
 * Tweakable defines...
 *--------------------------------------------------------------------------*/

#define MBLK_SIZE 65500L		/* Live version of Poco uses bigger chunks. */

#define NUM_SBLK	16			/* Number of small blocks in cache. 		*/
#define NUM_EXPF	32			/* Number of expression frames in cache.	*/
#define NUM_POCF	 4			/* Number of poco frames in cache.			*/

/*----------------------------------------------------------------------------
 * defines and data used by memory management routines...
 *	(NOTE: if any changes are made to cache_cookie, the cookie_val field
 *	MUST remain as the *last* field in the structure, due to the way that
 *	po_freemem() works!)
 *--------------------------------------------------------------------------*/

typedef struct cache_cookie 	/* When we hand out a block from a struct	*/
	{							/* cache it gets prefixed with one of these.*/
	Cache_ctl	*pctl;			/* When po_freemem() sees the MBLK_CACHED  */
	SHORT		cache_slot; 	/* cookie_val, it uses the rest of the info */
	USHORT		cookie_val; 	/* in this structure to free the block back */
	} Cache_cookie; 			/* to the cache it came from.				*/

typedef struct mblk_ctl 		/* This structure is used to track blocks of*/
	{							/* memory we have aquired from our parent.	*/
	struct mblk_ctl 			/* Each block starts with one of these, to	*/
			*next;				/* link it to the next aquired block. This	*/
	long	used;				/* allows us to easily free all aquired mem */
	long	unused; 			/* during error handling.  Used/unused space*/
	} Mblk_ctl; 				/* has meaning only for the current block.	*/

static Mblk_ctl *mblk_cur;		/* This outlives PCB, must live in BSS mem. */
static Poco_cb	*ppcb;			/* Used in error handling, must live in BSS.*/

#define MBLK_CACHED 0x0402		/* Magic #: Block came from struct cache.	*/
#define MBLK_ALLOCD 0x1126		/* Magic #: Block came from regular memory. */
#define MBLK_FREED	0xDEAD		/* Magic #: Block has already been free'd.  */

#define SIZ_SBLK	SMALLBLK_CACHE_SIZE 			   /* Defined in poco.h */
#define SIZ_EXPF	(sizeof(Exp_frame))
#define SIZ_POCF	(sizeof(Poco_frame)+(HASH_SIZE*sizeof(Symbol *)))

#define TOT_SBLK	(NUM_SBLK+(NUM_SBLK*(sizeof(Cache_cookie)+SIZ_SBLK)))
#define TOT_EXPF	(NUM_EXPF+(NUM_EXPF*(sizeof(Cache_cookie)+SIZ_EXPF)))
#define TOT_POCF	(NUM_POCF+(NUM_POCF*(sizeof(Cache_cookie)+SIZ_POCF)))

static Errcode new_mblk(Poco_cb *pcb)
/*****************************************************************************
 * routine to aquire a new mblk memory block from parent's memory allocator.
 * this is now the only point at which we have to check for an out-of-memory
 * condition during a compile, as all memory requests either come from a big
 * pre-allocated mblk, or from a pre-allocated struct cache.
 * we zero-out the memory when we aquire it, and then make no distinction
 * between a po_memalloc and po_memzalloc call, because we know the memory has been
 * cleared.  TPROF says this is a big win in terms of time spent in memset.
 ****************************************************************************/
{
Mblk_ctl *new;

if (NULL == (new = pj_malloc(MBLK_SIZE)))		/* get memory	 */
	{
	if (pcb != NULL)	/* If pcb is NULL, we are doing initial setup, we	*/
		{				/* don't have a pcb yet, so we just return err code.*/
		pcb->global_err = Err_no_memory;
		po_say_fatal(pcb, "poco: out of memory");
		}
	return Err_no_memory;
	}

poco_zero_bytes(new,MBLK_SIZE);
new->next	= mblk_cur;
mblk_cur	= new;
new->used	= sizeof(Mblk_ctl);
new->unused = MBLK_SIZE - sizeof(Mblk_ctl);

return Success;
}

static void init_cache_ctl(Cache_ctl *pctl, char *pmem, SHORT numslots, SHORT slotsize)
/*****************************************************************************
 * init a cache_ctl structure.
 ****************************************************************************/
{
pctl->inuse 	= pmem; 							/* inuse table at start */
pctl->pbase 	= pmem + numslots;					/* of block, data area	*/
pctl->slot_size = slotsize + sizeof(Cache_cookie);	/* follows inuse table. */
pctl->num_slots = numslots;
pctl->nxt_slot	= 0;
}

Errcode po_init_memory_management(Poco_cb **pcb)
/*****************************************************************************
 * init the compile-only memory, struct caches, and the first mblk cache area.
 * this routine inits memory that we **know** does not need to live beyond
 * the compile phase.  all such memory is allocated from the parent as a
 * single big chunk, and then this routine divies it up for its separate
 * uses.  we keep a pointer to the first thing in this single chunk (ppcb)
 * so that later we can free the entire chunk, either at the end of a
 * successfull compile, or during error cleanup handling.
 ****************************************************************************/
{
char	*pmem;

if (NULL == (pmem = pj_zalloc(sizeof(Poco_cb) +  /* if new caches are added  */
							 TOT_SBLK + 		/* add their TOT_xxxx names */
							 TOT_EXPF + 		/* to this list.			*/
							 TOT_POCF)))
	return Err_no_memory;

/*
 * set up the poco_cb area, remember its location so we can free it later...
 */

*pcb = ppcb = (Poco_cb *)pmem;
pmem += sizeof(Poco_cb);

/*
 * set up each of the struct caching areas.  if new caches are added,
 * another pair of lines for each needs to be added here...
 */

init_cache_ctl(&ppcb->smallblk_cache, pmem, NUM_SBLK, SIZ_SBLK);
pmem += TOT_SBLK;

init_cache_ctl(&ppcb->expf_cache, pmem, NUM_EXPF, SIZ_EXPF);
pmem += TOT_EXPF;

init_cache_ctl(&ppcb->pocf_cache, pmem, NUM_POCF, SIZ_POCF);
pmem += TOT_POCF;

return new_mblk(NULL);
}

void po_free_compile_memory(void)
/*****************************************************************************
 * free transient memory used only during the compile phase.
 * all this type of memory is alloc'd in one block.  the first item in the
 * block is the pcb, which we keep a pointer to solely for use in this routine.
 ****************************************************************************/
{
pj_gentle_free(ppcb);
ppcb = NULL;
}

void po_free_all_memory(void)
/*****************************************************************************
 * free all mblk memory blocks.  must be called after final run of poco pgm.
 * may also be called as part of error handling, the implication is mainly
 * that you can't run a poco program after this has been done unless you
 * recompile it.  the memory freed here is that which is used at compile time
 * only & that used during compile and run time.
 ****************************************************************************/
{
Mblk_ctl *cur, *next;

cur = mblk_cur;
while (cur != NULL)
	{
	next = cur->next;
	pj_free(cur);
	cur = next;
	}
mblk_cur = NULL;

po_free_compile_memory();
}

void po_freemem(void *pt)
/*****************************************************************************
 * routine to free memory aquired via po_memalloc, po_memzalloc, or pbegcache.
 * there is a little trickyness to watch out for here:	if memory is handed
 * out from a struct cache (by pbegcache), a full cache_cookie will be present
 * on the front of the memory block.  if the memory is aquired from po_memalloc,
 * only a SHORT value will be on the front of the block.  because cookie_val
 * always appears at the end of a cache_cookie, that ensures that the SHORT
 * which precedes the block is our magic number, and that pc->cookie_val will
 * always address the SHORT that precedes the block. (ugly, but it works.)
 * put another way, if the magic number indicates a non-cached block, don't
 * try to touch any fields in the cache_cookie except cookie_val, because
 * they won't really be part of the cookie, they'll be memory belonging to
 * someone else.
 ****************************************************************************/
{
register Cache_cookie *pc = pt;
register Cache_ctl	  *pctl;

pc--;					/* Back up from start of block to start of cookie.	*/

switch(pc->cookie_val)
	{
	case MBLK_CACHED:
		pctl = pc->pctl;
		pctl->inuse[pc->cache_slot] = 0;
		if (pc->cache_slot < pctl->nxt_slot)
			pctl->nxt_slot = pc->cache_slot;
		break;
	case MBLK_ALLOCD:
		pc->cookie_val = MBLK_FREED;
		break;

#ifdef DEVELOPEMENT

	case MBLK_FREED:
		fprintf(stdout,"\npoc_freemem: freeing memory twice!!!\n");
		break;
	default:
		fprintf(stdout,"\npoc_freemem: unknown magic cookie!!!\n");
		break;
#endif

	}
}

void *po_memalloc(Poco_cb *pcb, register long size)
/*****************************************************************************
 * return a chunk of memory from the current mblk, get a new mblk if needed.
 * from this routine's point of view, we can't be out of memory.  either
 * there is already memory available in the current mblk, or new_mblk() will
 * never return because of the longjmp in po_say_fatal.
 ****************************************************************************/
{
USHORT *pt;

size += sizeof(*pt);

if (size > mblk_cur->unused)
	new_mblk(pcb);

pt = (USHORT *)( ((char *)mblk_cur) + mblk_cur->used );
mblk_cur->used	 += size;
mblk_cur->unused -= size;

*pt++ = MBLK_ALLOCD;

return pt;
}

void *po_memzalloc(Poco_cb *pcb, register size_t size)
/*****************************************************************************
 * return a chunk of memory from the current mblk, get a new mblk if needed.
 * from this routine's point of view, we can't be out of memory.  either
 * there is already memory available in the current mblk, or new_mblk() will
 * never return because of the longjmp in po_say_fatal.
 *
 * this routine can prolly be killed, unless it is determined that under
 * Watcom it is better to zero-out little blocks individually rather than
 * zapping each big mblk as it is allocated.
 ****************************************************************************/
{
USHORT *pt;

size += sizeof(*pt);

if (size > mblk_cur->unused)
	new_mblk(pcb);

pt = (USHORT *)( ((char *)mblk_cur) + mblk_cur->used );
mblk_cur->used	 += size;
mblk_cur->unused -= size;

*pt++ = MBLK_ALLOCD;

return pt;
}

void *po_cache_malloc(Poco_cb *pcb, register Cache_ctl *pctl)
/*****************************************************************************
 * get an item from a struct cache area, or via po_memalloc if the cache is full.
 * we always start searching the inuse table for the cache at the 'nxt_slot'
 * location.  nxt_slot is maintained both here and by po_freemem().  when
 * we hand out a cache block, we set nxt_slot to indicate the slot following
 * the one we handed out.  when a slot is free'd, po_freemem() will set
 * nxt_slot to the slot which was freed if that value is lower than the
 * current value of nxt_slot.  thus, we generally have nxt_slot pointing at
 * a free slot, but very rarely we will have to do a linear search of the
 * inuse table to find an open slot.  For this reason, caches of more than
 * 32 items might hit a point of diminishing returns unless it is known in
 * advance that slots will be freed in exact reverse order of allocation.
 ****************************************************************************/
{
SHORT		 cache_slot;
Cache_cookie *pmem;

for (cache_slot = pctl->nxt_slot; cache_slot < pctl->num_slots; ++cache_slot)
	{
	if (pctl->inuse[cache_slot] == 0)
		{
		pctl->inuse[cache_slot] = 1;
		pctl->nxt_slot = cache_slot + 1;
		pmem = (Cache_cookie *)( pctl->pbase + cache_slot * pctl->slot_size);
		pmem->pctl		 = pctl;
		pmem->cache_slot = cache_slot;
		pmem->cookie_val = MBLK_CACHED;
		++pmem;
		goto OUT;
		}
	}

pctl->nxt_slot = pctl->num_slots;

pmem = po_memalloc(pcb, pctl->slot_size);

OUT:

return pmem;

}

char *po_clone_string(Poco_cb *pcb, char *s)
/*****************************************************************************
 * clone a string, return a pointer to it.
 * (Note to self:
 * this was moved to here because someday I'm gonna think of a way to exploit
 * a direct-copy of the string into an mblk area without needing to walk the
 * string twice, to get its length and then to copy.  seems like we could
 * do it all at once, but we also have to account for a nearly-full mblk.)
 ****************************************************************************/
{
int size;
char *d;

size = strlen(s)+1;
d = po_memalloc(pcb, size);
poco_copy_bytes(s, d, size);
return(d);
}
