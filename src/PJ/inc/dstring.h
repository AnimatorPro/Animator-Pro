#ifndef DSTRING_H
#define DSTRING_H
/* dstring.h - dynamicly sized string handling.  Initially the string
 * will point to a smallish (currently 80) byte static buffer.  If additions
 * to the string overflow this then a piece of memory is allocated for it.
 * */
#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif 

struct lfile;

/* The main data structure is */
	#define DST_SMALL 80
	typedef struct dstring
		{
		char *buf;
		int bmax;
		int blen;
		Boolean is_dynamic;
		char sbuf[DST_SMALL];
		} Dstring;
/* Methods are */
	void dstring_init(Dstring *d);		/* constructor */
	void dstring_cleanup(Dstring *d);	/* destructor */
	Errcode dstring_addc(Dstring *ds, 	/* Append a character */
		char c);					/* char to append */
	Errcode dstring_memcpy(Dstring *ds, /* Copy buffer into dstring */
		char *s, 	/* buffer to copy */
		int len);	/* length of buffer */
	Errcode dstring_strcpy(Dstring *ds, /* Copy string into dstring */
		char *s);	/* 0 terminated string */
	Errcode dstring_memcat(Dstring *ds, /* Splice two dstrings */
		Dstring *ss);	/* Dstring to add at end */
	Errcode dstring_strcat(Dstring *ds, /* Splice zero terminated dstrings */
		Dstring *ss);	/* Dstring to add at end */
	Errcode dstring_newbuf(Dstring *ds,   /* get buffer for dstring */
		unsigned int newsize, 		/* size of buffer */
		Boolean copy_old);			/* copy old contents? */

	/* get clone at expense of dstring buffer */
	Errcode dstring_get_clone(Dstring *ds,char **ptext);

extern Errcode dstring_get_line(Dstring *ds, struct lfile *f);

#endif /* DSTRING_H */
