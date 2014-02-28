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
Errcode rcompact(void); /* Defrag ram-disk and release blocks not in use */
long rdos_dfree(void); /* Amount ram-disk has to go */

typedef struct rdir
	{
	struct rdir *next;
	char *name;
	long size;
	short omode;
	Boolean open;
	} Rdir;

Rfile ropen(const char *name, int mode);
Rfile rcreate(const char *name, int mode);
Errcode rclose(Rfile t);
long rread(Rfile t, void *buf, long count);
long rwrite(Rfile t,  void *buf, long count);
long rseek(Rfile t, long offset, int mode);
long rtell(Rfile t);
Errcode rdelete(const char *name);
void rstats(long *alloc, long *free);
Errcode rerror(void);
Errcode rexists(const char *name);
Errcode rrename(const char *old, const char *new);
Errcode rget_dir(Rdir **plist);
void rfree_dir(Rdir **pdir);

extern void rdisk_set_max_to_avail(void);

#endif /* RFILE_H */
