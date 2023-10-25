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

#define assertm(exp, msg) assert(((void)msg, exp))


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
ffi_type* po_ffi_type_from_ido_type(IdoType ido_type) {
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

const char* po_ffi_name_for_type(const ffi_type* type) {
	/* Can't switch on a pointer, but this is fine for now. */
	if (type == &ffi_type_sint) {
		return "int";
	}
	if (type == &ffi_type_uint) {
		return "unsigned int";
	}
	else if (type == &ffi_type_uint32) {
		return "unsigned long";
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
	else if (type == &ffi_type_uint8) {
		return "uint8";
	}
	else if (type == &ffi_type_sint8) {
		return "sint8";
	}
	else if (type == &ffi_type_uint16) {
		return "uint16";
	}
	else if (type == &ffi_type_sint16) {
		return "sint16";
	}
	else if (type == &ffi_type_uint64) {
		return "uint64";
	}
	else if (type == &ffi_type_sint64) {
		return "sint64";
	}
	else if (type == &ffi_type_float) {
		return "float";
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
Boolean po_ffi_is_variadic(const Po_FFI* binding) {
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
static char* po_ffi_status_str(const ffi_status status) {
	switch(status) {
		case FFI_OK:
			return "OK";
		case FFI_BAD_TYPEDEF:
			return "Bad Typedef";
		case FFI_BAD_ABI:
			return "Bad ABI";
		case FFI_BAD_ARGTYPE:
			return "Bad ArgType";
	}
}

// ===============================================================
static inline char* po_ffi_argtype_str(const ffi_type* type) {
	if (type == &ffi_type_void)    return "ffi_type_void";
	if (type == &ffi_type_uint8)   return "ffi_type_uint8";
	if (type == &ffi_type_sint8)   return "ffi_type_sint8";
	if (type == &ffi_type_uint16)  return "ffi_type_uint16";
	if (type == &ffi_type_sint16)  return "ffi_type_sint16";
	if (type == &ffi_type_uint32)  return "ffi_type_uint32";
	if (type == &ffi_type_sint32)  return "ffi_type_sint32";
	if (type == &ffi_type_uint64)  return "ffi_type_uint64";
	if (type == &ffi_type_sint64)  return "ffi_type_sint64";
	if (type == &ffi_type_float)   return "ffi_type_float";
	if (type == &ffi_type_double)  return "ffi_type_double";
	if (type == &ffi_type_pointer) return "ffi_type_pointer";

	return "";
}


// ===============================================================
static size_t po_ffi_argtype_size(const ffi_type* type) {
	if (type == &ffi_type_void)    return sizeof(void*);
	if (type == &ffi_type_uint8)   return sizeof(uint8_t);
	if (type == &ffi_type_sint8)   return sizeof(int8_t);
	if (type == &ffi_type_uint16)  return sizeof(uint16_t);
	if (type == &ffi_type_sint16)  return sizeof(int16_t);
	if (type == &ffi_type_uint32)  return sizeof(uint32_t);
	if (type == &ffi_type_sint32)  return sizeof(int32_t);
	if (type == &ffi_type_uint64)  return sizeof(uint64_t);
	if (type == &ffi_type_sint64)  return sizeof(int64_t);
	if (type == &ffi_type_float)   return sizeof(float);
	if (type == &ffi_type_double)  return sizeof(double);
	if (type == &ffi_type_pointer) return sizeof(void*);

	assertm(0, "Invalid FFI argument type!");
	return 0;
}


// ===============================================================
/*
	Fill in variadic parameter values and types from the stack pointer.
*/
static void po_ffi_assign_variadic_parameters(Po_FFI* binding, const long var_arg_count,
									          const void* in_stack, const ffi_type** variadic_types)
{
	unsigned int i;
	void* stack = in_stack;

	/* Note: we adjusted the parameters outside in po_ffi_call(),
	 * so we don't need to worry here about the extra two parameters
	 * poco generates for variadic functions. */

//	for(i = 0; i < out->arg_count; i++) {
//		out->data[i]          = binding->data[i];
//		out->arg_ido_types[i] = binding->arg_ido_types[i];
//		out->arg_types[i]     = binding->arg_types[i];
//		out->arg_sizes[i]     = binding->arg_sizes[i];
//	}

	if (var_arg_count) {
		for (i = 0; i < var_arg_count; i++) {
			const size_t argtype_size = po_ffi_argtype_size(variadic_types[i]);
			const char* argtype_name = po_ffi_argtype_str(variadic_types[i]);
			binding->data[binding->arg_count+i].p = stack;
			binding->args[binding->arg_count+i] = &binding->data[binding->arg_count+i];
			binding->arg_types[binding->arg_count+i] = variadic_types[i];
			binding->arg_sizes[binding->arg_count+i] = argtype_size;
			// No need to worry about IDO types-- this structure
			// won't last long enough to be queried.
			stack = OPTR(stack, argtype_size);
			binding->arg_count += 1;
		}
	}

	binding->arg_types[binding->arg_count] = NULL;
	binding->data[binding->arg_count].p = NULL;

//	// finally, make sure all the arg pointers are correct
//	void* current = (void*)binding->data;
//	for (i = 0; i < original_arg_count; i++) {
//		binding->args[i] = current;
//		// current = OPTR(current, binding->arg_sizes[i]);
//		current = OPTR(current, sizeof(Po_FFI_Data));
//		fprintf(stderr, "\t+ ArgType %d: %s\n", i, po_ffi_argtype_str(binding->arg_types[i]));
//	}
}


// ===============================================================
/*
	Fully deallocate a Po_FFI struct pointer created by po_ffi_new
	or po_ffi_assign_variadic_parameters.
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
static Po_FuncMap* po_ffi_funcmap_new() {
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
static void po_ffi_funcmap_delete(Po_FuncMap* func_map) {
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
Po_FFI* po_ffi_find_binding_by_name(const Poco_run_env* env, const char* name) {
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
Pt_num po_ffi_call(const Po_FFI* binding, const Pt_num* stack_in, const ffi_type** variadic_types)
{
	Po_FFI exec_binding;
	Pt_num result;
	Popot_make_null(&result.ppt);

	if (!binding) {
		builtin_err = Err_poco_ffi_func_not_found;
		return result;
	}

	memcpy(&exec_binding, binding, sizeof(Po_FFI));
	const unsigned int original_arg_count = binding->arg_count;

	const int is_variadic = po_ffi_is_variadic(&exec_binding) ? 2 : 0;

	/* These don't change for non-variadics, or for variadics
	 * with no extra passed parameters. */
	long arg_count = 0;
	long arg_size  = 0;
	Pt_num* stack = stack_in;

	if (is_variadic) {
		/* Poco passes in an argument count and a length in bytes
		 * of the argument data passed.  We have to pop those here
		 * in order to get to the good data for our functions. */
		arg_count = stack->l;
		stack = OPTR(stack, sizeof(long));

		arg_size  = stack->l;
		stack = OPTR(stack, sizeof(long));
	}

	/* This first part only copies the initial binding's argument values! 
	 * The variadic parameters need to be copied in po_ffi_assign_variadic_parameters,
	 * if they exist.
	 *
	 * Variadic functions always have two extra parameters inserted by the
	 * system before the real parameters, as the old system wrapped those
	 * calls in binding functions; we're skipping them here. */
	
	for (unsigned int arg_index = is_variadic; arg_index < binding->arg_count; arg_index++) {
		const unsigned int true_arg_index = arg_index - is_variadic;
		switch(binding->arg_ido_types[arg_index]) {
			case IDO_INT:
				exec_binding.data[true_arg_index].i = stack->i;
				exec_binding.args[true_arg_index] = &exec_binding.data[true_arg_index].i;
				stack = OPTR(stack, sizeof(int));
				break;
			case IDO_LONG:
				exec_binding.data[true_arg_index].l = stack->l;
				exec_binding.args[true_arg_index] = &exec_binding.data[true_arg_index].l;
				stack = OPTR(stack, sizeof(long));
				break;
			case IDO_DOUBLE:
				exec_binding.data[true_arg_index].d = stack->d;
				exec_binding.args[true_arg_index] = &exec_binding.data[true_arg_index].d;
				stack = OPTR(stack, sizeof(double));
				break;
			case IDO_POINTER:
				exec_binding.data[true_arg_index].p = stack->p;
				exec_binding.args[true_arg_index] = &exec_binding.data[true_arg_index].p;
				stack = OPTR(stack, sizeof(Popot));
				break;
			case IDO_CPT:
				exec_binding.data[true_arg_index].p = stack->p;
				exec_binding.args[true_arg_index] = &exec_binding.data[true_arg_index].p;
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

	// important!
	exec_binding.arg_count -= is_variadic;

	if (is_variadic && arg_count) {
		// make the new interface objet with the new data
		po_ffi_assign_variadic_parameters(&exec_binding, arg_count, (void*)stack, variadic_types);
	}

	/* At this point all the binding types and sizes are set,
	 * but we still need to assign the data pointers.
	 *
	 * This seems strange, but the function requires us to pass
	 * a memory location, even if it's for POD data.  For pointers,
	 * we need the memory location of the pointer, not the location
	 * of the data it's pointing to! */

	// for(int i = 0; i < exec_binding.arg_count; i++) {
	// 	exec_binding.args[i] = (void*)&exec_binding.data[i].p;
	// }

	// Null-pad the last argument
	exec_binding.args[exec_binding.arg_count] = NULL;
	exec_binding.arg_types[exec_binding.arg_count] = NULL;

	ffi_status status = ffi_prep_cif_var(
	  &exec_binding.interface,
	  FFI_DEFAULT_ABI,
	  original_arg_count - is_variadic,  // note that we are comparing the base arg count
	  exec_binding.arg_count,            // vs the total arguments passed in
	  exec_binding.result_type,
	  exec_binding.arg_types);

	if (status != FFI_OK) {
		fprintf(stderr, "po_ffi_call: '%s' couldn't prep variadic CIF-- %s.\n",
				exec_binding.name, po_ffi_status_str(status));
		return result;
	}

	/* We store the ffi_arg result value in the binding itself, but
	 * we break the amount out for a proper return below. */
	exec_binding.result = 0;
	ffi_call(&exec_binding.interface, FFI_FN(exec_binding.function),
			 &exec_binding.result, &exec_binding.args);

	switch(exec_binding.result_ido_type) {
		case IDO_INT:
			result.i = *((int*)&exec_binding.result);
			break;
		case IDO_LONG:
			result.l = *((long*)&exec_binding.result);
			break;
		case IDO_DOUBLE:
			result.d = *((double*)&exec_binding.result);
			break;
		case IDO_POINTER:
		case IDO_CPT:
			result.p = (void*)(*((UBYTE*)&exec_binding.result));
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

