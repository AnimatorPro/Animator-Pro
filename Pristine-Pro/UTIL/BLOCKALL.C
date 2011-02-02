/* Blockall.c - The block-allocator.  This is useful if
 * you are in a situation that requires a lot of allocations that will
 * all be released at once.    
 *
 * You need to call construct_block_allocator() first,  and 
 * destroy_block_allocator() when you want to free everything up.
 * In between call alloc_from_block().
 *
 * alloc_from_block() asks for RAM from a lower level memory manager
 * in block_size chunks.  If a request fits in a block it is already
 * using then alloc_from_block() can quickly return the next piece
 * of that block.  Otherwise it goes for a new block.  If the memory
 * request is larger than block-size then it gets its very own block
 * of exactly the right size.
 *
 * 512 seems a good block_size for most situations.
 */


#include "stdtypes.h"
#include "ptrmacro.h"
#include "memory.h"
#include "blockall.h"

void construct_block_allocator(Block_allocator *b, long block_size,
	void *(*get_ram)(unsigned), void (free_ram)(void *pt))
/* Set up a block_allocator for use. 
 * A typical call might be:
 *		construct_block_allocator(&b, 512, malloc, free);
 */
{
	b->list = NULL;
	b->free_pt = NULL;
	b->free_left = 0;
	b->block_size = block_size;
	b->get_ram = get_ram;
	b->free_ram = free_ram;
}


void destroy_block_allocator(Block_allocator *b)
/* Free up the mem_block's associated with allocator. */
{
	struct mem_block *list;
	struct mem_block *next;

	next = b->list;
	while ((list = next) != NULL)
		{
		next = list->next;
		b->free_ram(list);
		}
	clear_struct(b);		/* Just so we die quickly if still in use after
							 * being destroyed. */
}

static void *alloc_another_block(Block_allocator *b, unsigned size)
/* Helper function that allocates a new block and puts it on the list
 * (but DOESN'T update the current block in free_pt and free_left). */
{
	Mem_block *mb;

	if ((mb = b->get_ram(size + sizeof(*mb))) == NULL)
		return NULL;
	mb->next = b->list;
	b->list = mb;
	return (mb+1);
}

void *alloc_from_block(Block_allocator *b, unsigned size)
/* This guy actually does the allocation, out of the current block if
 * possible, otherwise out of a new one. */
{
	void *pt;

	if (size > b->block_size)	/* Big blocks serviced directly */
		return alloc_another_block(b,size);
	if (b->free_left < size)
		{
		if ((b->free_pt = alloc_another_block(b, b->block_size)) == NULL)
			return NULL;
		b->free_left = b->block_size;
		}
	b->free_left -= size;
	pt = b->free_pt;
	b->free_pt += size;
	return pt;
}
