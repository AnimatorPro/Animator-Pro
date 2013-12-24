/*****************************************************************************
 *
 * code.c	- Code buffer management routines.
 *
 *
 * MAINTENANCE:
 *	08/17/90	(Ian)
 *				Fixed re-alloc logic in add_code().  Old logic added a
 *				space twice as big as the remaining free space instead of
 *				twice the full buffer size.  New logic allocates a buffer
 *				that is EXPANDCBUF times the size of the full buffer.
 *				Also, add_code() now does a direct copy into the code buffer
 *				when the size is 2, 4, or 12 bytes, and only calls memcopy
 *				for other sizes.
 *				In po_add_op(), two calls were always made to add_code().  Now
 *				the second call is only made if there is data to be coded.
 *	08/22/90	(Ian)
 *				Fixed po_cat_code() and po_copy_code() so that they can't call
 *				add_code() with a size of zero.  This allows elimination of
 *				the test in add_code(), and reduces the # of calls to it.
 *	08/25/90	(Ian)
 *				Changed all occurances of freemem() and gentle_freemem() to
 *				po_freemem() and poc_gentle_freemem().
 *	08/27/90	(Ian)
 *				Changed the logic in add_code() so that the first time a
 *				code buffer is expanded (eg, the small builtin code buffer is
 *				full), then next code buffer comes from the small-block cache
 *				(cache of 512 byte blocks).  On subsequent expansions beyond
 *				small-block size, regular malloc'd memory is obtained, using
 *				a modified form of the old formula.
 *	10/07/90	(Ian)
 *				Fixed a bug in add_code().	The logic that decides whether
 *				to grab a new code buffer from cache now checks to insure
 *				that old_size+op_size will fit in a cache buffer.
 *	10/07/90	(Ian)
 *				Removed NULL checks from calls to memory allocation.
 *	 9/06/91	(Jim)
 *				Added checks for OP_BAD in code routines during
 *				development.  Had to separate po_add_code_fixup
 *				for array-initialization fixup processing.
 *	06/03/92	(Ian)
 *				The po_code_pop() routine used to check for the preceding
 *				op being an OP_xPUSH, and if so, it optimized away the
 *				PUSH/POP pair.	If the preceeding instruction was actually
 *				a piece of data (ie, offset/length) that looked like an
 *				OP_xPUSH, we'd glitch out completely; so now we don't try.
 ****************************************************************************/

#include <string.h>
#include "poco.h"

#define EXPANDCBUF 2	/* Make code buffer 2 times larger on re-alloc */

void po_init_code_buf(Poco_cb *pcb, Code_buf *c)
/*****************************************************************************
 * init pre-allocated code buffer (set sizes and pointers for the small
 * embedded code buffer).
 ****************************************************************************/
{
	c->cryptic = CCRYPTIC;
	c->code_buf = c->code_pt = c->cbuf;
	c->alloced_end = c->cbuf + sizeof(c->cbuf);
}

void po_trash_code_buf(Poco_cb *pcb, Code_buf *c)
/*****************************************************************************
 * free codebuf (if code buffer pointer is to malloc'd block, free the block.)
 ****************************************************************************/
{

#ifdef DEVELOPMENT
	if (c->cryptic != CCRYPTIC)
		{
		if (c->cryptic == CTRASHED)
			po_say_internal(pcb, "trashing code_buf twice");
		else
			po_say_internal(pcb, "trashing uninitted code_buf");
		return;
		}
#endif /* DEVELOPMENT */

	c->cryptic = CTRASHED;
	if (c->code_buf != c->cbuf)
		po_freemem(c->code_buf);
}

