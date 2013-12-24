
/* macro.c - Stuff to help record user input (usually first into a temp file)
   and then play it back without c_input() routine suspecting too much. */

#define INPUT_INTERNALS
#include <stdarg.h>
#include "ptrmacro.h"
#include "errcodes.h"
#include "input.h"
#include "idriver.h"
#include "vmagics.h"
#include "jfile.h"
#include "lstdio.h"
#include "ffile.h"
#include "jimk.h"
#include "commonst.h"
#include "softmenu.h"

#define POLL_INTERVAL 100 /* abort polling interval 1/10th second */


typedef struct machead  {
	Chunk_id id;
	BYTE realtime;
	PADTO(32,machead,padfill);
} Machead;


/* the macro record types ALL macro records start with a USHORT if
 * the sign bit is off the record is an input event record if on it is 
 * another type of macro record The sign bit is defined as MACRO_REC in 
 * input.h */

/* MACRO_REC record size is in the first 8 bits of the USHORT
 * this is the size of data following the SHORT word. the record type
 * is in the 0x0F bits allowing 16 types 0x40, 0x20, and 0x10
 * may be used by the record itself */

#define MR_TYPES 0x0F
#define MR_POLLABORT 0
#define MR_EOF		 0xF	/* written at end of file */

#define IS_MRTYPE(flags,type) \
	(((flags)&(MACRO_REC|MR_TYPES))==(MACRO_REC|(type)))

/* flags for abort polling control record this will give you the number of 
 * nest levels that this abort occurred at from 1 to 7  for now only 6 levels
 * are permissable If we need more I guess we could make a 7 == next byte
 * is level of abort but for now 7 is enough */

#define AR_ABORTLEVEL 0x70U /* this rec defines a user abort and has a count */
#define ABORTLEVEL_1  0x10U /* 1 bit of abortlevel */

typedef union twobytes {
	USHORT flags;
	UBYTE b[2];
} Twobytes;


typedef struct abrec {
    UBYTE flags;	 /* flags for current abort record */
	SHORT counts[7]; /* up to seven levels of counts startin with 
					  * lowest level */
	USHORT next;	 /* extra short for read beyond */
} Abortrec;

typedef struct maccb {
	Machead mh;     /* macro file header record */

	FILE *fp;       /* macro file pointer */
	LONG sizeleft;  /* count down to end of file */
	SHORT repeats;  /* number of repeat plays of macro */

	Twobytes next; /* prefetched flags word for next macro record */

	/* input event polling atom control */

	ULONG iocount;  /* input poll count left or accumulated */
	USHORT ioflags; /* flags for current input record in buffer */
	char mbuf[28];  /* buffer to contain input macro records */
	void *macbuf;   /* pointer to current record byte after flags */

	Short_xy lastmouse; /* last mouse read or written */
	USHORT last_histate; /* last state high word bits */
	USHORT last_pressure; /* last input pressure */ 

	/* abort record */

	Abortrec ar;    /* current abort record if read */
	BYTE mab_level; /* abort nest level read from macro record */
	BYTE ab_level;  /* abort nest level in program */

} Maccb;

static Maccb Mcb;

/**** abort poll recursive structure ***/

typedef struct abnest {
	SHORT count;
	SHORT nest;
	struct abnest *push;
	struct abnest *pop;
} Abortnest;

struct _error_checker_ {
	char ec0[sizeof(Abortbuf) == sizeof(Abortnest)]; /* will error if != */
	char ec1[PRESSURE_MAX == (UBYTE)PRESSURE_MAX]; /* must fit in a byte! */
};

static Abortnest abort;

/***************************/
/* If the input (not a MACRO_REC) record is a hybred of input flags and 
 * some reused input flags defining a variable length record 
 * containing variable combination of several items 
 * items dependent on flags values */

#define MACSTATE (~(BDOWNBITS<<BUPBIT1)) /* state flags saved in macro
										  * ( all but button up flags these 6
										  *   bits we can use for other things)
										  */

/* if KEYHIT is set in MACFLAGS then a USHORT key value is present */

/* event MAY have a non 0, check count and has previous state flags in first
 * short word after initial state flags */

#define HAS_CCOUNT  (1<<(BUPBIT1+0))

/* this is only in the previous state flags, if set this means the count is 
 * a ULONG or USHORT and not a UBYTE */

