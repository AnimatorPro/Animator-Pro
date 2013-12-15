/*****************************************************************************
 * LIMITS.H		Compiler datatype limits for Poco.
 *
 *				Note that because Poco currently lacks full support for 
 *				unsigned datatypes, the unsigned MAX values are the same as  
 *				the signed MAX values.
 *
 * 11/22/90		Created.
 ****************************************************************************/

#ifndef __POCO__
#error This LIMITS.H file is for use with POCO C only!
#endif

#ifndef LIMITS_H
#define LIMITS_H

#define MB_LEN_MAX  1
#define CHAR_BITS	8
#define CHAR_MAX	127
#define CHAR_MIN	(-128)
#define SCHAR_MAX   127
#define SCHAR_MIN	(-128)
#define UCHAR_MAX	127
#define SHRT_MAX	32767 
#define SHRT_MIN	(-32768)          
#define USHRT_MAX	32767  
#define INT_MAX		2147483647
#define INT_MIN		(-2147483648)
#define UINT_MAX	2147483647
#define LONG_MAX	2147483647
#define LONG_MIN	(-2147483648)
#define ULONG_MAX	2147483647

#endif
