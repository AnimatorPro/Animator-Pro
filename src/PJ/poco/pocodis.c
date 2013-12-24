/*****************************************************************************
 *
 * pocodis.c - stuff to dis-assmble poco code.
 *
 *	10/05/90	(Ian)
 *				Converted sprintf/po_say_fatal pairs to po_say_fatal w/formatting.
 *	10/26/90	(Ian)
 *				Changed printing of pointer types to %p instead of %lx.
 *	12/10/90	(Ian)
 *				In po_disasm(), added check for op>0 as well as op<numops.
 ****************************************************************************/

#include "poco.h"
#include "pocoop.h"

Boolean po_check_instr_table(Poco_cb *pcb)
/*****************************************************************************
 * do sanity check on instruction table.
 ****************************************************************************/
{

#ifdef DEVELOPMENT
int i;

for (i=0; i<po_ins_table_els; i++)
	{
	if (i != po_ins_table[i].op_type)
		{
		po_say_internal(pcb, "opcode mismatch %d != %d on %s",
			i, po_ins_table[i].op_type, po_ins_table[i].op_name);
		}
	}
#endif
return(TRUE);
}

char *find_c_name(C_frame *list, void *fpt)
/*****************************************************************************
 * search list of c_frames to find a given function.
 ****************************************************************************/
{
while (list != NULL)
	{
	if (fpt == list->code_pt)
		return(list->name);
	list = list->mlink;
	}
return("(unknown)");
}

void *po_disasm(FILE *f, void *code, C_frame *cframes)
/*****************************************************************************
 * disassemble a single instruction.
 ****************************************************************************/
{
int op;
Func_frame *fuf;
Poco_op_table *pta;

op = ((int *)code)[0];
code = OPTR(code, sizeof(op) );
if (op >= 0 && op < po_ins_table_els)
	{
	pta = po_ins_table + op;
	fprintf(f, "\t%-15s\t", pta->op_name);
	switch (pta->op_ext)
		{
		case OEX_NONE:
			break;
		case OEX_VAR:
		case OEX_INT:
		case OEX_LABEL:
			fprintf(f, "\t%d",
				((int *)code)[0]);
			break;
		case OEX_ADDRESS:
			fprintf(f, "\tvar %d size %ld",
				((int *)OPTR(code,0))[0],
				((long *)OPTR(code,sizeof(int)))[0]);
			break;
		case OEX_LONG:
			fprintf(f, "\t%ld", ((LONG *)code)[0] );
			break;
		case OEX_POINTER:
			fprintf(f, "\tmin 0x%p max 0x%p pt 0x%p",
				((Popot *)code)->min,
				((Popot *)code)->max,
				((Popot *)code)->pt);
			break;
		case OEX_DOUBLE:
			fprintf(f, "\t%f",
				((double *)code)[0]);
			break;
		case OEX_FUNCTION:
			fuf = ((Func_frame **)code)[0];
			fprintf(f, "\t%s", fuf->name);
			break;
		case OEX_CFUNCTION:
			if (cframes != NULL)
				fprintf(f, "\t%s",
					find_c_name(cframes, ((void **)code)[0]));
			break;
		}
	code = OPTR(code, pta->op_size);
	fprintf(f, "\n");
	}
else
	{
	fprintf(f, "Wild op %d 0x%d\n", op);
	code = OPTR(code, sizeof(op));
	}
return(code);
}

void dump_code(Poco_cb *pcb, FILE *file, void *code, long csize)
/*****************************************************************************
 * disassmble lots of ops.
 ****************************************************************************/
{
void *end;
end = OPTR(code,csize);
while (code < end)
	{
	code = po_disasm(file, code, (C_frame *)(pcb->run.protos));
	}
fflush(file);
}

void po_dump_file(Poco_cb *pcb)
/*****************************************************************************
 * disassemble an entire poco program.
 ****************************************************************************/
{
Func_frame *ff;
FILE *file;

if (( file = pcb->po_dump_file) == NULL)
	return;
ff = pcb->run.fff;
while (ff != NULL)
	{
	fprintf(file, ".......%s..%ld......\n", ff->name,
		ff->code_size);
	dump_code(pcb, file, ff->code_pt, ff->code_size);
	ff = ff->next;
	}
}

#ifdef DEVELOPMENT

void po_dump_codebuf(Poco_cb *pcb, Code_buf *cbuf)
/*****************************************************************************
 * disassmble ops to stdout on the fly during compile; for debugging.
 ****************************************************************************/
{
	dump_code(pcb, stdout, cbuf->code_buf, (cbuf->code_pt - cbuf->code_buf));
}

#endif /* DEVELOPMENT */