static
Boolean add_code(Poco_cb *pcb, Code_buf *cbuf, void *ops, SHORT op_size)
/*****************************************************************************
 * add code to buffer, expand buffer if needed.
 ****************************************************************************/
{
	Code	*next_op;
	register unsigned int ropsize = op_size;

#ifdef DEVELOPMENT
	if (ropsize <= 0)
		{
		po_say_internal(pcb, "add_code called with ropsize == 0!");
		}
	if (cbuf->cryptic != CCRYPTIC)
		{
		po_say_internal(pcb, "add_code using uninitialized code_buf");
		return(FALSE);
		}
#endif

	next_op = OPTR(cbuf->code_pt, ropsize);

	if (next_op > cbuf->alloced_end)		/* if we ran out of space... */
		{
		Code	*new_buf,
				*old_buf;
		long	old_used,
				new_size;

		old_buf  = cbuf->code_buf;
		old_used = (UBYTE *)cbuf->code_pt - (UBYTE *)old_buf;
		if (old_buf == cbuf->cbuf && old_used+ropsize < SMALLBLK_CACHE_SIZE)
			{
			new_size = SMALLBLK_CACHE_SIZE;
			new_buf  = po_cache_malloc(pcb, &pcb->smallblk_cache);
			}
		else
			{
			new_size = ropsize + EXPANDCBUF * ((UBYTE *)cbuf->alloced_end - (UBYTE *)old_buf);
			new_buf = po_memalloc(pcb, new_size);
			}
		poco_copy_bytes(old_buf, new_buf, (size_t)old_used);
		cbuf->code_buf	  = new_buf;
		cbuf->alloced_end = new_buf + new_size;
		cbuf->code_pt	  = new_buf + old_used;
		if (old_buf != cbuf->cbuf)
			po_freemem(old_buf);
		next_op = OPTR(cbuf->code_pt, ropsize);
		}

	switch(ropsize) 	/* try to do small common-sized copies fast... */
		{
		case sizeof(short):
			*(unsigned short *)cbuf->code_pt = *(unsigned short *)ops;
			break;
		case sizeof(long):
			*(unsigned long *)cbuf->code_pt = *(unsigned long *)ops;
			break;
		default:
			poco_copy_bytes(ops, cbuf->code_pt, ropsize);
			break;
		}

	cbuf->code_pt = next_op;
	return(TRUE);
}

Boolean po_add_op(Poco_cb *pcb, Code_buf *cbuf,
				int op, void *data, SHORT data_size)
/*****************************************************************************
 * Add a new opcode, and optional data for the op.
 ****************************************************************************/
{
#ifdef DEVELOPMENT
	if (op <= OP_BAD || op >= OP_PAST_LAST)
		{
		po_say_fatal(pcb, "Trying to code invalid opcode %d (not %d-%d)"
		, op, OP_BAD, OP_PAST_LAST);
		}
#endif /* DEVELOPMENT */
	if (!add_code(pcb, cbuf, &op, sizeof(op)))
		return(FALSE);
	if (data_size > 0)
		return(add_code(pcb, cbuf, data, data_size) );
	else
		return(TRUE);
}

void po_backup_code(Poco_cb *pcb, Code_buf *cb, int op_size)
/*****************************************************************************
 * move code pointer back by op_size.
 ****************************************************************************/
{

#ifdef DEVELOPMENT
	if (cb->code_pt - cb->code_buf < op_size)	/* should never happen */
		po_say_internal(pcb, "error in po_backup_code");
	else
#endif
		cb->code_pt -= op_size;
}

long po_cbuf_code_size(Code_buf *c)
/*****************************************************************************
 * return size of code currently in buffer.
 ****************************************************************************/
{
	return(c->code_pt - c->code_buf);
}

Boolean po_cat_code(Poco_cb *pcb, Code_buf *dest, Code_buf *end)
/*****************************************************************************
 * concatenate two chunks of code...
 ****************************************************************************/
{
SHORT size;

	if (0 == (size = end->code_pt - end->code_buf))
		return TRUE;
	return(add_code(pcb, dest, end->code_buf, size));
}

Boolean po_copy_code(Poco_cb *pcb, Code_buf *source, Code_buf *dest)
/*****************************************************************************
 * make dest a copy of source
 ****************************************************************************/
{
SHORT size;

	dest->code_pt = dest->code_buf; 	/* reset dest code pt. back to start */
	if (0 == (size = source->code_pt - source->code_buf))
		return TRUE;
	return(add_code(pcb, dest, source->code_buf, size));
}

void po_code_op(Poco_cb *pcb, Code_buf *cbuf, int op)
/*****************************************************************************
 * code an op with no data
 ****************************************************************************/
{
	po_add_op(pcb, cbuf, op, NULL, 0);
}

void po_add_code_fixup(Poco_cb *pcb, Code_buf *cbuf, int fixup)
/*****************************************************************************
 * Add fixup offset to a code-buf
 ****************************************************************************/
{
	add_code(pcb, cbuf, &fixup, sizeof(fixup));
}

void po_code_pop(Poco_cb *pcb, Code_buf *cbuf, int op, int pushop)
/*****************************************************************************
 * code a pop instruction.
 *
 * 05/03/92:  We used to do this, but the predicted disaster happened:
 *	 if the previous instruction was a push of the same type, the two cancel
 *	 out, so we un-code the push rather than coding the pop.
 *	 Note to self (jk) - this routine looks like a disaster waiting to
 *	 happen.  If the previous op has some data that happens to be
 *	 the same value as the pushop instruction this could get nasty!
 ****************************************************************************/
{
	po_add_op(pcb, cbuf, op, NULL, 0);		/* couldn't optimize it away. */
	return;
}


