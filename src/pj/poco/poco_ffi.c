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
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>

#include "poco.h"
#include "pocolib.h"
#include "pocoface.h"
#include "ptrmacro.h"

#include <errno.h>
#include <hashmap.h>

#ifdef _MSC_VER
#include <float.h>
#endif

extern void* malloc();
extern void free();


typedef HASHMAP(void, Po_FFI) _func_hash_map;
typedef HASHMAP(char, void)   _name_hash_map;


struct po_func_map {
	 _func_hash_map map;
	 _name_hash_map name_map;
};


size_t po_ffi_funcmap_hash(const void* data);
int po_ffi_func_compare(const void* left, const void* right);


// ===============================================================
/*
 * Convert a poco type to an FFI type.
 */
static inline ffi_type* po_ffi_type_from_ido_type(IdoType ido_type) {
	//!TODO: Handle sub types?

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
 * Return a string name for a specified FFI type.
 */

const char* po_ffi_name_for_type(ffi_type* type) {
	/* Can't switch on a pointer, but this is fine for now. */
	if (type == &ffi_type_sint) {
		return "int";
	}
	else if (type == &ffi_type_sint32) {
		return "long";
	}
	else if (type == &ffi_type_double) {
		return "double";
	}
	else if (type == &ffi_type_pointer) {
		return "pointer";
	}
	else if (type == &ffi_type_void) {
		return "void";
	}

	return "unknown type";
}

// ===============================================================
/*
 * Return the size of the specified FFI type in bytes
 */
static size_t po_ffi_ido_type_size(IdoType ido_type) {
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

	/* !TODO:
	 * Current variadic functions are wrapped with a special signature in the original
	 * version of poco. We have to handle them on both ends.
	 */
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
		binding->arg_types[index]     = po_ffi_type_from_ido_type(param->ti->ido_type);
		binding->arg_sizes[index]     = po_ffi_ido_type_size(param->ti->ido_type);

		total_data_size += binding->arg_sizes[index];
		index += 1;
		param = param->link;
	}

	/* Set result type, or default to void if it doesn't exist. */
	binding->result_ido_type = frame->return_type ? frame->return_type->ido_type : IDO_VOID;
	binding->result_type = (frame->return_type ? po_ffi_type_from_ido_type(frame->return_type->ido_type) :
							&ffi_type_void);

	/* Instead of allocating small bits of memory for each
	 * individual parameter, allocate one big block and then
	 * make the pointers offsets into it. */

	//!TODO: Ignore this and point directly into the stack?
	binding->data_size = total_data_size;

	void* current = (void*)binding->data;
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
	free(binding);
}


// ===============================================================
/*
 * Quick hash for function pointers, and a comparator.
 */

size_t po_ffi_funcmap_hash(const void* data) {
	return hashmap_hash_default(data, sizeof(void*));
}

int po_ffi_func_compare(const void* left, const void* right) {
	if (left == right) {
		return 0;
	}
	else {
		return 1;
	}
}


// ===============================================================
/*
 * Create a new hashmap for functions with the wrapper struct.
 */
Po_FuncMap* po_ffi_funcmap_new() {
	Po_FuncMap* result = malloc(sizeof(Po_FuncMap));
	if (!result) {
		return NULL;
	}

	hashmap_init(&result->map,      po_ffi_funcmap_hash, po_ffi_func_compare);
	hashmap_init(&result->name_map, hashmap_hash_string_i, strcmp);
	return result;
}


// ===============================================================
/*
 * Destroys a function hashmap and deallocates its data.
 */
void po_ffi_funcmap_delete(Po_FuncMap* func_map) {
	Po_FFI* binding = NULL;
	void* key       = NULL;
	char* name      = NULL;

	hashmap_foreach(key, binding, &func_map->map) {
		/* not sure if this is necessary */
		hashmap_remove(&func_map->map, key);
		/* free is necessary */
		free(binding);
	}

	hashmap_foreach(name, binding, &func_map->name_map) {
		hashmap_remove(&func_map->name_map, name);
	}

	hashmap_cleanup(&func_map->map);
	hashmap_cleanup(&func_map->name_map);
	free(func_map);
}


// ===============================================================
/*
 * 	Provided po_ffi_build_structures has been run on the environment,
 *	try to find and return the function specified by the pointer key.
 *	
 *	Returns NULL if the function is not in the map.
*/
Po_FFI* po_ffi_find_binding(const Poco_run_env* env, const void* key) {
	if (!env->func_map) {
		fprintf(stderr, "%s: called on env with NULL map.\n", __FUNCTION__);
		return NULL;
	}

	Po_FFI* binding = hashmap_get(&env->func_map->map, key);
	if (!binding) {
		builtin_err = Err_poco_ffi_func_not_found;
	}
	return binding;
}

