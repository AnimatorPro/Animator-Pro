/* Blockall.h - interface to the block-allocator.  This is useful if
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

#ifndef BLOCKALL_H
#define BLOCKALL_H

typedef struct mem_block
/* Linked list of memory blocks. */
	{
	struct mem_block *next;
	/* Following the next pointer is the real stuff of course... */
	} Mem_block;

typedef struct block_allocator
/* Our master allocator object. */
	{
	struct mem_block *list;	/* list of all blocks */
	char *free_pt;			/* Points to free area within a block. */
	long free_left;			/* Size of free area within a block. */
	long block_size;		/* Minimum block size. */
	void *(*get_ram)(unsigned);	/* Where to get memory. */
	void (*free_ram)(void *pt); /* Where to free memory. */
	int biggest;			//DEBUG
	} Block_allocator;


void construct_block_allocator(Block_allocator *b, long block_size,
	void *(*get_ram)(unsigned), void (free_ram)(void *pt));
/* Set up a block_allocator for use. 
 * A typical call might be:
 *		construct_block_allocator(&b, 512, malloc, free);
 */

void destroy_block_allocator(Block_allocator *b);
/* Free up the mem_block's associated with allocator. */

void *alloc_from_block(Block_allocator *b, unsigned size);
/* This guy actually does the allocation, out of the current block if
 * possible, otherwise out of a new one. */

#endif /* BLOCKALL_H */
