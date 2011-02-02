#define WIDGET_INTERNALS
#include "widget.h"

/****************************************************************************
*                          IBMPC PARALLEL PORT BIT DEFINES
****************************************************************************/
#define	INIT		0x04	/* init off bit	*/
#define STROBE		0x01	/* strobe bit */
#define NBUSY		0x80	/* not busy bit	*/
#define ACKBIT		0x40	/* acknowledge off bit */
#define ERROR_BIT	0x08	/* error bit */


/***************************************************************************
*                       SENTINELPRO FAMILY CODES
****************************************************************************/
#define AB_FAMILY	0xC0	/* 1100 0000 */
#define AC_FAMILY	0xA0	/* 1010 0000 */
#define AD_FAMILY	0x90	/* 1001 0000 */
#define AE_FAMILY	0x88	/* 1000 1000 */
#define AF_FAMILY	0x84	/* 1000 0100 */
#define AG_FAMILY	0x82	/* 1000 0010 */
#define AH_FAMILY	0x81	/* 1000 0001 */

#define BC_FAMILY	0x60	/* 0110 0000 */
#define BD_FAMILY	0x50	/* 0101 0000 */
#define BE_FAMILY	0x48	/* 0100 1000 */
#define BF_FAMILY	0x44	/* 0100 0100 */
#define BG_FAMILY	0x42	/* 0100 0010 */
#define BH_FAMILY	0x41	/* 0100 0001 */

#define CD_FAMILY	0x30	/* 0011 0000 */
#define CE_FAMILY	0x28	/* 0010 1000 */
#define CF_FAMILY	0x24	/* 0010 0100 */
#define CG_FAMILY	0x22	/* 0010 0010 */
#define CH_FAMILY	0x21	/* 0010 0001 */

#define DE_FAMILY	0x18	/* 0001 1000 */
#define DF_FAMILY	0x14	/* 0001 0100 */
#define DG_FAMILY	0x12	/* 0001 0010 */
#define DH_FAMILY	0x11	/* 0001 0001 */

#define EF_FAMILY	0x0c	/* 0000 1100 */
#define EG_FAMILY	0x0a	/* 0000 1010 */
#define EH_FAMILY	0x09	/* 0000 1001 */

#define FG_FAMILY	0x06	/* 0000 0110 */
#define FH_FAMILY	0x05	/* 0000 0101 */

#define GH_FAMILY	0x03	/* 0000 0011 */


/***************************************************************************
*                    SENTINELPRO CLOCK AND DATA BIT DEFINES
****************************************************************************/
#define DATABIT		0x10		/* data bit for sentinel	*/
#define CLKBIT		0x04		/* clock bit for sentinel	*/

/***************************************************************************
*                         SENTINELPRO STATE PATTERNS 
****************************************************************************/
#define SP0 (0xff - CE_FAMILY)       /* Selects low,clock high,data high*/
#define SP1 (SP0 - CLKBIT)           /* Selects low,clock low,data high */
#define SP2 (SP0 - DATABIT)          /* Selects low,clock high,data low */
#define SP3 (SP0 - CLKBIT - DATABIT) /* Selects low,clock low,data low  */
#define SP4 (0xff)                    /* All data lines high, reset      */

/************ static data ************/

/* a list such that BRACKOFF + UNBOFF1 + UNBOFF2 ... == 0 */
/* within the stages of checking a widget bvector has these added to it 
 * so it gets to the original vector desired */

#define UNBOFF1 (11119)
#define UNBOFF2     (103050700-11110)
#define UNBOFF3  (20406080)
#define UNBOFF4 (-(BRACKOFF + 123456789))

/* vector for bracketer function pre loaded with first offset
 * will have vector + BRACKOFF added to it by set_bvector() */

static void *widge_bvector = ((void *)UNBOFF1); 


Widge_rexlib widgelib;  /* forward reference */

/******** macros for host library functions **********/

#define printf      			(*(widgelib.hl.printf))
#define outb(port,val) 			(*(widgelib.hl.outb))(port,val)
#define inb(port) 				(*(widgelib.hl.inb))(port)

static Boolean ask_retry()
{
	return((*(widgelib.hl.ask_retry))());
}
/**** externals for exit instructions in bracket.asm *****/

extern ULONG bracket_exit, ok_exit, err_widget_exit;

/****** macros to define for widget from port id ******/

static SHORT PortAddr[3] = { 0x3BC, 0x378, 0x278 }; 

