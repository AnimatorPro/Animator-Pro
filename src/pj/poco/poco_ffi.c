/****************************************************************************
 * poco_ffi.c - replacement for runcall.asm, using libffi to pass parameters
 *              to compiled functions based on parsed Func_frame objects.
* 
 * I went this direction because the runcall.asm solution is brilliant, but
 * I was hoping to keep the project as cross-platform as possible.  Libffi
 * looks like the best bet and is in use by some of the world's biggest
 * projects.
 *
 * MAINTENANCE:
 *	28/dec/2022    (kiki)    File created, and structs added to poco.h.
 *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>

#include "poco.h"
#include "pocoface.h"
#include "ptrmacro.h"

#ifdef _MSC_VER
#include <float.h>
#endif

extern void* malloc();
extern void free();


// ===============================================================
/*
 * Convert a poco type to an FFI type.
 */
static inline ffi_type* po_get_ffi_type(IdoType ido_type) {
	switch(ido_type) {
		case IDO_INT:
			return &ffi_type_sint;
			break;
		case IDO_LONG:
			return &ffi_type_sint32;
			break;
		case IDO_DOUBLE:
			return &ffi_type_double;
			break;

		/* kiki note:
		 * the old pointers were much smaller; I'm not sure that
		 * there is really a difference between IDO_POINTER and
		 * IDO_CPT anymore. */

#ifdef STRING_EXPERIMENT
		case IDO_STRING:
#endif

		case IDO_POINTER:
		case IDO_CPT:
			return &ffi_type_pointer;
			break;

		case IDO_VOID:
			return &ffi_type_void;

		default:
			return NULL;
	}
}


// ===============================================================
/*
 * Return the size of the specified FFI type in bytes
 */
static size_t po_get_ffi_type_size(IdoType ido_type) {
	switch(ido_type) {
		case IDO_INT:
			return sizeof(int);
		case IDO_LONG:
			return sizeof(long);
		case IDO_DOUBLE:
			return sizeof(double);

#ifdef STRING_EXPERIMENT
		case IDO_STRING:
#endif
		/* kiki note:
		 * the old pointers were much smaller; I'm not sure that
		 * there is really a difference between IDO_POINTER and
		 * IDO_CPT anymore. */

		case IDO_POINTER:
			return sizeof(Popot);
		case IDO_CPT:
			return sizeof(void*);

		default:
			return 0;
	}
}


// ===============================================================
static int po_ffi_is_variadic(Po_FFI* binding) {
	return binding->flags & PO_FFI_VARIADIC;
}