#define CCOUNT_LONG HAS_CCOUNT
#define CCOUNT_SHORT PAUSE_WAIT 

#define PAUSE_WAIT	 (1<<(BUPBIT1+1)) /* before this event is executed we
									   * are to pause and wait for a key hit */

#define HAS_PRESSURE (1<<(BUPBIT1+2)) /* event has UBYTE of pressure value */
#define HAS_MOUSEX   (1<<(BUPBIT1+3)) /* event has SHORT of mouse X coor */
#define HAS_MOUSEY   (1<<(BUPBIT1+4)) /* event has SHORT of mouse Y coor */
#define HAS_HISTATE (1<<(BUPBIT1+5)) /* event has high state bits */

#define HISTATE_WORD(state) (((USHORT*)&(state))[1])
#define HISTATE_BITS(state) (HISTATE_WORD(state)&(((ULONG)HI_BSTATES)>>16))

#ifdef DEBUG
mbox(char *fmt,...)
{
va_list args;

	if(Mcb.fp)
	printf("ct %d p %d ", Mcb.iocount, fftell(Mcb.fp));
	va_start(args,fmt);
	vprintf(fmt,args);
	va_end(args);
	printf("\n");
}
#endif

void close_macro(void)
{
BYTE olevel;
static UBYTE eoflags[2] = {(MACRO_REC|MR_EOF),(MACRO_REC|MR_EOF)};

	if(Mcb.fp != NULL)
	{
		if(icb.macro_mode == MAKE_MACRO)
		{
			Mcb.mh.id.size = fftell(Mcb.fp);
			ffwrite(Mcb.fp,&eoflags,sizeof(eoflags)); /* two bytes of eof */
			ffwriteoset(Mcb.fp,&Mcb.mh,0,sizeof(Mcb.mh));
		}
		ffclose(&Mcb.fp);
	}

	/* clear all but abort nesting level count */
	olevel = Mcb.ab_level;
	clear_struct(&Mcb);
	Mcb.ab_level = olevel;

	icb.macro_clocked = 0;
	icb.macro_mode = 0;
}
static create_macro(char *path, Boolean realtime)
{
Errcode err;

	close_macro(); /* will re-initialize everything to zeros */

	if((err = ffopen(path,&Mcb.fp,"wb+")) < Success)
		goto error;

	Mcb.mh.id.type = MAC_MAGIC;

	if((err = ffwrite(Mcb.fp,&Mcb.mh,sizeof(Mcb.mh))) < Success)
		goto error;

	icb.macro_clocked = Mcb.mh.realtime = realtime;
	icb.macro_mode = MAKE_MACRO;

	/* these values should force recording of data the first time */
	Mcb.lastmouse.x = Mcb.lastmouse.y = -30000; /* values highly unlikely */ 
	Mcb.last_histate = ~0U; /* value that won't exist */
	Mcb.last_pressure = ~0; /* valure that won't exist */

	return(Success);
error:
	close_macro();
	pj_delete(macro_name);
	return(softerr(err,"!%s", "macro_create", path ));
}
static Errcode macro_write_error(Errcode err)
{
	close_macro();
	pj_delete(macro_name);
	return(softerr(err,"macro_write"));
}
static Errcode macro_read_error(Errcode err)
{
	close_macro();
	return(softerr(err,"macro_read"));
}
Errcode put_macro(Boolean ishit)
{
Errcode err;
char buf[36];
USHORT *oflags;
void *macbuf;
#define Flags (*((USHORT *)buf))

	if(!ishit) /* just checking increment check count */
	{
		++Mcb.iocount;
		return(Success);
	}
	macbuf = buf;
	VPUTVAL(macbuf,USHORT,icb.state & MACSTATE);

	if(Mcb.iocount > 0)
	{
		oflags = macbuf;
		*oflags = icb.ostate & MACSTATE;
		macbuf = OPTR(macbuf,sizeof(USHORT));

		if(Mcb.iocount <= 0x0FF)
		{
			VPUTVAL(macbuf,UBYTE,Mcb.iocount);
		}
		else if(Mcb.iocount <= 0x0FFFF)
		{
			VPUTVAL(macbuf,USHORT,Mcb.iocount);
			*oflags |= CCOUNT_SHORT;
		}
		else
		{
			VPUTVAL(macbuf,ULONG,Mcb.iocount);
			*oflags |= CCOUNT_LONG;
		}

		if(icb.lastsx != Mcb.lastmouse.x)
		{
			VPUTVAL(macbuf,SHORT,icb.lastsx);
			*oflags |= HAS_MOUSEX; 
		}
		if(icb.lastsy != Mcb.lastmouse.y)
		{
			VPUTVAL(macbuf,SHORT,icb.lastsy);
			*oflags |= HAS_MOUSEY; 
		}
		if(HISTATE_BITS(icb.ostate) != Mcb.last_histate)
		{
			VPUTVAL(macbuf,USHORT,HISTATE_BITS(icb.ostate));
			*oflags |= HAS_HISTATE;
		}
		Flags |= HAS_CCOUNT; 
	}
	Mcb.iocount = 0;

	if(Flags & KEYHIT)
	{
		VPUTVAL(macbuf,USHORT,icb.inkey);
	}
	if(icb.pressure != Mcb.last_pressure)
	{
		VPUTVAL(macbuf,UBYTE,icb.pressure);
		Mcb.last_pressure = icb.pressure;
		Flags |= HAS_PRESSURE; 
	}
	if(icb.sx != Mcb.lastmouse.x)
	{
		VPUTVAL(macbuf,SHORT,icb.sx);
		Flags |= HAS_MOUSEX; 
	}
	if(icb.sy != Mcb.lastmouse.y)
	{
		VPUTVAL(macbuf,SHORT,icb.sy);
		Flags |= HAS_MOUSEY; 
	}
	Mcb.lastmouse = *((Short_xy *)&icb.sx);

	if(HISTATE_BITS(icb.state) != Mcb.last_histate)
	{
		Mcb.last_histate = HISTATE_BITS(icb.state);
		VPUTVAL(macbuf,USHORT,Mcb.last_histate);
		Flags |= HAS_HISTATE;
	}

	if((err = ffwrite(Mcb.fp,buf,SIZE(buf,macbuf))) < Success)
		macro_write_error(err);

	return(err);

#undef Flags
}

