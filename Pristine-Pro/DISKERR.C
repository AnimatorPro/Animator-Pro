
/* diskerr.c - Just make it so dos don't put up that abort/retry/fail
   message on my pretty screen.  Automatically fail.  I'll cope
   elsewhere from there.  Actually this is about the only Turbo C specific
   library routine I'm using.  Still yet another good reason not
   to switch to Microsoft! */

#include "stdtypes.h"

static int de_handler(int errval, int ax, int bp, int si)
{
return(0);		/* Give up DOS, but please don't abort me... */
}

void init_de(void)
/* Have TurboC library deal with telling DOS what to do with it's
   critical disk error handler. */
{
harderr(de_handler);
}

#ifdef __WATCOMC__
static void harderr(FUNC handler)
{
}
#endif

