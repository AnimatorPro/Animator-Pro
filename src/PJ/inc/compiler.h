/*****************************************************************************
 * COMPILER.H - Fine tune options and standardize macros between compilers.
 *
 *	This is a good place to put #pragma statements and other information
 *	specific to a given compiler.  Every module in PJ and all its subsystems
 *	and libraries will include this header file.
 *
 *	In addition to options, performance tuning statements, and so on, this
 *	header defines several macros which standardize operations between
 *	different compilers.  (IE, quirk-fixes.)  For example, a va_list type
 *	is an array-of-one under Watcom, but a scalar item under High C.  When
 *	you add a new compiler type, each of these macros must be defined as
 *	appropriate to your compiler.  Currently, these macros are:
 *
 *		NOFUNC			- A NULL pointer that works for function pointers.
 *		copy_va_list	- Copy a value from one va_list variable to another.
 *
 *	To add a new compiler, just clone one of the existing sections, change
 *	the compiler ID in the #if defined() statement, and add your options and
 *	quirk-fix macros.
 ****************************************************************************/

#ifndef COMPILER_H

/*****************************************************************************
 * Items specific to 386/protected mode, but not any given compiler...
 *
 *	norm_pointer is a function for the 86/286, but in protected mode with
 *	a flat address space it's unnecessary.
 ****************************************************************************/

#define norm_pointer(c)  ((void *)(c))

/* Compile time assertions. */
#define ASSERT_CONCAT_(a, b)    a##b
#define ASSERT_CONCAT(a, b)     ASSERT_CONCAT_(a, b)
#define STATIC_ASSERT(module, e) \
	struct ASSERT_CONCAT(static_assert_##module##_line_, __LINE__) \
		{ unsigned int bf : !!(e); }

#define GCC_PACKED

/*****************************************************************************
 * Watcom C/386 v8.0
 ****************************************************************************/

#if defined(__WATCOMC__)

/*----------------------------------------------------------------------------
 * compiler directives...
 *	put pragmas for options and performance tuning, etc, here.
 *--------------------------------------------------------------------------*/

#pragma off(unreferenced);					/* don't whine about unused     */
											/* function parameters. 		*/

#define PROTECTED							/* not sure what this is for.	*/

#ifndef _toupper
  #define _toupper(c)	((c) + 'A' - 'a')   /* these are missing from       */
  #define _tolower(c)	((c) + 'a' - 'A')   /* Watcom's current ctype.h     */
#endif

extern char *_STACKTOP; 					/* Watcom stack goodies...		*/
extern char *_STACKLOW; 					/* not sure what these are for, */
#define __STACK_R32__						/* suspect only for PJ internals*/

/*----------------------------------------------------------------------------
 * PJ-specific items...
 *	this section must appear for each compiler.  the macro definitions
 *	should be modified as needed for your compiler.
 *
 *	in Watcom, a function pointer can be the normal ((void*)0).
 *	macro to copy a va_list value assumes that va list is a char *va_list[1].
 *--------------------------------------------------------------------------*/

#define NOFUNC					((void*)0)
#define copy_va_list(src,dest)	{dest[0]=src[0];}

#include "pjinline.h" /* we can write better inlines than Watcom, include them */

/*****************************************************************************
 * Metaware High C v1.6
 ****************************************************************************/

#elif defined(__HIGHC__)

/*----------------------------------------------------------------------------
 * compiler directives...
 *	put pragmas for options and performance tuning, etc, here.
 *--------------------------------------------------------------------------*/

#pragma on(387);			/* ok to generate 80387 instructions			*/

#pragma on(floating_point); /* BUT, do not use non emulated 80387			*/
							/* instructions. If you KNOW this code will 	*/
							/* ONLY be used with an 80387 loaded machine	*/
							/* change this to 'on'.                         */

/*----------------------------------------------------------------------------
 * PJ-specific items...
 *	this section must appear for each compiler.  the macro definitions
 *	should be modified as needed for your compiler.
 *
 *	in High C, a function pointer can be 0, but not ((void*)0)
 *	macro to copy a va_list value assumes that va list is a char *va_list.
 *--------------------------------------------------------------------------*/

#define NOFUNC					0L
#define copy_va_list(src,dest)	{dest=src}


/*****************************************************************************
 * Borland/Turbo C (for poco development work only)
 ****************************************************************************/

#elif defined(__BORLANDC__) || defined(__TURBOC__)

/*----------------------------------------------------------------------------
 * compiler directives...
 *	put pragmas for options and performance tuning, etc, here.
 *--------------------------------------------------------------------------*/

/* nothing borland-specific needed right now */

/*----------------------------------------------------------------------------
 * PJ-specific items...
 *	this section must appear for each compiler.  the macro definitions
 *	should be modified as needed for your compiler.
 *
 *	in Watcom, a function pointer can be the normal ((void*)0).
 *	macro to copy a va_list value assumes that va list is a char *va_list[1].
 *--------------------------------------------------------------------------*/

#define NOFUNC					((void*)0)
#define copy_va_list(src,dest)	{dest=src;}


/* GCC, Clang. */
#elif defined(__GNUC__) || defined(__clang__)

#define NOFUNC                  ((void*)0)
#define copy_va_list(src,dest)  va_copy(dest,src)

#undef GCC_PACKED
#define GCC_PACKED  __attribute__((packed))


/*****************************************************************************
 * Unknown compiler, whine and die.
 ****************************************************************************/

#else
#error Unknown compiler type in COMPILER.H!!!
#endif

#endif /* COMPILER_H */