static Errcode play_macro(int repeats)
{
Errcode err;

	close_macro();
	if((err = ffopen(macro_name,&Mcb.fp,rb_str)) < Success)
		goto error;

	if((err = ffread(Mcb.fp,&Mcb.mh,sizeof(Mcb.mh))) < Success)
		goto error;

	if(Mcb.mh.id.type != MAC_MAGIC || Mcb.mh.id.size < sizeof(Machead))
	{
		err = Err_bad_magic;
		goto error;
	}

	/* pre-fetch next record header flags */

	if((err = ffread(Mcb.fp,&Mcb.next,sizeof(Twobytes))) < Success)
		goto error;

	Mcb.sizeleft = Mcb.mh.id.size - (sizeof(Machead)+sizeof(USHORT));

	Mcb.lastmouse = *((Short_xy *)&icb.sx);
	icb.macro_mode = USE_MACRO;
	icb.macro_clocked = Mcb.mh.realtime;
	Mcb.repeats = repeats;
	return(Success);
error:
	close_macro();
	return(err);
}
static Errcode poll_macro_abort()

/**** poll to see if user wants to abort macro playback ****/
{
Errcode err;
ULONG time;
static ULONG last_time;
Icb_savebuf *pushed;
Icb_savebuf icb_save;
USHORT obutns;

	time = (ULONG)pj_clock_1000();
	if((time-last_time) < POLL_INTERVAL) /* unsigneds may wrap but that's ok */
		return(Success);

	last_time = time;
	if(pj_key_is())
	{
		pj_key_in();
	}
	else /* call driver direct and check for the JSTHIT(MBRIGHT) condition */
	{
		obutns = icb.idriver->buttons;
		(*icb.idriver->lib->input)(icb.idriver);
		if(!((obutns^icb.idriver->buttons) & icb.idriver->buttons & MBRIGHT))
			return(Success);
	}

	/* make sure icb is saved recursing or not */

	if(NULL == (pushed = check_push_icb())) 
		save_icb_state(&icb_save);

	icb.state |= icb.idriver->buttons & (MBPEN|MBRIGHT); /* to re-sync */ 
	icb.macro_mode &= ~MACRO_OK; /* NO NO NO we don't want macro input here */
	if(soft_yes_no_box("macro_abort"))
	{
		close_macro();
		err = Err_abort;
	}
	else
	{
		icb.macro_mode |= MACRO_OK;
		err = Success;
	}

	/* be sure we restore, if pushed, pop is done returning to _poll_input() */

	if(pushed == NULL) 
		restore_icb_state(&icb_save);

	return(err);
}
Errcode get_macro_input(void)
/* read and process macro into input stream */
{
int readsize;
Errcode err;
USHORT oflags;

	if((err = poll_macro_abort()) < Success)
		goto error;

	if(Mcb.macbuf == NULL) /* no input record in buffer read one in */
	{
		if(Mcb.sizeleft <= 0) /* end of macro file stream */
		{
			if(--Mcb.repeats > 0)
			{
				if((err = play_macro(Mcb.repeats)) < Success)
					goto error;
			}
			else
			{
				err = Err_abort;
				goto error;
			}
		}

		if(Mcb.next.b[0] & MACRO_REC) /* not an input record */
			goto reuse_last;

		/* retrieve record head and calculate record size to read */

		Mcb.ioflags = Mcb.next.flags;
		readsize = sizeof(USHORT); /* short for prefetch of next record head */

		if(Mcb.ioflags & HAS_CCOUNT) /* get previous io state and count */
		{
			if((err = ffread(Mcb.fp,&oflags,sizeof(USHORT))) < Success)
				goto error;

			Mcb.sizeleft -= sizeof(USHORT);

			if(oflags & CCOUNT_SHORT)
				readsize += sizeof(USHORT);
			else if(oflags & CCOUNT_LONG)
				readsize += sizeof(ULONG);
			else
				readsize += sizeof(UBYTE);

			if(oflags & HAS_MOUSEX)
				readsize += sizeof(SHORT); 
			if(oflags & HAS_MOUSEY)
				readsize += sizeof(SHORT); 
			if(oflags & HAS_HISTATE)
				readsize += sizeof(USHORT);
		}

		if(Mcb.ioflags & KEYHIT)
			readsize += sizeof(USHORT);
		if(Mcb.ioflags & HAS_PRESSURE)
			readsize += sizeof(UBYTE); 
		if(Mcb.ioflags & HAS_MOUSEX)
			readsize += sizeof(SHORT); 
		if(Mcb.ioflags & HAS_MOUSEY)
			readsize += sizeof(SHORT); 
		if(Mcb.ioflags & HAS_HISTATE)
			readsize += sizeof(USHORT);

		if((Mcb.sizeleft -= readsize) < 0)
			readsize -= sizeof(USHORT);	/* end of file, NO next record */

		if( readsize != 0 
			&& (err = ffread(Mcb.fp,Mcb.mbuf,readsize)) < Success)
		{
			goto error;
		}

		/* get last short in buffer, this is head of next rec or garbage if
		 * end of file (previous short before buffer) */

		Mcb.next.flags = *(USHORT *)&(Mcb.mbuf[readsize-sizeof(USHORT)]);

		Mcb.macbuf = Mcb.mbuf; /* flag record present, at start of buffer */

		if(Mcb.ioflags & HAS_CCOUNT)
		{
			/* add count to possibly decremented value and get count of
			 * polls left to do before using this record */

			if(oflags & CCOUNT_SHORT)
			{
				Mcb.iocount += VGETVAL(Mcb.macbuf,USHORT);
			}
			else if(oflags & CCOUNT_LONG)
			{
				Mcb.iocount += VGETVAL(Mcb.macbuf,ULONG);
			}
			else
			{
				Mcb.iocount += VGETVAL(Mcb.macbuf,UBYTE);
			}

			if((LONG)Mcb.iocount < 0) 
				/* oops, file corrupt or program outta sync */
			{
				err = Err_macrosync;
				goto error;
			}

			/* item with poll count has to provide previous input state
			 * to re-create recorded conditions in _poll_input() */

			icb.state = oflags & MACSTATE;

			if(oflags & HAS_MOUSEX)
			{
				icb.sx = VGETVAL(Mcb.macbuf,SHORT);
			}
			if(oflags & HAS_MOUSEY)
			{
				icb.sy = VGETVAL(Mcb.macbuf,SHORT);
			}
			if(oflags & HAS_HISTATE)
			{
				HISTATE_WORD(icb.state) |= VGETVAL(Mcb.macbuf,USHORT);
			}

			if(Mcb.iocount == 0) /* if count is zero we must put this
								  * iostate into the previous state 
								  * otherwise next poll will do this 
								  * in poll input */
			{
				SET_BUPBITS(icb.state);
				icb.ostate = icb.state & ALL_BSTATES;
				get_menucursorxy(); /* recalc previous values as current */
				ICB_COPYTO_LAST();  /* put them in last buffer */
			}
		}
	}

	if(Mcb.iocount > 0) /* we have record, but keep polling */
		goto reuse_last;

	/* count satisfied, let user have recorded input */

	icb.state = Mcb.ioflags & MACSTATE;

	if(icb.state & KEYHIT)
	{
		icb.inkey = VGETVAL(Mcb.macbuf,USHORT);
	}
	else
		icb.inkey = 0;

	if(Mcb.ioflags & HAS_PRESSURE)
	{
		icb.pressure = VGETVAL(Mcb.macbuf,UBYTE);
	}
	else
		icb.pressure = PRESSURE_MAX;

	if(Mcb.ioflags & HAS_MOUSEX)
	{
		Mcb.lastmouse.x = VGETVAL(Mcb.macbuf,SHORT);
	}
	if(Mcb.ioflags & HAS_MOUSEY)
	{
		Mcb.lastmouse.y = VGETVAL(Mcb.macbuf,SHORT);
	}
	*((Short_xy *)&icb.sx) = Mcb.lastmouse;
	if(Mcb.ioflags & HAS_HISTATE)
	{
		Mcb.last_histate = VGETVAL(Mcb.macbuf,USHORT);
	}
	HISTATE_WORD(icb.state) |= Mcb.last_histate; 

	Mcb.macbuf = NULL; /* flag macro consumed no longer present */
	return(Success);

reuse_last:

	--Mcb.iocount;			  /* decrement input poll counter */
	icb.state &= ALL_BSTATES; /* clear changed _poll_input() unique flags */
	return(Success);

error:
	return(macro_read_error(err));
}
void qsave_macro(void)
{
char *title;
char buf[50];

	close_macro();
	if (!pj_exists(macro_name))
		soft_continu_box("no_macro");
	else
	{
		if ((title = vset_get_filename(stack_string("save_mac",buf),
				".REC", save_str,MACRO_PATH,NULL,1))!= NULL)
		{
			if (overwrite_old(title))
				{
				pj_copyfile(macro_name, title); /* < 0 is bad */
				}
		}
	}
}
void qload_macro(void)
{
Errcode err;
Machead mh;
char *title;
Jfile f;
char buf[50];

	if ((title =  vset_get_filename(stack_string("load_mac",buf),
		".REC", load_str,MACRO_PATH,NULL,1)) == NULL)
	{
		return;
	}

	close_macro();

	if ((f = pj_open(title, JREADONLY)) == JNONE)
	{
		err = pj_ioerr();
		goto error;
	}
	if((err = pj_read_ecode(f,&mh,(long)sizeof(mh))) < Success)
	{
		if(err == Err_eof)
			err = Err_truncated;
		goto error;
	}
	if(mh.id.type != MAC_MAGIC)
	{
		err = Err_bad_magic;
		goto error;
	}
	pj_close(f);
	f = JNONE;
	err = pj_copyfile(title, macro_name);
error:
	pj_close(f);
	softerr(err,"!%s", "macro_load", title);
}

