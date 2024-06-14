/*****************************************************************************
 *
 * trace. c - stuff to help debug a poco program.
 *
 *	manages structures to associate code with the source that produced it.
 *	also stuff to print out a stack trace.
 *
 * MAINTENANCE
 *	08/25/90	(Ian)
 *				po_add_line_data() now takes no action if line number is zero.
 *				(IE, builtin protos don't need line data, it gets ignored.)
 *	08/25/90	(Ian)
 *				Changed all occurances of freemem() and gentle_freemem() to
 *				po_freemem() and poc_gentle_freemem().
 *	08/27/90	(Ian)
 *				Took the check for line number zero out of po_add_line_data(),
 *				the statement() routine will now do this check and eliminate
 *				the call to po_add_line_data() if the line number is zero.
 *	08/28/90	(Ian)
 *				Changed po_new_line_data() to allocate the initial line data
 *				offsets/lnums from the small-block cache (cache of 512 byte
 *				blocks).  Changed po_compress_line_data() to call the resize
 *				routine even if the new size equals the current size; this
 *				forces the line data to get moved to a real (non-cached)
 *				storage block during compression, even if there happens to
 *				be exactly 64 entries in the data area (needed because the
 *				line data lives beyond the compile phase, and the cache area
 *				does not.)
 *	10/07/90	(Ian)
 *				Removed NULL checks from calls to memory allocation.
 *	10/21/90	(Ian)
 *				Rewrote find_builtin_name() because of the way we handle lib
 *				protos now, it's not convenient to search the lib protos
 *				for the name, now we check on the list of runtime protos.
 *				Also, tweaked i86_ptr_to_long() to work under TURBO.
 *	09/06/91	(Jim)
 *				Added some String support.
 *	09/17/92	(Ian)
 *				Fixed erronious line number reporting that happened when the
 *				first item in the linedata was found in find_line().
 ****************************************************************************/

#include <string.h>

#include "poco.h"
#include "pocoface.h"

#define i86_ptr_to_long(a) (a)

/*****************************************************************************
 * alloc and init a new line_data structure.
 ****************************************************************************/
Line_data *po_new_line_data(Poco_cb *pcb)
{
#define DSIZE SMALLBLK_CACHE_SIZE / (2 * sizeof(long))
	Line_data *new;

	new = po_memzalloc(pcb, sizeof(*new));
	new->offsets = po_cache_malloc(pcb, &pcb->smallblk_cache);
	new->lines = new->offsets + DSIZE;
	new->alloc = DSIZE;
	return (new);
}

/*****************************************************************************
 * resize line data offsets & line numbers areas to hold given # of elements.
 ****************************************************************************/
static bool resize_line_data(Poco_cb *pcb, Line_data *ld, int nalloc)
{
	long *no, *nl;
	int count = ld->count;

	no = po_memalloc(pcb, nalloc * 2 * sizeof(long));
	nl = no + nalloc;
	poco_copy_bytes(ld->offsets, no, count * sizeof(long));
	poco_copy_bytes(ld->lines, nl, count * sizeof(long));
	po_freemem(ld->offsets);
	ld->offsets = no;
	ld->lines = nl;
	ld->alloc = nalloc;
	return (true);
}

/*****************************************************************************
 * shrink the line data offsets/lines area to the right size.
 ****************************************************************************/
bool po_compress_line_data(Poco_cb *pcb, Line_data *ld)
{
#ifdef DEVELOPMENT
	if (ld == NULL) /* added to watch out for trouble with the new concept of */
	{               /* not tying line_data structs to FTY_STRUCT poco_frames. */
		po_say_internal(pcb, "NULL line-data pointer in po_compress_line_data");
	}
#endif

	if (ld->count == 0) {
		poc_gentle_freemem(ld->offsets);
		ld->offsets = NULL;
		return true;
	} else {
		return (resize_line_data(pcb, ld, ld->count));
	}
}

/*****************************************************************************
 * free a line_data struct, and its associated offsets/line data area.
 ****************************************************************************/
void po_free_line_data(Line_data *ld)
{
	if (ld != NULL) {
		poc_gentle_freemem(ld->offsets);
		po_freemem(ld);
	}
}

/*****************************************************************************
 * add a new line number/code offset pair to a line_data struct.
 * if we are out of room, we resize the data area to twice its current size.
 ****************************************************************************/
bool po_add_line_data(Poco_cb *pcb, Line_data *ld, long offset, long line)
{
	int count;

#ifdef DEVELOPMENT
	if (ld == NULL) /* added to watch out for trouble with the new concept of */
	{               /* not tying line_data structs to FTY_STRUCT poco_frames. */
		po_say_internal(pcb, "trying to add using NULL ptr in po_add_line_data");
		return false;
	}
#endif

	if (ld->alloc <= (count = ld->count)) {
		if (!resize_line_data(pcb, ld, ld->alloc << 1)) {
			return (false);
		}
	}
	ld->offsets[count] = offset;
	ld->lines[count] = line;
	ld->count += 1;
	return (true);
}

/*****************************************************************************
 * find the source code line number for a given code offset.
 ****************************************************************************/
