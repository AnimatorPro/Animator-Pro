/*****************************************************************************
 * STDDEF.H		Standard type definitions.
 *
 *				Note a little strangeness in the offsetof() macro...We have
 *				to use a cast of NULL rather than the typical cast of 0, and
 *				we have to subtract the resulting member pointer from NULL.
 *				All of this is because Poco won't allow pointer/number casts.
 *
 * 11/22/90		Created for Poco.
 ****************************************************************************/

#ifndef STDDEF_H
#define STDDEF_H

#ifndef __POCO__
#error This STDDEF.H file is for use with Poco C only!
#endif

typedef long 	ptrdiff_t;
typedef long	size_t;
typedef char 	wchar_t;

#define offsetof(struc, mbr)  (size_t)( (char *)(&(((struc *)NULL)->mbr))-(char *)NULL )

#endif