void qstart_macro(void)
{
	create_macro(macro_name,FALSE);
}
void qrealtime_macro(void)
{
	create_macro(macro_name,TRUE);
}
void qclose_macro(void)
/* returns TRUE if macro is actually closed FALSE if macro is currently
 * running */
{
	if((icb.macro_mode|MACRO_OK) == USE_MACRO)
		return;
	close_macro();
}
void quse_macro(void)
{
	if((icb.macro_mode|MACRO_OK) == USE_MACRO)
		return;
	softerr(play_macro(1),"macro_use");
}
void qrepeat_macro(void)
{
SHORT reps;

	if((icb.macro_mode|MACRO_OK) == USE_MACRO)
		return;
	reps = flix.hdr.frame_count-1;
	if(!soft_qreq_number(&reps,1,100,"macro_repeat"))
		return;
	softerr(play_macro(reps),"macro_use");
}

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
static Errcode write_abort_rec()
{
Errcode err;
UBYTE buf[32]; /* for now only 7 levels possible */
void *abuf;
Abortnest *pan;

	buf[0] = MACRO_REC|MR_POLLABORT;
	abuf = &buf[1];

	if(Mcb.ar.flags & AR_ABORTLEVEL) /* we are aborting from here */
	{
		pan = &abort;
		while(pan != NULL)
		{
			VPUTVAL(abuf,SHORT,pan->count);
			buf[0] += ABORTLEVEL_1;
			pan = (void *)(pan->pop);
		}
	}
	if((err = ffwrite(Mcb.fp,&buf,SIZE(buf,abuf))) < Success)
		return(macro_write_error(err));
	return(Success);
}
static Errcode read_abort_rec()
{
Errcode err;
int readsize;

	Mcb.ar.flags = Mcb.next.b[0];
	if(0 != (readsize = Mcb.ar.flags & AR_ABORTLEVEL))
	{
		((UBYTE *)(&Mcb.ar))[1] = Mcb.next.b[1];

		/* readsize = num levels counts - one byte + sizeof next */

		Mcb.mab_level = (readsize /= ABORTLEVEL_1) - 1;
		readsize = (readsize * sizeof(USHORT)) - 1 + sizeof(USHORT);

		if((err = ffread(Mcb.fp,OPTR(Mcb.ar.counts,1),readsize)) < Success)
			return(macro_write_error(err));

		Mcb.sizeleft -= readsize;
		Mcb.next.flags = *((USHORT *)(OPTR(&Mcb.ar,readsize)));

	}
	else /* one byte non abort poll end record move last byte
		  * to first and fetch next byte in file */
	{
		Mcb.next.b[0] = Mcb.next.b[1];
		Mcb.next.b[1] = ffgetc(Mcb.fp);
		--Mcb.sizeleft; 
	}
	return(Success);
}
void pstart_abort_atom(Abortbuf *sbuf)

