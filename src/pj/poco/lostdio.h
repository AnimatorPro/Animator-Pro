/* lostdio.h - in PJ proper we use our own version of fopen() fread(), etc,
   that may be working on files that actually are chuncks of ram.  Rather
   than drag in all that code and make it work in real mode under Turbo,
   this file makes the Turbo version use the normal stdio stuff. */

#ifndef LOSTDIO_H
#define LOSTDIO_H
	#include "lstdio.h"
#endif /* LOSTDIO_H */