void po_code_void_pt(Poco_cb *pcb, Code_buf *cbuf, int op, void *val)
/*****************************************************************************
 * code op with a void pointer
 ****************************************************************************/
{
	po_add_op(pcb, cbuf, op, &val, sizeof(val));
}

void po_code_double(Poco_cb *pcb, Code_buf *cbuf, int op, double val)
/*****************************************************************************
 * code op with double data
 ****************************************************************************/
{
	po_add_op(pcb, cbuf, op, &val, sizeof(double));
}

void po_code_long(Poco_cb *pcb, Code_buf *cbuf, int op, long val)
/*****************************************************************************
 * code op with long data
 ****************************************************************************/
{
	po_add_op(pcb, cbuf, op, &val, sizeof(val));
}

void po_code_address(Poco_cb *pcb, Code_buf *cbuf, int op,
	int doff, long dsize)
/*****************************************************************************
 * code an op plus an address (offset and size info)
 ****************************************************************************/
{
	struct int_long {int i;long l;} il;

	il.i = doff;
	il.l = dsize;
	po_add_op(pcb,cbuf, op, &il, sizeof(il) );
}


long po_code_int(Poco_cb *pcb, Code_buf *cbuf, int op, int val)
/*****************************************************************************
 * code op with int data, return fixup position.
 ****************************************************************************/
{
	long fixup_pos;

	fixup_pos = cbuf->code_pt - cbuf->code_buf + sizeof(op);
	po_add_op(pcb, cbuf, op, &val, sizeof(val));
	return(fixup_pos);
}

void po_code_popot(Poco_cb *pcb, Code_buf *cbuf,
	int op, void *min, void *max, void *pt)
/*****************************************************************************
 * Code an op plus a protected poco pointer (min max and value)
 ****************************************************************************/
{
	Popot ppt;

	ppt.min = min;
	ppt.max = max;
	ppt.pt = pt;
	po_add_op(pcb, cbuf, op, &ppt, sizeof(ppt));
}

void po_int_fixup(Code_buf *cbuf, long fixup_pos, int val)
/*****************************************************************************
 * add offset at fixup_pos
 ****************************************************************************/
{
	((int *)OPTR(cbuf->code_buf, fixup_pos))[0] += val;
}

static
Boolean resolve_labels(Poco_cb *pcb, Poco_frame *pf)
/*****************************************************************************
 * Scan through label references and add offset where label declared.
 ****************************************************************************/
{
	Code_label	*cl,
				*cnext;
	Use_label	*ul,
				*unext;
	long		offset;
	Code		*cbuf;
	Boolean 	retval = TRUE;

	cbuf = pf->fcd.code_buf;
	cnext = pf->labels;
	while ((cl = cnext) != NULL)
		{
		if (cl->lvar != NULL && cl->code_pos == 0)
			{
			po_undefined(pcb, cl->lvar->name);
			retval = FALSE;
			}
		unext = cl->uses;
		while ((ul = unext) != NULL)
			{
			offset = cl->code_pos - ul->code_pos;
			((int *)(cbuf+ul->code_pos))[0] = offset;
			unext = ul->next;
			po_freemem(ul);
			}
		cnext = cl->next;
		po_freemem(cl);
		}
	return(retval);
}

Boolean po_compress_func(Poco_cb *pcb, Poco_frame *pf, Func_frame *new)
/*****************************************************************************
 * convert a poco-frame to the smaller func_frame;
 *	Copies code to buffer that's just the right size
 *	and convert local-symbol-list to parameter-only-list.
 *	Resolve labels. Then append func_frame to pcb->run.fff.
 ****************************************************************************/
{
	long csize;

	if (!resolve_labels(pcb, pf))
		return(FALSE);

#ifdef STRING_EXPERIMENT
	po_free_local_string_list(pcb,pf);
#endif /* STRING_EXPERIMENT */
	/* Move code to place just big enough to fit */

	csize = ((UBYTE *)(pf->fcd.code_pt)) - ((UBYTE *)(pf->fcd.code_buf));
	if (csize == 0)
		new->code_pt = NULL;
	else
		{
		new->code_pt = po_memalloc(pcb, csize);
		poco_copy_bytes(pf->fcd.code_buf, new->code_pt, csize);
		}
	new->type = pf->type;
	new->code_size = csize;
	new->ld = pf->ld;
	if (!po_compress_line_data(pcb,new->ld))
		return(FALSE);
	new->next = pcb->run.fff;
	pcb->run.fff = new;
	pf->ld = NULL;
	return(TRUE);
}
