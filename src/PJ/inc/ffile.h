#ifndef FFILE_H
#define FFILE_H

#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

/** note: requires inclusion of <stdio.h> */

/* these are calls directed to stdio routines that use our errcode conventions
 and have some niceties we want */

void ffclose(FILE **pfp);
Errcode ffopen(char *path, FILE **pfp, char *fmode);
Errcode ffread(FILE *fp,void *buf,LONG size);
Errcode ffreadoset(FILE *fp,void *buf, LONG oset,LONG size);
Errcode ffwrite(FILE *fp,void *buf,LONG size);
Errcode ffwriteoset(FILE *fp,void *buf, LONG oset,LONG size);
LONG fftell(FILE *fp);
Errcode ffseek(FILE *fp,LONG oset,int how);
#define ffgetc(fp) fgetc(fp)

extern Errcode pj_errno_errcode(void);

#endif /* FFILE_H */
