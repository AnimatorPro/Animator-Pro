#ifndef WIDGET_H
#define WIDGET_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef PTRMACRO_H
	#include "ptrmacro.h"
#endif


/* Where outside code calls to to check widget */

typedef struct widgelib {
	void (*init_bracket)(void *vector);
	void *bracket_it;
	void *testdat;
} Widgelib;

#ifdef BIG_COMMENT
/*
  	The WBRACK() macro will "bracket" an errcode returning function with 
	a widget check. 

	err = WBRACK(name_of_function,(arg1,arg2,...));

	In code it can substitute for:

	err = name_of_function(arg1,arg2,...);

  	The WBRACK() may return Err_no_memory or Err_widget in addition to 
  	what the function would.

	One way to use it would be to define macros in the program bracketing
	critical functions such as:

	#define open_window(owargs) WBRACK(open_window,(owargs))
*/
#endif /* BIG_COMMENT */

/* the WBRAK macro */

#define BRACKOFF 2543  /* offset applied to bracket vector to make it hard 
						* to trace with a debugger. It is resolved by the
						* compiler and will fold in with the constant
						* address of the function to be bracketed. */

#define WBRACK(funk,args) \
	( (widge_funcs->init_bracket)(OPTR(funk,BRACKOFF)),\
	  (*((EFUNC)widge_funcs->bracket_it))args)


/* this is used to insure the global pointers are what they should be */

#define WRESET1(func,args) (widge_funcs = &widge_initlib,func args)
#define WRESET2(func,args) (widge_initlib.init_bracket = init_widget,func args)

#ifndef WIDGET_INTERNALS

extern Widgelib widge_initlib;
extern void init_widget(void *);
extern Widgelib *widge_funcs;


#else /* WIDGET_INTERNALS */

#define EXIT_CODE_SIZE 32

/* items passed IN to widget rex code from "host" */

typedef struct widge_hostlib {
	struct widgelib **pwlib;		/* pointer to what to free */
	void *self_free;  /* buffer to move self free code into */
	void (*free_wlib)(struct widgelib **pwl,void *bvector);
	int (*inb)(int port);
	void (*outb)(int port, int value);
	int (*printf)(char *fmt,...);
	Boolean (*ask_retry)();
	UBYTE port;
} Widge_hostlib;

/* the whole thing as declared in widget code */

typedef struct widge_rexlib {
	Widgelib wl;
	void *code;
	Widge_hostlib hl;
} Widge_rexlib;

#endif /* WIDGET_INTERNALS */

#endif /* WIDGET_H Leave at end of file */