#define NUM_PORTS Array_els(PortAddr)

#define data_port (PortAddr[widgelib.hl.port])
#define status_port (PortAddr[widgelib.hl.port]+1)
#define cmd_port (PortAddr[widgelib.hl.port]+2)

static void SendOne()
{
	outb(data_port,SP1); /* Selects low, clock low, data high*/
	outb(data_port,SP0); /* Selects low, clock high,data high*/
}
static void SendZero()
{
	outb(data_port,SP3);   /* Selects low,clock low,data low  */
	outb(data_port,SP2);   /* Selects low,clock high,data low */
	outb(data_port,SP0);   /* Selects low,clock high,data high*/
}
static ULONG widge_query(char* str)
{
UBYTE c;
UBYTE cmask;
ULONG result;
UBYTE cmd_reg;  /* The command register contents   */

	/* Reset all Sentinel devices */
	outb(data_port,SP4);      /* All data lines high */

	cmd_reg = inb(cmd_port);  /* Get command reg contents */

    /* Set both INIT and STROBE high   */
	outb(cmd_port,( cmd_reg | INIT ) & ( 0xff - STROBE ));

    /* Select target Sentinel */
	outb(data_port,SP0);  /* Selects low,clock high,data high*/
	SendZero();      /* Ensure SentinelPros are awake   */
	SendZero();

	result = 0;
	while((c = *str++) != 0)
	{
		for(cmask = 1;cmask & 0x0F;cmask <<= 1)
		{
			result <<= 1;
			if(!(((UBYTE)inb(status_port)) & NBUSY))
				result |= 1;
			(*((c & cmask)?SendZero:SendOne))();
		}
	}
	outb(data_port,SP4);   /* deselect all Sentinels  */
	outb(cmd_port,( cmd_reg | INIT ) & ( 0xff - STROBE ));
	return(result);
}
/* functions called by assembler glue module */

void copy_free_code(char *src, char *max, void *bvector)
{
char *dst;
void **pbv;

	dst = (char *)(widgelib.hl.self_free);
	pbv = &bvector;
	while(src < max)
		*dst++ = *src++;
	*pbv = OPTR(bvector,UNBOFF4);  /* this modifies the item on the stack
									* which is what is jumped to by 
									* bracket.asm */
}


int random(void)
{
static void *randseed = &widgelib; /* this address will vary with
								    * position of load which should vary */
#define RS (*((ULONG *)&randseed))

 	return((((ULONG)(RS=((RS * 0x41c64e6d)+0x3039)))>>10)&0x7fff);

#undef RS
}

struct query_entry {
	char *str;
	ULONG widge_ret;
}; 

struct query_entry query_data[] = {
	{ "Mira & Heidi", 0x8823b779, },
	{ "(C) Jim Kent", 0x9ba4fa25, },
	{ "Peter Kennard", 0xcb36fe56, },
	{ "Dancing Flame", 0x49404001, },
	{ "Widget Widget", 0xaa927dec, },
};

void *widge_check()
/* called by widge_bracket() in bracket.asm  */
{
int retries = 5; /* five chances */
struct query_entry *q;
int portcount;
void *bvect;

	bvect = OPTR(widge_bvector,UNBOFF2);

	while(retries-- > 0)
	{
		/* if we dont get it try all the ports */

		for(portcount = 0;portcount < NUM_PORTS;++portcount)
		{
			/* pick random string and try it */
		 	q = query_data + random()%Array_els(query_data);

			/* query hardware */
			if(q->widge_ret == widge_query(q->str))
			{
				bracket_exit = ok_exit;
				goto out;
			}
			/* try next port */
			if((++widgelib.hl.port) >= NUM_PORTS)
				widgelib.hl.port = 0;
		}
		if(!ask_retry())
			break;
	}

out:
	return(OPTR(bvect,UNBOFF3));
}

/***** functions for host use and library *****/

static void set_bvector(void *bvector)
/* function gets vector from caller and puts it in the static area */
{
LONG *free;

	free = (LONG *)widgelib.hl.self_free;
	*free++ = 0;
	*free++ = 0;
	*free = 0;
	*((LONG *)&widge_bvector) += *((LONG *)&bvector);
}

extern int widge_bracket(); /* in bracket.asm */

Widge_rexlib widgelib = {
	{ 
		set_bvector,
		widge_bracket,
		widge_query,
	},
	/* the rest filled in by host set to zeros here */
};
