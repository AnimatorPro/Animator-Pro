/**** poll abort stuff that replaces the abort polling stuff in macro.c
      for the non macro input library *****/

#define INPUT_INTERNALS
#include "ptrmacro.h"
#include "errcodes.h"
#include "input.h"

#define POLL_INTERVAL 450 /* abort polling interval */

/********************* nested abort polling stuff ********************/
static Boolean (*_verify_abort)(void *dat);
void *_verify_dat;
void set_abort_verify(Boolean (*verify)(void *dat), void *dat)
{
	_verify_abort = verify;
	_verify_dat = dat;
}
static Boolean verify_abort(void)
{
	if(_verify_abort == NULL)
		return(TRUE);
	return((*_verify_abort)(_verify_dat));
}
void pstart_abort_atom(Abortbuf *sbuf)
{
	/* does nothing, just to allow macro code to use this input */
}
void start_abort_atom(void)
{
	/* does nothing, just to allow macro code to use this input */
}
Errcode end_abort_atom()
{
	return(Success);
}
Errcode errend_abort_atom(Errcode err)
{
	return(err);
}
Errcode poll_abort()
/* use whenever you wish to poll if user has requested an abort using a 
 * key hit or right pen click */
{
ULONG time;
static ULONG last_time;

	time = (ULONG)_getclock();
	if((time-last_time) > POLL_INTERVAL)
	{
		last_time = time;
		icb.waithit = KEYHIT|MBRIGHT;
		if(0 == (icb.waithit = _poll_input(0))) /* will clear waithit */
			goto done;
		icb.waithit = 0;
		if(verify_abort()) 
			return(Err_abort);
	}
done:
	return(Success);
}
