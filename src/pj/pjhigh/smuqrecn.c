#include "errcodes.h"
#include "jimk.h"
#include "ftextf.h"
#include "reqlib.h"
#include "softmenu.h"

typedef struct udd {
	Errcode (*update)(void *uddat, SHORT val);
	void *uddat;
	SHORT min, max;
} Udd;

static Errcode clip_udat(void *dat, SHORT val)
{
	Udd *ud = dat;
	if (val < ud->min)
		val = ud->min;
	if (val > ud->max)
		val = ud->max;
	ud->update(ud->uddat,val);
	return Success;
}
Boolean clip_soft_qreq_number(short *inum,short min,short max, 
		Errcode (*update)(void *data, SHORT val), void *vfuncdat,
		char *key, ...)
/* Force number returned by number requestor to be between min and max */
{
Boolean ret;
Udd ud;
va_list args;

	if(update)
	{
		ud.update = update;
		ud.uddat = vfuncdat;
		ud.min = min;
		ud.max = max;
		update = clip_udat;
	}
	va_start(args,key);
	for(;;)
	{
		if((ret = vsoft_qreq_number(inum,min,max,key,args,update,&ud))!=FALSE)
		{
			if (*inum < min)
				*inum = min;
			else if (*inum > max)
				*inum = max;
			else
				break;
			softerr(Err_nogood,"!%d%d","outa_range", min, max);
		}
		else
			break;
	}
	va_end(args);
	return(ret);
}
