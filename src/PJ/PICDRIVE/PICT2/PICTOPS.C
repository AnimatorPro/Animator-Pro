/***************************************************************
pictops.c - Pict file reader opcode vector table and parser.

Pict file pdr modules:

	Created by Peter Kennard.  Sept 29, 1991
		Implements non-pattern fill mode bitmap operations
		and transfer mode blit. Parses and ignores all other
		opcodes.
****************************************************************/

#include "errcodes.h"
#include "pict.h"
#include "picdrive.h"
#include "syslib.h"



#define VAR16  -1  /* opsize is a chunk16 */
#define VAR32  -2  /* opsize is a chunk32 */
#define VARPOLY -3 /* it is a polygon */
#define VARLEN -4
#define VAREGION -5 /* region? */

/* Table of data sizes for fixed length opcodes from 0 to 0xFF.
 * Note that in 2 byte versions the data + opcode sizes must be word aligned.
 * The values here are the data sizes only. */

BYTE pict_opsizes[0x100] =
{
	0,		/* no op */
	VAREGION, 	/* Clip */
	8, 	/* BkPat */
	2,	/* TxFont */
	2, 	/* TxFace */
	2, 	/* TxMode */
	4, 	/* SpExtra */
	4, 	/* PnSize */
	2, 	/* PnMode */
	8, 	/* PnPat */
	8, 	/* FillPat */
	4, 	/* OvSize */
	4, 	/* Origin */
	2, 	/* TxSize */
	4, 	/* FgColor */
	4, 	/* BkColor */

	/* 0x10 */
	8, 	/* TxRatio */
	1, 	/* Version */
	VARLEN,
	VARLEN,
	VARLEN,
	2,
	2,
	0,
	0,
	0,
	6, /* Rgb color VARLEN, */
	6, /* Rgb color VARLEN, */
	0,
	6, /* Rgb color VARLEN, */
	0,
	6, /* Rgb color VARLEN, */

	/* 0x20 */
	8, 	/* line */
	4,
	6,
	2,

	/* 0x24 */
	VAR16, /* apple reserved chunk16 */
	VAR16, /* apple reserved chunk16 */
	VAR16, /* apple reserved chunk16 */
	VAR16, /* apple reserved chunk16 */

	/* 0x28 */
	VARLEN, /* LongText */
	VARLEN, /* DHText */
	VARLEN, /* DVText */
	VARLEN, /* DHDVText */

	/* 0x2c */
	VAR16, /* apple reserved chunk16 */
	VAR16, /* apple reserved chunk16 */
	VAR16, /* apple reserved chunk16 */
	VAR16, /* apple reserved chunk16 */

	/* 0x30 */
	8, /* frameRect */
	8,
	8,
	8,
	8,

	/* 0x35 */
	8,  /* apple reserved */
	8,  /* apple reserved */
	8,  /* apple reserved */

	/* 0x38 */
	0, /* frameSameRect */
	0,
	0,
	0,
	0,

	/* 0x3c */
	0,  /* apple reserved opcode */
	0,  /* apple reserved opcode */
	0,  /* apple reserved opcode */

	/* 0x40 */
	8, /* frameRRect */
	8,
	8,
	8,
	8,

	/* 0x45 */
	8,  /* apple reserved */
	8,  /* apple reserved */
	8,  /* apple reserved */

	/* 0x48 */
	0,	/* frameSameRRect */
	0,
	0,
	0,
	0,

	/* 0x4d */
	0,  /* apple reserved opcode */
	0,  /* apple reserved opcode */
	0,  /* apple reserved opcode */

	/* 0x50 */
	8,  /* frameOval */
	8,
	8,
	8,
	8,

	/* 0x55 */
	8,  /* apple reserved */
	8,  /* apple reserved */
	8,  /* apple reserved */

	/* 0x58 */
	0,  /* frameSameOval */
	0,
	0,
	0,
	0,

	/* 0x5d */
	0,  /* apple reserved opcode */
	0,  /* apple reserved opcode */
	0,  /* apple reserved opcode */

	/* 0x60 */
	12,  /* frameArc */
	12,
	12,
	12,
	12,

	/* 0x65 */
	12,  /* apple reserved */
	12,  /* apple reserved */
	12,  /* apple reserved */

	/* 0x68 */
	4,   /* frameSameArc */
	4,
	4,
	4,
	4,

	/* 0x6d */
	4,  /* apple reserved */
	4,  /* apple reserved */
	4,  /* apple reserved */

	/* 0x70 */
	VARPOLY, /* framePoly */
	VARPOLY,
	VARPOLY,
	VARPOLY,
	VARPOLY,

	/* 0x75 */
	VARPOLY, /* apple reserved poly */
	VARPOLY, /* apple reserved poly */
	VARPOLY, /* apple reserved poly */

	/* 0x78 */ 
	0, /* frameSamePoly */
	0,
	0,
	0,
	0,

	/* 0x7d */
	0,  /* apple reserved opcode */
	0,  /* apple reserved opcode */
	0,  /* apple reserved opcode */

	/* 0x80 */
	VAREGION, /* frameRgn */
	VAREGION, /* paintRgn */
	VAREGION, /* eraseRgn */
	VAREGION, /* invertRgn */
	VAREGION, /* fillRgn */

	/* 0x85 */
	VAREGION, /* apple reserved opcode */
	VAREGION, /* apple reserved opcode */
	VAREGION, /* apple reserved opcode */

	/* 0x88 */
	0, /* frameSameRgn */
	0, /* paintSameRgn */
	0, /* eraseSameRgn */
	0, /* invertSameRgn */
	0, /* fillSameRgn */

	/* 0x8D */
	0,  /* apple reserved opcode */
	0,  /* apple reserved opcode */
	0,  /* apple reserved opcode */

	/* 0x90 */
	VARLEN, /* bitsRect */
	VARLEN, /* bitsRgn */

	/* 0x92 */
	VAR16,  /* apple reserved chunk16 */
	VAR16,  /* apple reserved chunk16 */
	VAR16,  /* apple reserved chunk16 */
	VAR16,  /* apple reserved chunk16 */
	VAR16,  /* apple reserved chunk16 */
	VAR16,  /* apple reserved chunk16 */

	/* 0x98 */
	VARLEN, /* PackBitsRect */
	VARLEN, /* PackBitsRgn */

	/* 0x9a */
	VAR16,  /* apple reserved chunk16 */
	VAR16,  /* apple reserved chunk16 */
	VAR16,  /* apple reserved chunk16 */
	VAR16,  /* apple reserved chunk16 */
	VAR16,  /* apple reserved chunk16 */
	VAR16,  /* apple reserved chunk16 */

	/* 0xA0 */
	2,  	/* ShortComment */
	VARLEN,  /* LongComment */

	/* 0xA2 */
	            VAR16,VAR16, /* apple reserved chunk16 */
	VAR16,VAR16,VAR16,VAR16, /* apple reserved chunk16 */
	VAR16,VAR16,VAR16,VAR16, /* apple reserved chunk16 */
	VAR16,VAR16,VAR16,VAR16, /* apple reserved chunk16 */

	/* 0xB0 */
	0,0,0,0, 	/* apple reserved opcodes */
	0,0,0,0, 	/* apple reserved opcodes */
	0,0,0,0, 	/* apple reserved opcodes */
	0,0,0,0, 	/* apple reserved opcodes */

	/* 0xC0 */
	0,0,0,0, 	/* apple reserved opcodes */
	0,0,0,0, 	/* apple reserved opcodes */
	0,0,0,0, 	/* apple reserved opcodes */
	0,0,0,0, 	/* apple reserved opcodes */

	/* 0xD0 */
	VAR32,VAR32,VAR32,VAR32,  /* apple reserved chunk32 */
	VAR32,VAR32,VAR32,VAR32,  /* apple reserved chunk32 */
	VAR32,VAR32,VAR32,VAR32,  /* apple reserved chunk32 */
	VAR32,VAR32,VAR32,VAR32,  /* apple reserved chunk32 */

	/* 0xE0 */
	VAR32,VAR32,VAR32,VAR32,  /* apple reserved chunk32 */
	VAR32,VAR32,VAR32,VAR32,  /* apple reserved chunk32 */
	VAR32,VAR32,VAR32,VAR32,  /* apple reserved chunk32 */
	VAR32,VAR32,VAR32,VAR32,  /* apple reserved chunk32 */

	/* 0xF0 */
	VAR32,VAR32,VAR32,VAR32,  /* apple reserved chunk32 */
	VAR32,VAR32,VAR32,VAR32,  /* apple reserved chunk32 */
	VAR32,VAR32,VAR32,VAR32,  /* apple reserved chunk32 */
	VAR32,VAR32,VAR32,     	  /* apple reserved chunk32 */

	/* 0xFF */    
	0,           /* end of pic opcode */
};



