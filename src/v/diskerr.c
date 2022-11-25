/* diskerr.c - Just make it so dos don't put up that abort/retry/fail
   message on my pretty screen.  Automatically fail.  I'll cope
   elsewhere from there.  Actually this is about the only Turbo C specific
   library routine I'm using.  Still yet another good reason not
   to switch to Microsoft! */

#include "jimk.h"

int crit_errval;

static
de_handler(errval, ax, bp, si)
int errval, ax, bp, si;
{
crit_errval = errval;
return(0);		/* Give up DOS, but please don't abort me... */
}

/* Have TurboC library deal with telling DOS what to do with it's
   critical disk error handler. */
init_de()
{
harderr(de_handler);
}

