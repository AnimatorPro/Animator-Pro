#define WIDGET_INTERNALS
#include "stdtypes.h"
#include "ptrmacro.h"
#include "memory.h"
#include "widget.h"
#include "encrypt.h"
#include "errcodes.h"


#define REX_CODE static ULONG widget_code[]
#define REX_FIXUP static USHORT widget_fixup[]

#include "rexdata.c"

#undef REX_CODE
#undef REX_FIXUP

extern LONG pj_clock_1000(); /* gets millisecond clock */
extern Boolean widge_ask_retry(void);
extern int inp(int port);
extern int printf(char *fmt,...);
extern void *widge_self_free(Widge_rexlib **plib, void *bvector);

static void widge_outb(int port, int value);
static Errcode ret_enomem() { return(Err_no_memory);}
static Widge_rexlib *new_widgelib(void *bvector);
void init_widget(void *vector);
static void widge_freelib(Widge_rexlib **plib, void *bvector);

/* data */

Widge_rexlib widge_initlib; /* foreward reference */

Widge_rexlib *widge_funcs = &widge_initlib;  /* external for "bracket" calls */


Widge_rexlib widge_initlib = {
	{ 
		init_widget,
		ret_enomem,
	},
	NULL,  /* code */
	{
		&widge_funcs,
		NULL,
		widge_freelib,
		inp,
		widge_outb,
		printf,
		widge_ask_retry,
		0,   	/* port */
	}
};


static void widge_outb(int port, int value) 
/***** does outport for widget code *****/
/* 
 * TIMING NOTE : There is a 10 microsecond minimum delay value required to 
 *               to let the data and response lines to settle. */
{
static int delay_count = -1;
int delay;

	if(delay_count < 0)
	{
		delay_count = pj_clock_1000();
	    for (delay = 0; delay < 100000; delay++);
		delay_count = (pj_clock_1000() - delay_count);
		delay_count = 100000/(delay_count?delay_count*100:100);
	}
	outp(port,value);
    for (delay = 0; delay < delay_count; delay++);
}

static Widge_rexlib *new_widgelib(void *bvector)
/* This is the heart of the widget code security, I figure if it is always
 * moving around all over the place and the original is hidden it will be very
 * hard for any one to figure out what's happening or to modify it in a useful
 * manner. The code that this "Loads" from the static buffer can be found in
 * widgerex.c */
{
struct widge_entry {
	ULONG codesize;
	Widge_rexlib *lib;
} *rex;
USHORT *fixup;
USHORT *max_fixup;
Widge_rexlib *lib = &widge_initlib;
Cryptic_data cd;
ULONG size;
void *dummy_buf;

	/* pseudo random alloc size to make next alloc locate itself in different
	 * positions every time (Hopefully) */

	size = ((pj_clock_1000()<<2) & 0x3FFF) | 1;
	if((dummy_buf = pj_malloc(size)) == NULL) 
		goto out;

	/* allocate buffer for clandestine relocatable code. The first word
	 * is set in entry.asm to be the size to allocate */

	/* Decrypt code, I feel the crc check is unwarrented since one bit altered
	 * will sorely mess up the decription method, and most likely kaboom */

	init_cryptic(&cd,REX_CODE_KEY,widget_code,&size,TRUE);

	/* decrypt the size to alloc and alocate a buffer */

	crypt_bytes(&cd,sizeof(size));

	if((rex = (struct widge_entry *)pj_zalloc(size)) == NULL)
		goto out;

	/* decrypt the rest of the buffer */
	rex->codesize = size;
	cd.out_data = &(rex->lib);
	crypt_bytes(&cd,sizeof(widget_code)-sizeof(size));

#ifdef NOT_ENCRIPTED
	if((rex = (struct widge_entry *)pj_zalloc(widget_code[0])) == NULL)
		goto out;

	/* copy in code */
	memcpy(rex,&widget_code[0],sizeof(widget_code));

	/* this will tell us if original code is messed with */
	crc = mem_crcsum(rex,sizeof(widget_code));

#endif /* NOT_ENCRYPTED */

	/* fixup addresses in relocatable code */

	fixup = &widget_fixup[0];
	max_fixup = OPTR(fixup,sizeof(widget_fixup));

	while(fixup < max_fixup)
	{
		*((ULONG *)(OPTR(rex,*fixup))) += *((ULONG *)(&rex));
		++fixup;
	}

	/* get pointer to "library" in new clandestine code 
	   and link in stuff code needs from the global environment */

	lib = rex->lib;
	lib->code = rex;
	lib->hl = widge_initlib.hl;
	lib->wl.init_bracket(bvector); /* set bracket vector via clandestine call */
out:
	pj_gentle_free(dummy_buf);
	return(lib);
}
static void widge_freelib(Widge_rexlib **plib, void *bvector)
{
Widge_rexlib *wl;

	if((wl = *plib) != NULL && (wl)->code != NULL)
	{
		widge_initlib.hl.port = wl->hl.port; /* save port found by rex code */
		pj_free((*plib)->code);
	}
	*plib = &widge_initlib;
}

void cleanup_widget()
{
	widge_freelib(&widge_funcs,NULL);
	pj_freez(&widge_initlib.hl.self_free);
}
void init_widget(void *bvector)
{
	if(widge_initlib.hl.self_free == NULL) 
	{
		/* alloctate buffer for rex code to put free jump bracket code into */
		if((widge_initlib.hl.self_free = pj_malloc(EXIT_CODE_SIZE)) == NULL)
			return;
	}
	widge_freelib(&widge_funcs,NULL); /* get rid of the old one if present */
	widge_funcs = new_widgelib(bvector);
}

#ifdef TESTING
extern Errcode to_be_bracketed(int a, int b, int c, int d, int e)
{
	printf("\nIn bracket %d %d %d %d %d", a,b,c,d,e );
	return(21);
}
#endif