// ===============================================================
/*
 *  Allocate a new Po_FFI struct, then fill it with calling information
 *  and places to store results based on the Func_frame.
 *
 *  Returns NULL if it is unable to allocate memory or
 *  otherwise create the libffi structures.
*/
Po_FFI* po_ffi_new(const C_frame* frame) {
	assert(frame != NULL);

	Po_FFI* binding = calloc(1, sizeof(Po_FFI));
	printf("[FFI] Binding %s...\n", frame->name);

	size_t index = 0;
	size_t total_data_size = 0;

	/* Assuming the frame will outlive the binding */
	binding->name      = frame->name;
	binding->function  = frame->code_pt;
	binding->arg_count = frame->pcount;

	/*
	 * Unfortunately, because params are stored as a linked list
	 * and we need to know if the last parameter is ellipsis, we
	 * have to traverse the list twice and act accordingly.
	 */

	Symbol* param = frame->parameters;
	index = 0;
	while (param) {
		if (!param->ti) {
			fprintf(stderr,
					"po_ffi_new: '%s' parameter %s (%zu) has no type info.\n",
					frame->name,
					param->name,
					index);
			po_ffi_delete(binding);
			return NULL;
		}

		if (param->ti->comp_count && param->ti->comp[0] == TYPE_ELLIPSIS) {
			/* Remove argument index for the ellipsis, but add two for the count and size params.
			 * Writing it out long like this to remind myself why. 
			 */
			binding->arg_count = binding->arg_count + 2 - 1;
			binding->flags |= PO_FFI_VARIADIC;
			break;
		}

		param = param->link;
		index += 1;
	}

	/* second loop -- set everything */
	param = frame->parameters;
	index = 0;

	if (po_ffi_is_variadic(binding)) {
		/* Add in the count / size parameters */
		binding->arg_ido_types[0] = binding->arg_ido_types[1] = IDO_LONG;
		binding->arg_types[0]     = binding->arg_types[1]     = &ffi_type_slong;
		/* near as I can tell, every param is a full pointer in size regardless of data */
		binding->arg_sizes[0]     = binding->arg_sizes[1]     = sizeof(void*);
		total_data_size  = sizeof(void*) * 2;
		index = 2;
	}

	while (param) {
		if (param->ti->comp_count && param->ti->comp[0] == TYPE_ELLIPSIS) {
			param = param->link;
			continue;
		}

		binding->arg_ido_types[index] = param->ti->ido_type;
		binding->arg_types[index]     = po_get_ffi_type(param->ti->ido_type);
		binding->arg_sizes[index]     = po_get_ffi_type_size(param->ti->ido_type);

		total_data_size += binding->arg_sizes[index];
		index += 1;
		param = param->link;
	}

	/* Set result type, or default to void if it doesn't exist. */
	binding->result_type = (frame->return_type ?
							po_get_ffi_type(frame->return_type->ido_type) :
							&ffi_type_void);

	/* Instead of allocating small bits of memory for each
	 * individual parameter, allocate one big block and then
	 * make the pointers offsets into it. */

	//!TODO: Ignore this and point directly into the stack?
	binding->data_size = total_data_size;
	binding->data = malloc(total_data_size);

	if (!binding->data) {
		fprintf(stderr, "po_ffi_new: '%s' unable to allocate %zu bytes for *data.\n",
				binding->name, binding->data_size);
		po_ffi_delete(binding);
		return NULL;
	}

	void* current = binding->data;
	for (index = 0; index < binding->arg_count; index++) {
		binding->args[index] = current;
		current += binding->arg_sizes[index];
	}

	/* Finally, generate the CIF */
	if (po_ffi_is_variadic(binding)) {
		/* For variadic functions, we set up a base case CIF so that
		 * calls with zero arguments can use this one, while calls
		 * with more arguments will need to create a new
		 * CIF at runtime. */
		if (ffi_prep_cif_var(&binding->interface,
						 FFI_DEFAULT_ABI,
						 binding->arg_count,
						 binding->arg_count,
						 binding->result_type,
						 &binding->arg_types) != FFI_OK) {
			fprintf(stderr, "po_ffi_new: '%s' couldn't create variadic CIF.\n", binding->name);
			po_ffi_delete(binding);
			return NULL;
		}
	}
	else {
		/* fixed-argument functions can stick with the prepared
		 * CIF for all calls */
		if (ffi_prep_cif(&binding->interface,
						  FFI_DEFAULT_ABI,
						  binding->arg_count,
						  binding->result_type,
						  binding->arg_types) != FFI_OK) {
			fprintf(stderr, "po_ffi_new: '%s' couldn't create fixed CIF.\n", binding->name);
			po_ffi_delete(binding);
			return NULL;
		}
	}

	return binding;
}


// ===============================================================
/*
	Fully deallocate a Po_FFI struct pointer created by po_ffi_new.
*/
void po_ffi_delete(Po_FFI* binding) {
	assert(binding != NULL);

	if (binding->data) {
		free(binding->data);
	}
	if (binding->data_variadic) {
		free(binding->data_variadic);
	}

	free(binding);
}

// ===============================================================
/*
	Call the C function specified by the binding, using the 
	Poco_run_env to grab passed parameters from the stack.
*/

void po_ffi_call(Poco_run_env* env, Po_FFI* binding) {
	printf("Calling %s - %d parameter%s\n",
		   binding->name,
		   binding->arg_count,
		   plural(binding->arg_count));
}

// ===============================================================
/*
	Take the compiled run environment and build the libffi strucures
	required for function calls.
*/

int po_ffi_build_structures(Poco_run_env* env) {
	if (!env ||
		!env->protos ||
		!env->protos->next ||
		!env->protos->next->mlink) {
			return Err_poco_ffi_no_protos;
	}

	/* Near as I can tell:
	 * - env->protos points to a Func_frame for the whole file
	   - env->protos->next point to the main function
	   - env->protos->mlink is the same as env->protos->next
	   - env->protos->next->mlink is the start of the library functions
	*/

	assert(env);
	assert(env->protos);
	assert(env->protos->next);
	assert(env->protos->next->mlink);

	C_frame* frame = env->protos->next->mlink;

	return Success;

	while (frame) {
		frame = frame->mlink;
		if (frame->type != CFF_C) {
			continue;
		}
		printf("[FFI] %s ...\n", frame->name);
		Po_FFI* binding = po_ffi_new(frame);
	}

	return Success;
}