/* functions for vector table */

Errcode unimpl_varlen(Pfile *);
Errcode skip_nodata_opcode(Pfile *);
Errcode skip_region(Pfile *);
Errcode skip_poly(Pfile *);
Errcode do_pixmap(Pfile *);
Errcode skip_longtext(Pfile *);
Errcode skip_dtext(Pfile *);
Errcode skip_d2text(Pfile *);
Errcode skip_pattern(Pfile *);
Errcode do_version(Pfile *);
Errcode return_eopic(Pfile *);
Errcode skip_long_comment(Pfile *);

Errcode do_rectop(Pfile *);
Errcode	do_frameSameRect(Pfile *);
Errcode do_fillSameRect(Pfile *);
Errcode do_truecolor(Pfile *);

/* These defines made it easier for me to code this. */
#define SKIPOP skip_nodata_opcode
#define SKIPF skip_fixedop
#define SKIPTEXT skip_str255
#define SKIP16 skip_chunk16
#define SKIP32 skip_chunk32
#define SKIPOLY skip_poly
#define SKIPREGION skip_region
#define SKIPLEN unimpl_varlen

/* Opcode handling vectors for opcodes 0 to oxFF */

Do_pictop low_vectors[0x100] =
{
	SKIPOP,  /* no op */
	SKIPREGION,  /* Clip */
	SKIPF, 	/* BkPat */
	SKIPF,	/* TxFont */
	SKIPF, 	/* TxFace */
	SKIPF, 	/* TxMode */
	SKIPF, 	/* SpExtra */
	SKIPF, 	/* PnSize */
	SKIPF, 	/* PnMode */
	SKIPF, 	/* PnPat */
	SKIPF, 	/* FillPat */
	SKIPF, 	/* OvSize */
	SKIPF, 	/* Origin */
	SKIPF, 	/* TxSize */
	SKIPF, 	/* FgColor */
	SKIPF,  	/* BkColor */

	/* 0x10 */
	SKIPF, 	/* TxRatio */
	do_version, 	/* Version */
	skip_pattern,
	skip_pattern,
	skip_pattern,
	SKIPF,
	SKIPF,
	SKIPF,
	SKIPF,
	SKIPF,
	SKIPF, 		/* RGBFgcol */
	SKIPF, 		/* RGBBkCol */
	SKIPOP,     /* HiliteMode */
	SKIPF, 		/* HiliteColor */
	SKIPOP,     /* DefHilite */
	SKIPF, 		/* OpColor */

	/* 0x20 */
	SKIPF, 	/* line */
	SKIPF,
	SKIPF,
	SKIPF,

	/* 0x24 */
	SKIP16, /* apple reserved chunk16 */
	SKIP16, /* apple reserved chunk16 */
	SKIP16, /* apple reserved chunk16 */
	SKIP16, /* apple reserved chunk16 */

	/* 0x28 */
	skip_longtext, /* LongText */
	skip_dtext,    /* DHText */
	skip_dtext,    /* DVText */
	skip_d2text,   /* DHDVText */

	/* 0x2c */
	SKIP16, /* apple reserved chunk16 */
	SKIP16, /* apple reserved chunk16 */
	SKIP16, /* apple reserved chunk16 */
	SKIP16, /* apple reserved chunk16 */

	/* 0x30 */
	do_rectop, /* frameRect */
	do_rectop,
	do_rectop,
	do_rectop,
	do_rectop,

	/* 0x35 */
	SKIPF,  /* apple reserved */
	SKIPF,  /* apple reserved */
	SKIPF,  /* apple reserved */

	/* 0x38 */
	SKIPOP, /* do_frameSameRect, */
	SKIPOP,
	SKIPOP,
	SKIPOP,
	SKIPOP, /* do_fillSameRect, */

	/* 0x3d */
	SKIPOP,  /* apple reserved opcode */
	SKIPOP,  /* apple reserved opcode */
	SKIPOP,  /* apple reserved opcode */

	/* 0x40 */
	do_rectop, /* frameRRect */
	do_rectop,
	do_rectop,
	do_rectop,
	do_rectop,

	/* 0x45 */
	SKIPF,  /* apple reserved */
	SKIPF,  /* apple reserved */
	SKIPF,  /* apple reserved */

	/* 0x48 */
	SKIPOP,	/* frameSameRRect */
	SKIPOP,
	SKIPOP,
	SKIPOP,
	SKIPOP,

	/* 0x4d */
	SKIPOP,  /* apple reserved opcode */
	SKIPOP,  /* apple reserved opcode */
	SKIPOP,  /* apple reserved opcode */

	/* 0x50 */
	do_rectop,  /* frameOval */
	do_rectop,
	do_rectop,
	do_rectop,
	do_rectop,

	/* 0x55 */
	SKIPF,  /* apple reserved */
	SKIPF,  /* apple reserved */
	SKIPF,  /* apple reserved */

	/* 0x58 */
	SKIPOP,  /* frameSameOval */
	SKIPOP,
	SKIPOP,
	SKIPOP,
	SKIPOP,

	/* 0x5d */
	SKIPOP,  /* apple reserved opcode */
	SKIPOP,  /* apple reserved opcode */
	SKIPOP,  /* apple reserved opcode */

	/* 0x60 */
	SKIPF,  /* frameArc */
	SKIPF,
	SKIPF,
	SKIPF,
	SKIPF,

	/* 0x65 */
	SKIPF,  /* apple reserved */
	SKIPF,  /* apple reserved */
	SKIPF,  /* apple reserved */

	/* 0x68 */
	SKIPF,   /* frameSameArc */
	SKIPF,
	SKIPF,
	SKIPF,
	SKIPF,

	/* 0x6d */
	SKIPF,  /* apple reserved */
	SKIPF,  /* apple reserved */
	SKIPF,  /* apple reserved */

	/* 0x70 */
	SKIPOLY, /* framePoly */
	SKIPOLY,
	SKIPOLY,
	SKIPOLY,
	SKIPOLY,

	/* 0x75 */
	SKIPOLY, /* apple reserved poly */
	SKIPOLY, /* apple reserved poly */
	SKIPOLY, /* apple reserved poly */

	/* 0x78 */ 
	SKIPOP, /* frameSamePoly */
	SKIPOP,
	SKIPOP,
	SKIPOP,
	SKIPOP,

	/* 0x7d */
	SKIPOP,  /* apple reserved opcode */
	SKIPOP,  /* apple reserved opcode */
	SKIPOP,  /* apple reserved opcode */

	/* 0x80 */
	SKIPREGION, /* frameRgn */
	SKIPREGION, /* paintRgn */
	SKIPREGION, /* eraseRgn */
	SKIPREGION, /* invertRgn */
	SKIPREGION, /* fillRgn */

	/* 0x85 */
	SKIPREGION, /* apple reserved opcode */
	SKIPREGION, /* apple reserved opcode */
	SKIPREGION, /* apple reserved opcode */

	/* 0x88 */
	SKIPOP, /* frameSameRgn */
	SKIPOP, /* paintSameRgn */
	SKIPOP, /* eraseSameRgn */
	SKIPOP, /* invertSameRgn */
	SKIPOP, /* fillSameRgn */

	/* 0x8D */
	SKIPOP,  /* apple reserved opcode */
	SKIPOP,  /* apple reserved opcode */
	SKIPOP,  /* apple reserved opcode */

	/* 0x90 */
	do_pixmap, /* bitsRect */
	do_pixmap, /* bitsRgn */

	/* 0x92 */
	SKIP16,  /* apple reserved chunk16 */
	SKIP16,  /* apple reserved chunk16 */
	SKIP16,  /* apple reserved chunk16 */
	SKIP16,  /* apple reserved chunk16 */
	SKIP16,  /* apple reserved chunk16 */
	SKIP16,  /* apple reserved chunk16 */

	/* 0x98 */
	do_pixmap, /* PackBitsRect */
	do_pixmap, /* PackBitsRgn */

	/* 0x9a */
	do_truecolor,  /* 16 and 32 bit pixel map records */
	SKIP16,  /* apple reserved chunk16 */
	SKIP16,  /* apple reserved chunk16 */
	SKIP16,  /* apple reserved chunk16 */
	SKIP16,  /* apple reserved chunk16 */
	SKIP16,  /* apple reserved chunk16 */

	/* 0xA0 */
	SKIPF,  /* ShortComment */
	skip_long_comment,  /* LongComment */

	/* 0xA2 */
	              SKIP16,SKIP16, /* apple reserved chunk16 */
	SKIP16,SKIP16,SKIP16,SKIP16, /* apple reserved chunk16 */
	SKIP16,SKIP16,SKIP16,SKIP16, /* apple reserved chunk16 */
	SKIP16,SKIP16,SKIP16,SKIP16, /* apple reserved chunk16 */

	/* 0xB0 */
	SKIPOP,SKIPOP,SKIPOP,SKIPOP, 	/* apple reserved opcodes */
	SKIPOP,SKIPOP,SKIPOP,SKIPOP, 	/* apple reserved opcodes */
	SKIPOP,SKIPOP,SKIPOP,SKIPOP, 	/* apple reserved opcodes */
	SKIPOP,SKIPOP,SKIPOP,SKIPOP, 	/* apple reserved opcodes */

	/* 0xC0 */
	SKIPOP,SKIPOP,SKIPOP,SKIPOP, 	/* apple reserved opcodes */
	SKIPOP,SKIPOP,SKIPOP,SKIPOP, 	/* apple reserved opcodes */
	SKIPOP,SKIPOP,SKIPOP,SKIPOP, 	/* apple reserved opcodes */
	SKIPOP,SKIPOP,SKIPOP,SKIPOP, 	/* apple reserved opcodes */

	/* 0xD0 */
	SKIP32,SKIP32,SKIP32,SKIP32,  /* apple reserved chunk32 */
	SKIP32,SKIP32,SKIP32,SKIP32,  /* apple reserved chunk32 */
	SKIP32,SKIP32,SKIP32,SKIP32,  /* apple reserved chunk32 */
	SKIP32,SKIP32,SKIP32,SKIP32,  /* apple reserved chunk32 */

	/* 0xE0 */
	SKIP32,SKIP32,SKIP32,SKIP32,  /* apple reserved chunk32 */
	SKIP32,SKIP32,SKIP32,SKIP32,  /* apple reserved chunk32 */
	SKIP32,SKIP32,SKIP32,SKIP32,  /* apple reserved chunk32 */
	SKIP32,SKIP32,SKIP32,SKIP32,  /* apple reserved chunk32 */

	/* 0xF0 */
	SKIP32,SKIP32,SKIP32,SKIP32,  /* apple reserved chunk32 */
	SKIP32,SKIP32,SKIP32,SKIP32,  /* apple reserved chunk32 */
	SKIP32,SKIP32,SKIP32,SKIP32,  /* apple reserved chunk32 */
	SKIP32,SKIP32,SKIP32,     	  /* apple reserved chunk32 */

	/* 0xFF */    
	return_eopic,       		  /* end of pic opcode */
};


