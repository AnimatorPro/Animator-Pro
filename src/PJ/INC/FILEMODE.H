#ifndef FILEMODE_H
#define FILEMODE_H


#ifdef JFILE_INTERNALS

/*  We do this to hide file structure from clients while allowing
    jfile code to compile with same prototypes... */

typedef struct jfl *Jfile;

#else
typedef void *Jfile;
#endif /* JFILE_INTERNALS */

#define JNONE NULL  /* if open fails this is it */

/* seek parameters */
#define JSEEK_START	0 /* defines for mode parameter to jseek */
#define JSEEK_REL	1
#define JSEEK_END	2

/* Normal MS-DOS open flags for mode parameter to jcreate() and jopen() */
#define JUNDEFINED -1
#define JREADONLY 0
#define JWRITEONLY 1
#define JREADWRITE 2

#ifdef PRIVATE_CODE 
/* Mask  to restrict attention to only MS-DOS flags */
#define MSOPEN_MODES 3
#endif /* PRIVATE_CODE */


#endif /* FILEMODE_H */

