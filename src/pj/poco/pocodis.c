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

/*****************************************************************************
 * do sanity check on instruction table.
 ****************************************************************************/
Boolean po_check_instr_table(Poco_cb* pcb)
{
#ifdef DEVELOPMENT
	int i;

	for (i = 0; i < po_ins_table_els; i++) {
		if (i != po_ins_table[i].op_type) {
			po_say_internal(pcb,
							"opcode mismatch %d != %d on %s",
							i,
							po_ins_table[i].op_type,
							po_ins_table[i].op_name);
		}
	}
#endif
	return (TRUE);
}

/*****************************************************************************
 * search list of c_frames to find a given function.
 ****************************************************************************/
char* find_c_name(C_frame* list, void* fpt)
{
	while (list != NULL) {
		if (fpt == list->code_pt)
			return (list->name);
		list = list->mlink;
	}
	return ("(unknown)");
}

/*****************************************************************************
 * disassemble a single instruction.
 ****************************************************************************/
void* po_disasm(FILE* f, void* code, C_frame* cframes)
{
	int op;
	Func_frame* fuf;
	Poco_op_table* pta;
	Popot* pp_code;

	op	 = ((int*)code)[0];
	code = OPTR(code, sizeof(op));
	if (op >= 0 && op < po_ins_table_els) {
		pta = po_ins_table + op;
		fprintf(f, "\t%-15s\t", pta->op_name);
		switch (pta->op_ext) {
			case OEX_NONE:
				break;
			case OEX_VAR:
			case OEX_INT:
			case OEX_LABEL:
				fprintf(f, "\t%d", ((int*)code)[0]);
				break;
			case OEX_ADDRESS:
				fprintf(f,
						"\tvar %d size %ld",
						((int*)OPTR(code, 0))[0],
						((long*)OPTR(code, sizeof(int)))[0]);
				break;
			case OEX_LONG:
				fprintf(f, "\t%d", ((LONG*)code)[0]);
				break;
			case OEX_POINTER:
				pp_code = (Popot*)code;
				fprintf(f,
//						"\tmin %p max %p pt %p (%llu bytes)",
						"\tpointer: %p (%lu bytes)",
						pp_code->pt,
						pp_code->max - pp_code->min);
				break;
			case OEX_DOUBLE:
				fprintf(f, "\t%f", ((double*)code)[0]);
				break;
			case OEX_FUNCTION:
				fuf = ((Func_frame**)code)[0];
				fprintf(f, "\t%s", fuf->name);
				break;
			case OEX_CFUNCTION:
				if (cframes != NULL)
					fprintf(f, "\t%s", find_c_name(cframes, ((void**)code)[0]));
				break;
		}
		code = OPTR(code, pta->op_size);
		fprintf(f, "\n");
	} else {
		fprintf(f, "Wild op 0x%d\n", op);
		code = OPTR(code, sizeof(op));
	}
	return code;
}


/*****************************************************************************
 * disassemble lots of ops, starting from runtime environment only
 ****************************************************************************/
void po_disassemble_code(Poco_run_env* poco_env, FILE* file, void* code, long csize)
{
	void* end;
	end = OPTR(code, csize);
	while (code < end) {
		code = po_disasm(file, code, (C_frame*)(poco_env->protos));
	}
	fflush(file);
}


/*****************************************************************************
 * disassemble lots of ops.
 ****************************************************************************/
void dump_code(Poco_cb* pcb, FILE* file, void* code, long csize)
{
	po_disassemble_code(&pcb->run, file, code, csize);
}


/*****************************************************************************
 * disassemble an entire poco program.
 ****************************************************************************/
void po_dump_file(Poco_cb* pcb)
{
	Func_frame* ff;
	FILE* file;

	if ((file = pcb->po_dump_file) == NULL)
		return;
	ff = pcb->run.fff;
	while (ff != NULL) {
		fprintf(file, "Program: %s -- Code size: %ld\n", ff->name, ff->code_size);
		dump_code(pcb, file, ff->code_pt, ff->code_size);
		ff = ff->next;
	}
}

/*****************************************************************************
 * disassemble an entire poco program and write to specified file.
 ****************************************************************************/
void po_disassemble_program(Poco_run_env* poco_env, FILE* fp) {
	if (fp == NULL) {
		return;
	}

	Func_frame* frame = poco_env->fff;
	while (frame != NULL) {
		fprintf(fp, "function: %s\tcode size: %ld\n", frame->name, frame->code_size);
		po_disassemble_code(poco_env, fp, frame->code_pt, frame->code_size);
		frame = frame->next;
	}
}

#ifdef DEVELOPMENT

/*****************************************************************************
 * disassmble ops to stdout on the fly during compile; for debugging.
 ****************************************************************************/
void po_dump_codebuf(Poco_cb* pcb, Code_buf* cbuf)
{
	dump_code(pcb, stdout, cbuf->code_buf, (cbuf->code_pt - cbuf->code_buf));
}

#endif /* DEVELOPMENT */
