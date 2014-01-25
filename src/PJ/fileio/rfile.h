
#ifndef RFILE_H
#define RFILE_H

#ifndef ERRCODES_H
	#include "errcodes.h"
#endif /* ERRCODES_H */

#ifdef RFILE_C
typedef struct temp_file *Rfile;
#else
typedef void  *Rfile;
#endif /* RFILE_C */

long set_rmax(long rmax); /* Set maximum size of ram-disk. Returns rmax 
							clipped to internal limits. */
Errcode tcompact(); /* Defrag ram-disk and release blocks of ram not in use */
long rdos_dfree();	/* Amount ram-disk has to go */

typedef struct rdir
	{
	struct rdir *next;
	char *name;
	long size;
	short omode;
	Boolean open;
	} Rdir;

Rfile ropen(char *name, int mode);
Rfile rcreate(char *name, int mode);
Errcode rclose(Rfile t);
long rread(Rfile t, void *buf, long count);
long rwrite(Rfile t,  void *buf, long count);
long rseek(Rfile t, long offset, int mode);
long rtell(Rfile t);
Errcode rdelete(char *name);
void rstats(long *alloc, long *free);
Errcode rerror();
Errcode rexists(char *name);
Errcode rrename(char *old, char *new);
Errcode rget_dir(Rdir **plist);

Errcode rset_priority(char *name, unsigned pri);
Errcode rset_dispose_flags(char *name, int dflags);

#endif /* RFILE_H */