Errcode do_pictops(Pfile *pf)
/* Read opcodes and call a vector depending on what it is until one reaches
 * a condition where pf->lasterr != 0 or an end of picture opcode is reached.
 *  Opcodes that are < 256 are in a table of vectors.  Opcodes with values
 * from 256 on are called by the if else logic. */
{
USHORT op;

	pf->bytes_since = 0; /* This function must start with file at an opcode */
	pf->got_bitmap = FALSE;	/* No bitmap yet. */

	for(;;)
	{
		/* Read opcode using version specific function. */
		if((*pf->read_opcode)(pf) < Success)
			break;

		op = pf->op;

#ifdef PRINTSTUFF
		printf("\n%03lx Op #%04x ", ftell(pf->file)-2, op );
#endif

		if(op < 0x0100)
		{
			(*low_vectors[op])(pf);
		}
		else if(op < 0x0200)
		{
			pf_seek_bytes(pf,2);
		}
		else if(op < 0x0C00)
		{	
			pf_seek_bytes(pf,2);
		}
		else if(op == 0x0C00) /* header op */
		{
			pf_seek_bytes(pf,24);
		}
		else if(op < 0x7F00)
		{
			pf_seek_bytes(pf,32);
		}
		else if(op < 0x8000U)
		{
			pf_seek_bytes(pf,254);
		}
		else if(op < 0x8100)
		{
			continue;
		}
		else
		{
			skip_chunk32(pf);
		}
		if(pf->lasterr != Success)
			break;
	}
	return(pf->lasterr);
}