/* will set savenest to sbuf. if savenest is non NULL it will PUSH the old atom
 * count and start a new count. This means a non NULL sbuf will
 * cause any subsequent nested abort atoms to be pushed one level
 * below and be considered a single entity to this atom with it's own variable
 * poll count only one abort may take place in a nested abort poll sequence,
 * and the number of input polls within the nested sequence must be 
 * invariant between record and playback of a macro file */
{
	if(abort.push)
	{
		*abort.push = abort;
		abort.pop = abort.push;
		abort.nest = 0;
		++Mcb.ab_level;
	}
	abort.push = (void *)sbuf;
	if(abort.nest++ > 0)
		return;
	if(!abort.pop) /* we are at top level of nesting */
		Mcb.ar.flags = 0; /* re init to empty record */
	abort.count = 0; /* start count over */
}
void start_abort_atom(void)
/* saves putting NULLs all over */
{
	pstart_abort_atom(NULL);
}

Errcode end_abort_atom()
{
SHORT *pcount;
Errcode err;
Abortnest *an;

	if(--abort.nest != 0)
		return(Success);

	err = Success;
	if(icb.macro_mode == MAKE_MACRO)
	{
		if(!abort.pop && !(Mcb.ar.flags & AR_ABORTLEVEL))
		{
			/* this nested abort was not aborted and record 
			 * not written by poll abort 
			 * write out non abort byte without an input poll */

			if( write_abort_rec() < Success)
				goto abort_error;
		}
	}
	else if(icb.macro_mode == USE_MACRO)
	{
		if(!Mcb.ar.flags) 
		{
			if(IS_MRTYPE(Mcb.next.b[0],MR_POLLABORT))
			{
				if(read_abort_rec() < Success)
					goto abort_error;
			}
			else if(!abort.pop) /* back to top level and didn't find it yet */
			{
				macro_read_error(Err_macrosync);
				goto abort_error;
			}
		}

		/* if aborted and level >= macro see if this atom was aborted */

		if(Mcb.ar.flags & AR_ABORTLEVEL 
			&& Mcb.ab_level <= Mcb.mab_level)  
		{
			pcount = &Mcb.ar.counts[Mcb.mab_level - Mcb.ab_level];
			for(an = abort.pop;an != NULL;an = an->pop)
			{
				if(*pcount++ > an->count)
					goto not_aborted;
			}
			_poll_input(0); /* read the unread stored abort input */
			goto abort_error;
		}
	}
	goto not_aborted;
abort_error:
	if(verify_abort()) /* if this has been recorded it will be TRUE */
		err = Err_abort;
not_aborted:

	if(abort.pop) /* if pushed pop it */
	{
		abort = *abort.pop;
		--Mcb.ab_level;
	}
	else
		abort.push = NULL; /* terminate nest if not popped */

	return(err); /* for program macro error will be an abort hit */
}
Errcode errend_abort_atom(Errcode err)

