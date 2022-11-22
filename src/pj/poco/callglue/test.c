#include <stdio.h>
#include "stdtypes.h"
#include "cstack.h"

typedef struct popot {
	void *min;
	void *max;
	void *pt;
} Popot;

extern int poco_call_ifunc(void *stack_descriptor, void *poco_stack,
					       int cstack_size, int (*cvector)() );

extern void *poco_call_pfunc(void *stack_descriptor, void *poco_stack,
					         int cstack_size, void *(*cvector)() );

extern USHORT poco_call_sfunc(void *stack_descriptor, void *poco_stack,
					         int cstack_size, USHORT (*cvector)() );

extern UBYTE poco_call_bfunc(void *stack_descriptor, void *poco_stack,
					         int cstack_size, UBYTE (*cvector)() );

extern double poco_call_ffunc(void *stack_descriptor, void *poco_stack,
					          int cstack_size, double (*cvector)() );

extern Popot poco_call_ppfunc(void *stack_descriptor, void *poco_stack,
					           int cstack_size, Popot (*cvector)() );

void postack_to_cstack(void *stack_desc,void *poco_stack,C_stack cstack)

#ifdef BIG_COMMENT

	This is an external symbol called by the poco_call_Xfunc() assembler code.

	Here we move the arguments from the poco stack into the "C" stack in the 
	format desired by the C stack.  The stack descriptor item should have 
	the actual number of arguments and their poco stack types described 
	so they can be looped onto the C stack.  The pointer to the poco stack
	is then put on the end of the C stack.  Since the stack descriptor will
	be defined by the poco code calling the C vector this means the poco_stack
	will be the last arg for the c function after all the arguments in the 
	prototype and is accessable by the C library code via va_arg or by 
	declaring the poco stack argument.
	
	I wrote some macros in cstack.h similar to va_arg() macros but work in 
	reverse to install data in a stack instead of taking it out.  There are
	two types of calls one to do incrementing and one to do pushing (down)
	on the stack.

#endif
{
struct abc { char *a; char c;char cc;int b; } *ps;

	ps = (struct abc *)poco_stack;
	add_c_stack(cstack,char *,&ps->a);
	add_c_stack(cstack,char,&ps->c);
	add_c_stack(cstack,char,&ps->cc);
	add_c_stack(cstack,int,&ps->b);

	/* put pointer to start of poco stack at end of args */

	add_c_stack(cstack,void *,poco_stack);
}

int c_int_func(char *a1, char a2, char a3, int a4, void *poco_stack)
{
	printf("a1 = |%s| a2 |%c| a3 |%c| a4 %d\n", a1, a2, a3, a4 );
	return(999);
}
void* c_ptr_func(char *a1, char a2, char a3, int a4, void *poco_stack)
{
	printf("a1 = |%s| a2 |%c| a3 |%c| a4 %d\n", a1, a2, a3, a4 );
	return("c_ptr_func return");
}
double c_float_func(char *a1, char a2, char a3, int a4, void *poco_stack)
{
	printf("a1 = |%s| a2 |%c| a3 |%c| a4 %d\n", a1, a2, a3, a4 );
	return(1.01234);
}
Popot c_popot_func(char *a1, char a2, char a3, int a4, void *poco_stack)
{
Popot pp;

	pp.min = "min";
	pp.max = "max";
	pp.pt = "pt";

	printf("a1 = |%s| a2 |%c| a3 |%c| a4 %d\n", a1, a2, a3, a4 );
	return(pp);
}

main()
{
int ret;
char *pret;
double dret;
Popot ppret;
struct { char *a; char c;char cc;int b; } pstack = { "arg 1",'A','B',123, };

	printf("mastack %d\n", &pstack);
	ret = poco_call_ifunc(NULL,&pstack,16,c_int_func);
	printf("return %d\n", ret);
	pret = (char *)poco_call_pfunc(NULL,&pstack,16,c_ptr_func);
	printf("preturn |%s|\n", pret);
	dret = poco_call_ffunc(NULL,&pstack,16,c_float_func);
	printf("dret |%f|\n", dret);
	ppret = poco_call_ppfunc(NULL,&pstack,16,c_popot_func);
	printf("ppret min |%s| max |%s| pt |%s|\n",
			ppret.min, ppret.max, ppret.pt );
}