/*********** opcode specific functions **************/
static Errcode skip_nodata_opcode(Pfile *pf)
{
#ifdef PRINTSTUFF
	printf("nodata");
#endif
	return(Success);
}
Errcode skip_region(Pfile *pf)
/* A region is a chunk with a leading 2byte size word. The size is *inclusive*
 * of the size word, Seek by it. */
{
USHORT size;

#ifdef PRINTSTUFF
	printf("region ");
#endif
	if(pf_read(pf,&size,sizeof(size)) >= Success)
	{
#ifdef PRINTSTUFF
	printf("sz %d ", intel_swap_word(size));
#endif
		if((size = intel_swap_word(size)) > 2)
			pf_seek_bytes(pf,size-2);

	}
	return(pf->lasterr);
}
static Errcode skip_poly(Pfile *pf)
/* A polgon is a chunk with a leading 2byte size word. The size is *inclusive*
 * of the size word. */
{
USHORT size;

#ifdef PRINTSTUFF
	printf("poly ");
#endif
	if(pf_read(pf,&size,sizeof(size)) >= Success)
	{
#ifdef PRINTSTUFF
	printf("sz %d ", intel_swap_word(size));
#endif
		if((size = intel_swap_word(size)) > 2)
			pf_seek_bytes(pf,size-2);

	}
	return(pf->lasterr);
}
static Errcode skip_fixedop(Pfile *pf)
/* Skips data for an opcode whose data size is fixed and in the table
 * pict_opsizes above. */
{
#ifdef PRINTSTUFF
	printf("fixed ");
#endif
	return(pf_seek_bytes(pf,pict_opsizes[pf->op]));
}
static Errcode skip_dtext(Pfile *pf)
/* Seek by text record preceded by dx and a size byte. */
{
struct {
	UBYTE dx;
	UBYTE size;
} dtext;

#ifdef PRINTSTUFF
	printf("dtext ");
#endif
	if(pf_read(pf,&dtext,sizeof(dtext)) >= Success)
		pf_seek_bytes(pf,dtext.size);
	return(pf->lasterr);
}
static Errcode skip_d2text(Pfile *pf)
/* Seek by a text record preceded by dx,dy and a size byte. */
{
struct {
	UBYTE dx;
	UBYTE dy;
	UBYTE size;
} dtext;

#ifdef PRINTSTUFF
	printf("d2text ");
#endif
	if(pf_read(pf,&dtext,sizeof(dtext)) >= Success)
		pf_seek_bytes(pf,dtext.size);
	return(pf->lasterr);
}
static Errcode skip_longtext(Pfile *pf)
/* Seek by a text preceded by a short point dx,dy and a size byte. */
{
struct {
	pPoint pt;
	UBYTE size;
} dtext;

#ifdef PRINTSTUFF
	printf("longtext ");
#endif
	if(pf_read(pf,&dtext,sizeof(dtext)) >= Success)
		pf_seek_bytes(pf,dtext.size);
	return(pf->lasterr);
}