/* like end poll atom but passes through errcode unless error is ok and 
 * poller returns ABORT */

{
	if(err >= Success)
		return(end_abort_atom());
	end_abort_atom();
	return(err);
}

Errcode poll_abort()
/* use whenever you wish to poll if user has requested an abort using a 
 * key hit or right pen click */
{
Errcode ret;
ULONG time;
static ULONG last_time;
BYTE macro_mode;
SHORT *pcount;
Abortnest *an;

	macro_mode = icb.macro_mode;
	ret = Success; /* non abort most common case */

	if(abort.nest > 0)
	{
		switch(macro_mode)
		{
			case MAKE_MACRO:
			{
				if(Mcb.ar.flags & AR_ABORTLEVEL) /* only one abort per atom */
					goto done;
				icb.macro_mode &= ~MACRO_OK; /* dont record polling input */ 
				++abort.count;       /* one more poll */
			}
			case USE_MACRO:
			{
				if(poll_macro_abort() < Success)
					goto error;

				++abort.count;
				if(!Mcb.ar.flags) /* havnt read abort rec yet */
				{
					if(!IS_MRTYPE(Mcb.next.b[0],MR_POLLABORT))
						goto done; /* not yet, go on */
					if(read_abort_rec() < Success)
						goto error;
					goto done;
				}

				if(!(Mcb.ar.flags & AR_ABORTLEVEL))
					goto done;

				if(Mcb.ab_level != Mcb.mab_level) /* not on aborted level */
					goto done;

				/* if all counts match counts on all levels ABORT! */
				pcount = Mcb.ar.counts;
				for(an = &abort;an != NULL;an = an->pop)
				{
					if(*pcount++ != an->count) /* no match */
						goto done; 
				}

				Mcb.ar.flags &= ~AR_ABORTLEVEL;  /* took care of this */
				_poll_input(0); /* get input that was recorded */
				verify_abort(); /* macro WILL abort, we don't record cancels */
				goto aborted;
			}
			default:
				break;
		}
	}

	time = (ULONG)pj_clock_1000();
	if(macro_mode == USE_MACRO || (time-last_time) > POLL_INTERVAL)
	{
		last_time = time;
		icb.waithit = KEYHIT|MBRIGHT;
		if(0 == (icb.waithit = _poll_input(0))) /* will clear waithit */
			goto done;
		icb.waithit = 0;

		if(macro_mode == MAKE_MACRO && abort.nest > 0)
		{
		LONG foffset;

			if((foffset = fftell(Mcb.fp)) < 0)
			{
				macro_read_error((Errcode)foffset);
				goto error;
			}

			/* flag to write record for abort at this level */ 
			Mcb.ar.flags |= AR_ABORTLEVEL; 
			if(write_abort_rec() < Success)
				goto error;
			if(put_macro(TRUE) < Success) /* write input that caused abort */
				goto error;
			if(verify_abort())
				goto aborted;

			/* Abort canceled! disregard aborting an cancelling input 
			 * unflag abort done */

			Mcb.ar.flags &= ~AR_ABORTLEVEL; 
			if((foffset = ffseek(Mcb.fp,foffset,SEEK_SET)) < 0)
			{
				macro_read_error(foffset);
				goto aborted;
			}
		}
		else if(verify_abort()) 
			goto aborted;
	}
	else if(icb.macro_mode == MAKE_MACRO) 
	{
		/* always increment macro count if making a macro.
		 * NOTE: that above we always check the input when using
		 * a macro and macro will always be ~MACRO_OK when polling in
		 * an abort atom NOTE: put_macro with FALSE will never return 
		 * error */

		put_macro(FALSE);
	}
	goto done;

error:
	if(!verify_abort()) /* error will be reported already */
		goto done;
aborted:
	ret = Err_abort;
done:
	icb.macro_mode |= MACRO_OK;
	return(ret);
}
