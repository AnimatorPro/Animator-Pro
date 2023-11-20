#include <stdio.h>
#include <ffi.h>

#include <vector>


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
	unsigned char* str;
} Po_FFI_Data;


class FFI_VarArgCall {
	std::vector<po_ffi_data> arguments;
	std::vector<ffi_type*> arg_types;
	std::vector<void*> args;

  public:
	FFI_VarArgCall() {};
	~FFI_VarArgCall() {};

	ffi_arg exec() {
		ffi_arg result = 0;
		ffi_cif cif;

		const int arg_count = arguments.size();
		ffi_type** arg_types_pointer = (ffi_type**)arg_types.data();

		void* _args[17];
		for (int i = 0; i < args.size(); i++) {
			_args[i] = args[i];
		}
		_args[args.size()] = nullptr;

		ffi_prep_cif_var(&cif, FFI_DEFAULT_ABI,
						 1,
						 arg_count,
						 &ffi_type_sint,
						 arg_types_pointer);

		ffi_call(&cif, FFI_FN(printf), &result, _args);
		return result;
	}

	void arguments_start() {
		arguments.clear();
		arg_types.clear();
		args.clear();

		arguments.reserve(17);
		arg_types.reserve(17);
		args.reserve(17);
	}

	void arguments_end() {
		arg_types.push_back(nullptr);
		args.push_back(nullptr);
	}

	void push_s8_arg(const int8_t arg) {
		Po_FFI_Data data = {0};
		data.c = arg;
		arguments.push_back(data);
		arg_types.push_back(&ffi_type_sint8);
		args.push_back(&arguments[arguments.size()-1]);
	}

	void push_int32_arg(const int32_t arg) {
		Po_FFI_Data data = {0};
		data.i = arg;
		arguments.push_back(data);
		arg_types.push_back(&ffi_type_sint32);
		args.push_back(&arguments[arguments.size()-1]);
	}

	void push_float_arg(const float arg) {
		Po_FFI_Data data = {0};
		data.f = arg;
		arguments.push_back(data);
		arg_types.push_back(&ffi_type_float);
		args.push_back(&arguments[arguments.size()-1]);
	}

	void push_double_arg(const double arg) {
		Po_FFI_Data data = {0};
		data.d = arg;
		arguments.push_back(data);
		arg_types.push_back(&ffi_type_double);
		args.push_back(&arguments[arguments.size()-1]);
	}

	void push_void_pointer_arg(void* arg) {
		Po_FFI_Data data = {0};
		data.p = arg;
		arguments.push_back(data);
		arg_types.push_back(&ffi_type_pointer);
		args.push_back(&arguments[arguments.size()-1]);
	}

	void push_string_arg(const char* arg) {
		push_void_pointer_arg((void*)arg);
	}
};


ffi_arg test_printf_class() {
	FFI_VarArgCall printf_call;

	const char* msg = "Testing: \"%.5f, %d\\n\"";

	printf_call.arguments_start();
		printf_call.push_string_arg(msg);
		printf_call.push_double_arg(3.14159);
		printf_call.push_int32_arg(7);
		printf_call.arguments_end();

	return printf_call.exec();
}

ffi_arg test_printf()
{
	ffi_cif cif;
	void *args[4];
	ffi_type *arg_types[4];

	char *format = "%.5f, %d\n";
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
		ffi_call(&cif, FFI_FN(puts), &rc, values);
		/* rc now holds the result of the call to puts */

		/* values holds a pointer to the function's arg, so to
		   call puts() again all we need to do is change the
		   value of s */
		s = "This is cool!";
		ffi_call(&cif, FFI_FN(puts), &rc, values);
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

//	hello_world();
//	ffi_arg result = test_printf();
	ffi_arg result = test_printf_class();
	return (int)result;
}

