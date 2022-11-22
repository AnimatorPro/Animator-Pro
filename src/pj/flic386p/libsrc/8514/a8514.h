
#ifndef A8514_H
#define A8514_H

#define WORD short
/*....................Bit Definitions....................................*/
#define NOERROR		0x00
#define NO8514A		0x80
/*....................GC250 REGISTERS....................................*/
#define DISP_STAT		0x02E8	/* Display Status Register		*/
#define H_TOTAL			0x02E8  /* Horizontal Total Register	*/
#define DAC_MASK		0x02EA	/* DAC Mask 					*/
#define DAC_RINDEX		0x02EB	/* DAC read index register		*/
#define DAC_WINDEX		0x02EC	/* DAC write index register		*/
#define DAC_DATA		0x02ED	/* DAC Data register			*/
#define H_DISPLAYED		0x06E8	/* Horiz Displayed Reg			*/
#define H_SYNC_STRT		0x0AE8	/* Horiz Sync Start Reg			*/
#define H_SYNC_WID		0x0EE8	/* Horiz Sync Width Reg			*/
#define V_TOTAL			0x12E8	/* Vertical Total Reg			*/
#define V_DISPLAYED		0x16E8	/* Vertical Displayed Reg		*/
#define V_SYNC_STRT		0x1AE8	/* Vertical Sync Start Reg		*/
#define V_SYNC_WID		0x1EE8	/* Vertical Sync Width Reg		*/
#define DISP_CNTL		0x22E8	/* Display Control Register		*/
#define SUBSYS_CNTL		0x42E8	/* Subsystem Control Reg		*/
#define SUBSYS_STAT 	0x42E8	/* Subsystem Status Reg			*/
#define ROM_PAGE_SEL	0x46E8	/* ROM Page Select Reg			*/
#define ADVFUNC_CNTL	0x4AE8	/* Adv Func Control Reg			*/
#define CUR_Y_POS		0x82E8	/* Current Y Position			*/
#define CUR_X_POS		0x86E8	/* Current Y Position			*/
#define DESTY_AXSTP		0x8AE8	/* Dest Y/Axial Step Reg		*/
#define DESTX_DIASTP	0x8EE8	/* Dest X/Diagl Step Reg		*/
#define ERR_TERM		0x92E8	/* Error Term Register			*/
#define MAJ_AXIS_PCNT	0x96E8	/* Major Axis Pixel Count		*/
#define COMMAND			0x9AE8	/* Command register				*/
#define GP_STATUS		0x9AE8	/* Graphics Processor Status	*/
#define CMD_STATUS		0x9AE9	/* Command status register		*/
#define SHORT_STROKE	0x9EE8	/* Short Stroke Vector Reg		*/
#define BKGD_COLOR		0xA2E8	/* Background Color 			*/
#define FRGD_COLOR		0xA6E8	/* Foreground Color 			*/
#define WRT_MASK		0xAAE8	/* Write Mask					*/
#define RD_MASK			0xAEE8	/* Read Mask					*/
#define COLOR_CMP		0xB2E8	/* Color Compare Register		*/
#define BKGD_MIX		0xB6E8	/* Background Mix Register		*/
#define FGRD_MIX		0xBAE8	/* Foreground Mix Register		*/
#define MLTFUNC_CNTL	0xBEE8	/* Multifunction Control		*/
#define PIX_TRANS		0xE2E8	/* Pixel Data Transfer Reg.		*/
/* ..................Bit definitions of Registers .....................*/
#define CMD_ACTIVE		0x02	/* Command is active? */
#define DATA_AVAIL		0x01	/* Input Data Available? */
#define NO8514			0x40	/* No 8514 Monitor present */
#define MONOCHROME 		0x10	/* Monochrome Monitor? */
#define PLANE8			0x80	/* 8 plane memory available */
/* ..................COMMAND mask bits ................................*/
#define WRITCMD		0x01
#define PLANAR		0x02
#define LSTPELNULL	0x04
#define STROKE_ALG	0x08
#define DRAWCMD		0x10
#define INCX		0x20
#define YMAJAXIS	0x40
#define INCY		0x80
#define PC_TRANS	0x100
#define BIT16		0x200
#define BYTE_SWAP	0x1000
#define NO_FCN		0x0000
#define LINE_DRAW	0x2000
#define FILL_X_RECT	0x4000
#define FILL_Y_RECT	0x6000
#define FILL_RECT	0x8000
#define AXIAL_LINE	0xA000
#define COPY_RECT	0xC000
#define	HANG		0xE000
/* ..................MIX Defines ........................................*/
#define MIX_NOT_DEST		0x00
#define MIX_ZERO			0x01
#define MIX_ONE				0x02
#define MIX_DEST			0x03
#define MIX_NOT_SRC			0x04
#define MIX_SRC_XOR_DEST	0x05
#define MIX_NOT				0x06
#define MIX_SRC				0x07
/* .................MIX Sources .........................................*/
#define B_CLR_ACTIVE	0x00
#define F_CLR_ACTIVE	0x20
#define PTRANS_ACTIVE	0x40
#define ALL_PLANE_CPY	0x60
/* .................MLTFUNC_CNTL ... high order nibble is an index .......*/
#define MINOR_AXIS_PCNT	0x0000
#define SCISSOR_T		0x1000
#define SCISSOR_L		0x2000
#define SCISSOR_B		0x3000
#define SCISSOR_R		0x4000
#define MEM_CNTL		0x5000
#define PIX_CNTL		0xA000
/* Mix operation select */
#define ONE_PLN_COPY 	0x0C0
/* Write transfers use fgdmix for 1's and bkgdmix for 0's */
#define WRT_PLN_MODE	0x080
/* Write transfers use fgdmix for 1's and bkgdmix for 0's */
#define PIX_MIX_SEL		0x040
#define FGD_MIX_ACT		0x0000
/* Misc bit deinitions */
#define STOP_SEQ		0x9000	/* subsystem cntl reg */
#define	START_SEQ		0x5000	/* subsystem cntl reg */
#define RESET_QUEUE_FULL 0x04

