#ifndef STDDEF_H
#define STDDEF_H

#ifndef _SIZE_T_DEFINED_
	#define _SIZE_T_DEFINED_
	typedef unsigned int size_t;
#endif

#ifndef _WCHAR_T_DEFINED_
	#define _WCHAR_T_DEFINED_
	typedef unsigned short wchar_t;
#endif

#ifndef NULL
	#define NULL	0L
#endif

typedef int	ptrdiff_t;

#define offsetof(typ,id) (size_t)&(((typ*)0)->id)

#endif /* STDDEF_H */