/*
 * 	Look up the function binding by name instead of func pointer.
 *
 *	Returns NULL if the function is not in the map.
*/
Po_FFI*  po_ffi_find_binding_by_name(const Poco_run_env* env, const char* name) {
	if (!env->func_map) {
		fprintf(stderr, "%s: called on env with NULL map.\n", __FUNCTION__);
		return NULL;
	}

	void* key = (void*)hashmap_get(&env->func_map->name_map, name);
	if (!key) {
		return NULL;
	}

	Po_FFI* binding = (Po_FFI*)hashmap_get(&env->func_map->map, key);
	return binding;
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

	/* allocate funcmap */
	env->func_map = po_ffi_funcmap_new();
	if (!env->func_map) {
			fprintf(stderr, "%s: Unable to allocate func map\n", __FUNCTION__);
			return Err_poco_ffi_no_func_map;
	}

	C_frame* frame = env->protos;
	int put_result = 0;

	while (frame->mlink) {
		frame = frame->mlink;
		if (frame->type != CFF_C) {
			continue;
		}

		Po_FFI* binding = po_ffi_new(frame);

		/* two maps: one for names, and one for bindings */
		hashmap_put(&env->func_map->name_map, binding->name, binding->function);
		put_result = hashmap_put(&env->func_map->map, binding->function, binding);

		if (put_result < 0) {
			/* puts fail if this already exists */
			po_ffi_delete(binding);
			//! TODO: handle gracefully
			if (put_result == -EEXIST) {
				//				fprintf(stderr, "%s: Pointer for func '%s' already exists\n",
				//						__FUNCTION__, frame->name);
			} else {
				fprintf(stderr,
						"%s: Unable to insert '%s' into func map (error %d)\n",
						__FUNCTION__,
						frame->name,
						put_result);
				return Err_poco_ffi_no_map_insert;
			}
		}
	}

	return Success;
}


// ===============================================================
/*
	Call the C function specified by the binding, grabbing
	passed parameters from the stack.

	Returns a Pt_num for acc.ret in runops.c.
*/

Pt_num po_ffi_call(Po_FFI* binding, const Pt_num* stack_in)
{
	Pt_num result;
	Popot_make_null(&result.ppt);

	if (!binding) {
		builtin_err = Err_poco_ffi_func_not_found;
		return result;
	}

	const int variadic_args = po_ffi_is_variadic(binding) ? -2 : 0;
	const unsigned int real_arg_count = binding->arg_count + variadic_args;

	// data for the function is already allocated at
	// binding creation time-- set the values
	long arg_count = 0;
	long arg_size  = 0;
	unsigned int arg_index = 0;
	Pt_num* stack = stack_in;

	if (variadic_args) {
		arg_count = stack->l;
		binding->data[arg_index].l = arg_count;
		stack = OPTR(stack, sizeof(long));
		arg_index += 1;

		arg_size  = stack->l;
		binding->data[arg_index].l = arg_size;
		stack = OPTR(stack, sizeof(long));
		arg_index += 1;
	}

	for (; arg_index < binding->arg_count; arg_index++) {
		switch(binding->arg_ido_types[arg_index]) {
			case IDO_INT:
				binding->data[arg_index].i = stack->i;
				stack = OPTR(stack, sizeof(int));
				break;
			case IDO_LONG:
				binding->data[arg_index].l = stack->l;
				stack = OPTR(stack, sizeof(long));
				break;
			case IDO_DOUBLE:
				binding->data[arg_index].d = stack->d;
				stack = OPTR(stack, sizeof(double));
				break;
			case IDO_POINTER:
				binding->data[arg_index].p = stack->p;
				//!TODO: is this right?
				stack = OPTR(stack, sizeof(Popot));
				break;
			case IDO_CPT:
				binding->data[arg_index].p = stack->p;
				//!TODO: is this right?
				stack = OPTR(stack, sizeof(void*));
				break;

#ifdef STRING_EXPERIMENT
			case IDO_STRING:
				psize = sizeof(PoString);
				break;
#endif /* STRING_EXPERIMENT */

			default:
				break;
		}
	}

	binding->result = 0;
	ffi_call(&binding->interface, FFI_FN(binding->function), &binding->result, binding->args);

	switch(binding->result_ido_type) {
		case IDO_INT:
			result.i = *((int*)&binding->result);
			break;
		case IDO_LONG:
			result.l = *((long*)&binding->result);
			break;
		case IDO_DOUBLE:
			result.d = *((double*)&binding->result);
			break;
		case IDO_POINTER:
		case IDO_CPT:
			result.p = (void*)(*((UBYTE*)&binding->result));
			break;

#ifdef STRING_EXPERIMENT
		case IDO_STRING:
			psize = sizeof(PoString);
			break;
#endif /* STRING_EXPERIMENT */

		default:
			break;
	}

	return result;
}

