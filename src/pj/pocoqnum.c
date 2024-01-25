/* pocoqnum.c - a dupe of the qnumber routine with an update function */

#include "jimk.h"
#include "errcodes.h"
#include "pocoface.h"
#include "pocolib.h"

typedef struct upddata {
	void *code;
	Popot *ppdat;
} Upddat;

static Errcode ppupdate(Upddat *udd, SHORT value)
{
Pt_num ret;
int ival;

	if(udd->code == NULL)
		return(Success);

	ival = value;
	if((builtin_err = poco_cont_ops(udd->code, &ret, 
							    (sizeof(Popot)+sizeof(ival)), *udd->ppdat,
								ival )) < Success)
	{
		return(builtin_err);
	}
	return(ret.i);
}

Boolean po_UdSlider(long vargcount, long vargsize, 
					Popot inum, int min, int max,
				    Popot update,  Popot data, Popot pofmt, ...)
/*****************************************************************************
 * Boolean UdQnumber(int *num, int min, int max,  
 				     Errcode (*update)(void *data, int num),
 				     void *data, char *fmt, ...)
	 Will abort requestor if update returns < Success 
 ****************************************************************************/
{
short num;
Boolean cancel;
Boolean mouse_was_on;
Upddat udd;
va_list args;
char *fmt;

	va_start(args, pofmt);

	if((fmt = pofmt.pt) == NULL)
		fmt = "";

	if(Success > po_check_formatf(0, vargcount, vargsize, fmt, args))
		return(builtin_err);

	if (inum.pt == NULL)
		return(builtin_err = Err_null_ref);
	num = *((int *)(inum.pt));

	if ((num < SHRT_MIN) ||
		(min < SHRT_MIN) ||
		(max < SHRT_MIN) ||
		(num > SHRT_MAX) ||
		(min > SHRT_MAX) ||
		(max > SHRT_MAX) ||
		(min > max))
		return(builtin_err = Err_parameter_range);

	mouse_was_on = show_mouse();

	if ((udd.code = update.pt) != NULL)
	{
		udd.code = po_fuf_code(udd.code);
		udd.ppdat = &data;
	}

	if(FALSE != (cancel = varg_qreq_number(&num,min,max,ppupdate,&udd,
										   NULL,fmt,args)))
	{
		*((int *)(inum.pt)) = num;
	}


	if(!mouse_was_on)
		hide_mouse();
	return(cancel);
}