#define VP1024	0		/* AI Monde 0,2,3		*/
#define VP640x4	1		/* AI Mode 1 (4 plane)  */
#define VP640x8	2		/* AI Mode 1 (8 plane)	*/


/*.........................CONTROL WORD BITFIELDS...................*/
#define GP_READ		0
#define GP_WRITE	1				/* Writing or reading screen */
#define GP_PIXEL	0
#define GP_PLANE	(1<<1)			/* Bitplane or chunky pixels */
#define GP_SETL		0
#define GP_SKIPL	(1<<2)			/* Skip last pixel in line? */
#define GP_LINEL	0
#define GP_LINES	(1<<3)			/* Short stroke or long line? */
#define GP_MOVE		0
#define GP_DRAW		(1<<4)			/* Draw pixels or just move x/y position */
#define GP_DECX		0
#define GP_INCX		(1<<5)			/* Increment or decrement x position? */
#define GP_AXISX	0
#define GP_AXISY	(1<<6)			/* Is Y or X the major axis? */
#define GP_DECY		0
#define GP_INCY		(1<<7)			/* Increment or decrement y position? */
#define GP_NODATA	0
#define GP_DATA		(1<<8)			/* Pixel Data Transfer register used? */
#define GP_BUS8		0
#define GP_BUS16	(1<<9)			/* 16 or 8 bit buss access */
#define GP_PIXEL8	0
#define GP_PIXEL16	(1<<10)			/* Always enabled - 16 bit internal */
#define GP_RESERVED (0<<11)
#define GP_NOSWAP	0
#define GP_SWAP		(1<<12)			/* Swap bytes on 16 bit PC transfers */
#define GPC_NOP		(0<<13)			/* Do nothing */
#define GPC_LINE	(1<<13)			/* Draw a line */
#define GPC_RECTX	(2<<13)			/* Rectangle drawn x-wise. Pos updated */
#define GPC_RECTY	(3<<13)			/* Rectangle drawn y-wise. Pos updated */
#define GPC_RECTS	(4<<13)			/* Rectangle.  XY Pos not updated at end */
#define GPC_LINEF	(5<<13)			/* Line for doing poly-fills. */
#define GPC_COPY	(6<<13)			/* Copy to/from memory */
#define GPC_HANG	(7<<13)			/* Make adapter hang & need reboot */


#ifndef STDTYPES_H
	#include "stdtypes.h"
#endif

#ifndef ERRCODES_H
	#include "errcodes.h"
#endif

#ifndef RASTER_H
	#include "raster.h"
#endif

typedef struct a8514_hw {
	LONG xcard, ycard;
	USHORT flags;
	USHORT screen_ix;
	struct vdevice *dev;  /* pointer back to null device */
} A8514_hw;

#define IS_DISPLAYED 0x0001	/* raster is currently displayed */

typedef union bmap_and_8514 {
	Bmap bm;			/* Ram/MCGA device specific */
	A8514_hw am;		/* IBM 8514/A device specific */
} Raster_hw;

typedef struct rast8514 {
	RASTHDR_FIELDS;
	Raster_hw hw;
} Rast8514;


/* this will cause a compiler that rejects arrays of size 0 xx[0] to barf
 * (most compilers in fact)
 * if the sizes of Rast8514 and Raster are not equal */

struct _8514_error_check_ {
	char xx[sizeof(Rast8514) == sizeof(Raster)];
};

/* This definition - LibRast - lets the compiler use your hardware specific 
 * raster structure as the "Raster" parameter to device and raster library
 * function vectors without the compiler requiring lots and lots of
 * type-casts or complaining about type-mismatches. It must be defined
 * _before_ rastlib.h or vdevice.h are included. */
#define LibRast Rast8514

#ifndef RASTLIB_H
	#include "rastlib.h"
#endif

#ifndef VDEVICE_H
	#include "vdevice.h"
#endif

#endif /* A8514_H */