#ifdef SLUFFED
static Errcode unimpl_varlen(Pfile *pf)
{
#ifdef PRINTSTUFF
	printf("unknown variable length item");
#endif
	return(pf->lasterr = Err_unimpl);
}
#endif

static Errcode do_version(Pfile *pf)
/* Reset version for a version change opcode. */
{
UBYTE version;

	if(pf_read(pf,&version,1) >= Success)
		set_version(pf,version);
	return(pf->lasterr);
}
static Errcode return_eopic(Pfile *pf)
/* End of picture. */
{
	return(pf->lasterr = RET_EOPIC);
}
static Errcode skip_long_comment(Pfile *pf)
/* Note I'm not sure whether the data size is inclusive or exclusive
 * of the sizeof(data.size).  I'm assuming exclusive of it like most of 
 * the other chunks I have encountered. */
{
struct {
	SHORT type;
	USHORT size;
} data;

#ifdef PRINTSTUFF
	printf("long comment !!!\n");
#endif

	if(pf_read(pf,&data,sizeof(data)) >= Success)
		return(pf_seek_bytes(pf,(USHORT)intel_swap_word(data.size)));
	return(pf->lasterr);
}
/****************** rectangle operations *********************/

#ifdef DO_DRAWOPS
static Errcode do_frameSameRect(Pfile *pf)
/* for now pen size 1 only */
{
	if(pf->screen == NULL)
		return(Success);

	pj_set_hline(pf->screen,40,pf->lastRect.left,pf->lastRect.top,
							   pf->lastRWidth );
	pj_set_hline(pf->screen,40,pf->lastRect.left + 1,pf->lastRect.bot,
							   pf->lastRWidth );
	pj_set_vline(pf->screen,40,pf->lastRect.left,pf->lastRect.top + 1,
							   pf->lastRHeight );
	pj_set_vline(pf->screen,40,pf->lastRect.right,pf->lastRect.top,
							   pf->lastRHeight );
	return(Success);
}
static Errcode do_fillSameRect(Pfile *pf)
{
	if(pf->screen == NULL)
		return(Success);

	pj_set_rect(pf->screen,20,pf->lastRect.left,pf->lastRect.top,
							  pf->lastRWidth, pf->lastRHeight );
	return(Success);
}
#endif /* DO_DRAWOPS */

static Errcode do_rectop(Pfile *pf)
/* Covers opcodes 0030 to 0034, 0040 to 0044, 0050 to 0054, These 
 * opcodes use an 8 byte rectangle. */
{

#ifndef DO_DRAWOPS
	return(pf_seek_bytes(pf,sizeof(pRect)));
#else

	if(pf->screen == NULL)
		return(pf_seek_bytes(pf,sizeof(pRect)));

	if(read_pRect(pf, &pf->lastRect) < Success)
		return(pf->lasterr);

	/* rectangle is relative to picture */
	pf->lastRect.top -= pf->frame.top;
	pf->lastRect.bot -= pf->frame.top;
	pf->lastRect.left -= pf->frame.left;
	pf->lastRect.right -= pf->frame.left;

	/* pre calc width and height */
	pf->lastRWidth = pf->lastRect.right - pf->lastRect.left;
  	pf->lastRHeight = pf->lastRect.bot - pf->lastRect.top;

	/* call correponding render routine now that data is loaded */
	return((*low_vectors[pf->op+8])(pf));

#endif /* DO_DRAWOPS */
}
