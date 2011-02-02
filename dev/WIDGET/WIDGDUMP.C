#include "widget.h"
#include "memory.h"

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
#define SP0 (0xff - BH_FAMILY)        /* Selects low,clock high,data high*/
#define SP1 (SP0 - CLKBIT)           /* Selects low,clock low,data high */
#define SP2 (SP0 - DATABIT)          /* Selects low,clock high,data low */
#define SP3 (SP0 - CLKBIT - DATABIT) /* Selects low,clock low,data low  */
#define SP4 (0xff)                    /* All data lines high, reset      */



/*
static int io_data[] = {~(0x3BC), ~(0x378), ~(0x278)};
static int io_stat[] = {~(0x3BD), ~(0x379), ~(0x279)};
static int io_cmd[] = {~(0x3BE), ~(0x37A), ~(0x27A)};

int *xio_data = io_data + ELEMENTS(io_data);
int *xio_stat = io_stat + ELEMENTS(io_stat);
int *xio_cmd = io_cmd + ELEMENTS(io_cmd);
*/

SHORT PortAddr[3] = { 0x3BC, 0x378, 0x278 }; 

#define data_port(wd) (PortAddr[wd->port])
#define status_port(wd) (PortAddr[wd->port]+1)
#define cmd_port(wd) (PortAddr[wd->port]+2)

/* macros to define ins and outs to three ports for widget from Widge_data */

#define out_data(wd,val)	widge_outb(data_port(wd),val)
#define out_status(wd,val) 	widge_outb(status_port(wd),val)
#define out_cmd(wd,val) 	widge_outb(cmd_port(wd),val)

#define in_data(wd) 	widge_inb(data_port(wd))
#define in_status(wd) 	widge_inb(status_port(wd))
#define in_cmd(wd) 		widge_inb(cmd_port(wd))


static void widge_outb(int port, int value) 
/***** does outport for widget code *****/
/* 
 * TIMING NOTE : There is a 10 microsecond minimum delay value required to 
 *               to let the data and response lines to settle. */
{
extern LONG pj_clock_1000(); /* gets millisecond clock */
static int delay_count = -1;
int delay;

	if(delay_count < 0)
	{
		delay_count = pj_clock_1000();
	    for (delay = 0; delay < 100000; delay++);
		delay_count = pj_clock_1000() - delay_count;
		delay_count = 100000/(delay_count * 100);
	}
	outp(port,value);
    for (delay = 0; delay < delay_count; delay++);
}
static int widge_inb(int port)
{
	return(inp(port));
}
void SendOne(Widge_data *wd)
{
	out_data(wd, SP1); /* Selects low, clock low, data high*/
	out_data(wd, SP0); /* Selects low, clock high,data high*/
}
void SendZero(Widge_data *wd)
{
	out_data(wd, SP3);   /* Selects low,clock low,data low  */
	out_data(wd, SP2);   /* Selects low,clock high,data low */
	out_data(wd, SP0);   /* Selects low,clock high,data high*/
}
ULONG widge_query(Widge_data *wd, char* str)
{
UBYTE c;
UBYTE cmask;
ULONG result;
UBYTE cmd_reg;  /* The command register contents   */

	/* Reset all Sentinel devices */
	out_data(wd,SP4);      /* All data lines high */

	cmd_reg = in_cmd(wd);  /* Get command reg contents */

    /* Set both INIT and STROBE high   */
	out_cmd(wd, ( cmd_reg | INIT ) & ( 0xff - STROBE ));

    /* Select target Sentinel */
	out_data(wd,SP0);  /* Selects low,clock high,data high*/
	SendZero(wd);      /* Ensure SentinelPros are awake   */
	SendZero(wd);

	result = 0;
	while((c = *str++) != 0)
	{
		for(cmask = 1;cmask & 0x0F;cmask <<= 1)
		{
			result <<= 1;
			if(!(((UBYTE)in_status(wd)) & NBUSY))
				result |= 1;
			(*((c & cmask)?SendZero:SendOne))(wd);
		}
	}
	out_data(wd,SP4);   /* deselect all Sentinels  */
	out_cmd(wd, ( cmd_reg | INIT ) & ( 0xff - STROBE ));
	return(result);
}

#ifndef DUMP_WIDGE /* code to dump contents of widget for testing */

static void INTS_OFF() {}
static void RSTR_INTS() {}

static int scread(Widge_data *wd,SHORT cell)
/****************************** s c r e a d ( ) ****************************/
/* This function will read a cell from the Sentinel - C given the cell     */
/* number and return a 16-bit result.                                      */
/***************************************************************************/
{
int accum = 0;    /* initialize to zero      */
int index;
UBYTE cmd_reg;   /* port save cell */
static BYTE WakeUp[33]   = { 0xFF,0xFC,0x5C,0x5D,0x5E,0x5F,0x5C,0x5D,0x5C,
     		                0x5D,0x5C,0x5D,0x5C,0x5D,0x5C,0x5D,0x5C,0x5D,
     		                0x5C,0x5D,0x5C,0x5D,0xFF,
     		                0xFF,0xFC,0x5C,0x5D,0x5E,0x5F,0x5E,0x5F,0x5C,0x5D };


	cmd_reg = in_cmd(wd);  /* Get command reg contents        */

    /* Set both INIT and STROBE high   */
	out_cmd(wd, ( cmd_reg | INIT ) & ( 0xff - STROBE ));

    /* Select target Sentinel */
	out_data(wd, SP0);   /* Selects low,clock high,data high*/
	SendZero(wd);          /* Ensure SentinelPros are awake   */
	SendZero(wd);


	for (index = 0; index < 23 ; index++)
		out_data(wd,WakeUp[index]);

	INTS_OFF(); /* DISABLE the interrupts here !   */

	for(index = 23; index < 33 ; index++)
		out_data(wd,WakeUp[index]);

	/* send a cell number      */
	for(index = 0x20; index > 0; index >>= 1)
	{
		if ((cell & index) == index)
			SendOne(wd);
		else
			SendZero(wd);             /* Send a zero bit         */
	}

/* Note: you do not have to enable interrupts at this point, just do NOT   */
/*       enable them BEFORE this point ! */

	RSTR_INTS();

	for (index = 0; index < 16 ; index++)   /* get the response */
	{
		SendZero(wd);    /* Send a zero bit */
		accum <<= 1;
		if(!(((UBYTE)in_status(wd)) & NBUSY)) /* invert NOT busy */
			accum |= 1; 
	}

	out_data(wd,0xFF);   /* deselect all Sentinels  */
	return(accum);      /* return the result       */
}
void widge_dump()
{
int i;
LONG result;
Widge_data wd;

	clear_struct(&wd);

	for (i=0;i<64;i++)            /* Look at all 64 cells of the Sentinel-C  */
	{
		result = scread(&wd,i);  /* Read 1 cell of the Sentinel-C */
		if(!(i & 0x7))
			printf("\n");
		printf("%4x ",result);
	}
}
#endif /* DUMP_WIDGE */
