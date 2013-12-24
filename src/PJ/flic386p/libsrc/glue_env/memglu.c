/*****************************************************************************
 * MEMGLU.C - Default memory allocators.
 *
 *	The application using the FLILIB can define custom memory allocators,
 *	and they will be used.	If the application doesn't supply custom routines,
 *	these routines will be linked, and will use the compiler's RTL allocators.
 *
 *	Note that if the client application wants to take over memory allocation
 *	done from within fliclib, all three of these functions must be coded
 *	within the client code!
 *
 *	If the client application is using -3r (parms in regs), it MUST #include
 *	PJSTYPES.H, and the three pragmas that appear below in the module must
 *	be coded in the module containing the replacement functions!
 *
 * Maintenance:
 *	11/27/91	This module can now be compiled with -3r or -3s option,
 *				depending on which version of the fliclib we're building.
 ****************************************************************************/

#include <stddef.h>
#include <malloc.h>
#include "pjstypes.h"

#pragma aux (FLICLIB3S) pj_malloc;
#pragma aux (FLICLIB3S) pj_zalloc;
#pragma aux (FLICLIB3S) pj_free;

void *pj_malloc(size_t amount)
{
	return malloc(amount);
}

void *pj_zalloc(size_t amount)
{
	return calloc(1, amount);
}

void pj_free(void *block)
{
	free(block);
}
