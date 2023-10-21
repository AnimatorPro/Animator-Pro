#include <stdio.h>
#include <ffi.h>

#include <ffi.h>
#include <stdio.h>

typedef union po_ffi_data /* Overlap popular datatypes in the same space */
{
	int i;
	short s;
//	UBYTE* bpt;
	char c;
	long l;
//	ULONG ul;
	float f;
	double d;
	void* p;
} Po_FFI_Data;

ffi_arg test_printf()
{
	ffi_cif cif;
	void *args[4];
	ffi_type *arg_types[4];

	char *format = "%.5g, %d\n";
	double doubleArg = 3.14159;
	signed int sintArg = 7;
	ffi_arg res = 0;

	arg_types[0] = &ffi_type_pointer;
	arg_types[1] = &ffi_type_double;
	arg_types[2] = &ffi_type_sint;
	arg_types[3] = NULL;

	/* This printf call is variadic */
	ffi_prep_cif_var(&cif, FFI_DEFAULT_ABI, 1, 3, &ffi_type_sint, arg_types);

	args[0] = &format;
	args[1] = &doubleArg;
	args[2] = &sintArg;
	args[3] = NULL;

	ffi_call(&cif, FFI_FN(printf), &res, args);

	return res;
}


void hello_world()
{
	ffi_cif cif;
	ffi_type* arg_types[4];
	void *values[4];
	char *s;
	const char *sub_string = "bunnies";
	ffi_arg rc;

	const double fvalue[1] = {3.14};

	/* Initialize the argument info vectors */
	arg_types[0] = &ffi_type_pointer;
	values[0] = &s;
	arg_types[1] = &ffi_type_double;
	values[1] = (void*)fvalue;

	arg_types[2] = NULL;
	values[2] = NULL;

	/* Initialize the cif */
	if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, 1,
					 &ffi_type_sint, arg_types) == FFI_OK)
	{
		s = "Hello World!";
		ffi_call(&cif, puts, &rc, values);
		/* rc now holds the result of the call to puts */

		/* values holds a pointer to the function's arg, so to
		   call puts() again all we need to do is change the
		   value of s */
		s = "This is cool!";
		ffi_call(&cif, puts, &rc, values);
	}
	else {
		printf("-- Could not create cif for regular function.\n");
	}

	if (ffi_prep_cif_var(&cif, FFI_DEFAULT_ABI,
						 1,
						 2,
					     &ffi_type_sint, arg_types) == FFI_OK)
	{
		/* values holds a pointer to the function's arg, so to
		   call puts() again all we need to do is change the
		   value of s */
		s = "This is cool, %.3f!\n";
		ffi_call(&cif, FFI_FN(printf), &rc, values);

		arg_types[1] = &ffi_type_pointer;
		char* msg = "poco";
		values[1] = (void*)&msg;

		s = "Hello, %s!\n";
		ffi_call(&cif, FFI_FN(printf), &rc, values);
		/* rc now holds the result of the call to puts */

	}
	else {
		printf("-- Could not create cif for variadic function.\n");
	}
}


int main(int argc, char** argv) {
	(void)argc;
	(void)argv;

	hello_world();
	ffi_arg result = test_printf();
	return (int)result;
}

