/****************************************************************************
 * CTYPE.H    	Character classification and conversion
 *
 *              This header file is coded such that it gives better
 *				performance than the CTYPE.H file that comes with most C
 *			    compilers, mainly by eliminating an addition operation for
 *				each use of an iswhatever(c) function. (Most CTYPE systems
 *				use [c+1] to handle EOF, we handle it better.)
 *
 *				This file also provides a 'roll your own' typing function,
 *				'isctype()'.  This macro allows testing of any arbitrary
 *				combo of types.  For example, to check whether a character
 *				is either space or punctuation, code 'isctype(c, _CTp|_CTs)'.
 *
 *				To use this file with non-Poco compilers, it is necessary to
 *				define the symbol GENERATE_CTYPE_TABLE (before the #include 
 *				for this file) in at least one of the modules that uses this
 *				file.
 *
 * 10/30/90 - 	Ported to poco.
 * 11/22/90 - 	Tested extensively, fixed a couple typos causing wrong results.
 *            	Also, added a couple #ifdef sections so that this file will
 *			  	work with any compiler, not just poco.
 ***************************************************************************/

#ifndef CTYPE_H
#define CTYPE_H
#define _CTYPE_H_INCLUDED			/* for Watcom */

#if defined(__POCO__) || defined(GENERATE_CTYPE_TABLE)

signed char ___pctype[257] = {
	0x00, /* EOF */
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x90, 0x90, 0x90, 0x90, 0x90, 0x80, 0x80,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x10, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
	0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42,
	0x42, 0x42, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
	0x20, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x04,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x04, 0x04, 0x04, 0x20, 0x20, 0x20, 0x20, 0x21,
	0x20, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x08,
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
	0x08, 0x08, 0x08, 0x20, 0x20, 0x20, 0x20, 0x80,
	};

signed char *__pctype = &___pctype[1];

#else

extern signed char *__pctype;

#endif /* __POCO__ or GENERATE_CTYPE_TABLE */

#define _CTb	0x0001 /* underbar, used by iscsym() */
#define _CTd    0x0002 /* numeric digit */
#define _CTu    0x0004 /* upper case */
#define _CTl    0x0008 /* lower case */
#define _CTs    0x0010 /* whitespace */
#define _CTp    0x0020 /* punctuation */
#define _CTx    0x0040 /* hexadecimal */
#define _CTc    0x0080 /* control character */

#undef  _toupper
#undef  _tolower
#undef  toascii
#define _toupper(c)  ((c)&0xFFDF)
#define _tolower(c)  ((c)|0x0020)
#define toascii(c)   ((c)&0x7F)

#define isascii(c)   (!((c)&0x0080))
#define isalnum(c)   (__pctype[(c)]&(_CTu|_CTl|_CTd))
#define isalpha(c)   (__pctype[(c)]&(_CTu|_CTl))
#define iscntrl(c)	 (__pctype[(c)]&_CTc)
#define isdigit(c)	 (__pctype[(c)]&_CTd)
#define isgraph(c)   (__pctype[(c)]&(_CTd|_CTu|_CTl|_CTp))
#define islower(c)	 (__pctype[(c)]&_CTl)
#define isprint(c)   (__pctype[(c)]>0)
#define ispunct(c)	 (__pctype[(c)]&_CTp)
#define isspace(c)	 (__pctype[(c)]&_CTs)
#define isupper(c)	 (__pctype[(c)]&_CTu)
#define isxdigit(c)  (__pctype[(c)]&_CTx)

#define iscsym(c)	 (__pctype[(c)]&(_CTu|_CTl|_CTb|_CTd))
#define iscsymf(c)   (__pctype[(c)]&(_CTu|_CTl|_CTb))
#define isctype(c,t) (__pctype[(c)]&(t)) /* roll your own combo */

#if defined(__POCO__) || defined(GENERATE_CTYPE_TABLE)
  int toupper(int c) {if (islower(c)) return c&0xFFDF; else return c;}
  int tolower(int c) {if (isupper(c)) return c|0x0020; else return c;}
#else
  extern int toupper(int);
  extern int tolower(int);	
#endif

#endif