long find_line(Line_data *ld, long offset)
{
	int i = ld->count;
	long *offsets = ld->offsets;
	long *lines = ld->lines;

	while (i--) {
		++lines;
		if (offset < *offsets++) {
			break;
		}
	}
	return (*(--lines));
}

/*****************************************************************************
 * find the name of a library function.
 ****************************************************************************/
static char *find_builtin_name(Poco_run_env *pe, void *val)
{
	Func_frame *fuf;

	for (fuf = pe->fff; fuf != NULL; fuf = fuf->mlink) {
		if (fuf->code_pt == val) {
			return fuf->name;
		}
	}
	return ("??unknown??");
}

/*****************************************************************************
 * locate the parent fuf of a given location in the program's code buffer.
 ****************************************************************************/
static Func_frame *which_frame(Poco_run_env *pe, void *ipin)
{
	Code *ip = ipin;
	Func_frame *fuf = pe->fff;

	while (fuf != NULL) {
		if (i86_ptr_to_long(ip) >= i86_ptr_to_long(fuf->code_pt) &&
			i86_ptr_to_long(ip) < i86_ptr_to_long(fuf->code_pt) + fuf->code_size) {
			break;
		}
		fuf = fuf->next;
	}
	return (fuf);
}

/*****************************************************************************
 * indicate whether symbol is an array of or pointer to char (ie, a string).
 ****************************************************************************/
static bool is_char_string_type(Type_info *ti)
{
	return (ti->comp_count == 2 && ti->comp[0] == TYPE_CHAR);
}

/*****************************************************************************
 * make a guess as to whether a string is printable ascii or not.
 ****************************************************************************/
bool po_seems_ascii(char *s)
{
	int count = 0;
	char c;

	for (count = 0; count < 50; count++) {
		c = *s++;
		if (c == 0) {
			break;
		}
		if (c < 7) {
			return false;
		}
	}
	if (count == 50) {
		return false;
	}
	return true;
}

/*****************************************************************************
 * print values that were passed as parameters, part of stack trace output.
 ****************************************************************************/
static void print_param(FILE *f, void *param, int offset, Type_info *ti)
{
	char *s;

	param = OPTR(param, offset);
	switch (ti->ido_type) {
		case IDO_INT:
			fprintf(f, "%d", ((int *)param)[0]);
			break;
		case IDO_LONG:
			fprintf(f, "%ld", ((long *)param)[0]);
			break;
		case IDO_POINTER:
			s = ((Popot *)param)->pt;
			if (s == NULL) {
				fprintf(f, "(NULL)");
				break;
			}
			if (ti->comp[ti->comp_count - 2] == TYPE_FUNCTION) {
				fprintf(f, "%s", ((Func_frame *)s)->name);
			} else if (is_char_string_type(ti) && po_seems_ascii(s)) {
				fprintf(f, "\"%s\"", s);
			} else {
				fprintf(f, "0x%p", s);
			}
			break;
		case IDO_CPT:
			s = ((char **)param)[0];
			if (is_char_string_type(ti) && po_seems_ascii(s)) {
				fprintf(f, "\"%s\"", s);
			} else {
				fprintf(f, "0x%p", s);
			}
			break;
		case IDO_DOUBLE:
			fprintf(f, "%f", ((double *)param)[0]);
			break;
#ifdef STRING_EXPERIMENT
		case IDO_STRING:
			s = PoStringBuf((PoString *)param);
			fprintf(f, "\"%s\"", s);
#endif /* STRING_EXPERIMENT */
	}
}

/*****************************************************************************
 * format error message & stack trace for a runtime error in a poco program.
 ****************************************************************************/
void po_print_trace(Poco_run_env *pe, FILE *tfile, Pt_num *stack, Pt_num *base, Pt_num *globals,
					Pt_num *ip, Errcode cerr)
{
	Func_frame *fuf;
	int i, j, pcount;
	Symbol *param;
	long line = 0;

	if ((fuf = which_frame(pe, ip)) != NULL) {
		line = find_line(fuf->ld, (Code *)ip - fuf->code_pt);
		fprintf(tfile, "near line %ld of %s\n", line, pe->fff->name);
	}

	if (pe->err_line != NULL) {
		*(pe->err_line) = line;
	}

	if (cerr) {
		fprintf(tfile, "The error was detected by library function: %s()\n",
				find_builtin_name(pe, ip->func));
	}

	fprintf(tfile, "Stack trace (call history):\n");

	for (i = 0; i < 50; i++) {
		if ((fuf = which_frame(pe, ip)) == NULL) {
			break;
		}
		fprintf(tfile, "\t%s(", fuf->name);
		param = fuf->parameters;
		pcount = fuf->pcount;
		if (pcount > 0) {
			for (j = 1; j < pcount; j++) {
				print_param(tfile, base, param->symval.doff, param->ti);
				fprintf(tfile, ", ");
				param = param->link;
			}
			print_param(tfile, base, param->symval.doff, param->ti);
		}
		fprintf(tfile, ")\n");
		ip = ((void **)base)[1];
		base = ((void **)base)[0];
	}
	fprintf(tfile, "\n");
}
