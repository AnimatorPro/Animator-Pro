#include <stdio.h>
#include <ffi.h>

#include <ffi.h>
#include <stdio.h>

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
	ffi_type *args[1];
	void *values[1];
	char *s;
	ffi_arg rc;

	/* Initialize the argument info vectors */
	args[0] = &ffi_type_pointer;
	values[0] = &s;

	/* Initialize the cif */
	if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, 1,
					 &ffi_type_sint, args) == FFI_OK)
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
}


int main(int argc, char** argv) {
	(void)argc;
	(void)argv;

	hello_world();
	ffi_arg result = test_printf();
	return (int)result;
}

