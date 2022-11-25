#ifndef PLEXP_H
#define PLEXP_H


/* Phar Lap .EXP file definitions */
/************************************************************************/
/*	Copyright (C) 1986-1988 Phar Lap Software, Inc.			*/
/*	Unpublished - rights reserved under the Copyright Laws of the	*/
/*	United States.  Use, duplication, or disclosure by the 		*/
/*	Government is subject to restrictions as set forth in 		*/
/*	subparagraph (c)(1)(ii) of the Rights in Technical Data and 	*/
/*	Computer Software clause at 252.227-7013.			*/
/*	Phar Lap Software, Inc., 60 Aberdeen Ave., Cambridge, MA 02138	*/
/************************************************************************/

/* Old .EXP file header */

typedef struct
{
	USHORT exe_sign;		/* [00] Signature word (MP) */
	USHORT exe_szrem;		/* [02] Remainder of the image size
					        when divided by the page
					        size (512 bytes) */
	USHORT exe_size;		/* [04] Size of image in pages */
	USHORT exe_relcnt;		/* [06] Number of relocation entries*/
	USHORT exe_hsize;		/* [08] Header size in paragraphs */
	USHORT exe_minpg;		/* [0A] Minimum number of extra 
					        4K pages to be allocated
					        at the end of a program when
					        it is loaded */
	USHORT exe_maxpg;		/* [0C] Maximum number of extra
					        4K pages to be allocated
					        at the end of a program when
					        it is loaded */
	ULONG	exe_esp;		/* [0E] Initial ESP */
	USHORT exe_chksum;		/* [12] Word checksum of file */
	ULONG	exe_eip;		/* [14] Initial EIP */
	USHORT exe_reloff;		/* [18] Offset of first relocation
					        item */
	USHORT exe_ovlno;		/* [1A] Overlay number */
	USHORT exe_unkw;		/* [1C] Unknown word, wants to be 1 */
} OEXP_HDR;


/* .EXP file header */

typedef struct
{

	/* Signature and level */

	USHORT exp_sign;		/* Signature word (P2 or P3) */
	USHORT exp_level;		/* Level */
	USHORT exp_hsize;		/* Header size */
	ULONG exp_flsize;		/* File size in bytes */
	USHORT exp_chksum;		/* Checksum */

	/* Table of contents */

	ULONG exp_rtp;			/* File offset of the run-time
					   parameters */
	ULONG exp_rtpsize;		/* Size in bytes of the run-time
					   parameters */	
	ULONG exp_rel;			/* File offset of the relocation
					   table */
	ULONG exp_relsize;		/* Size in bytes of the relocation
					   table */
	ULONG exp_sit;			/* File offset of the segment
					   information table */
	ULONG exp_sitsize;		/* Size in bytes of the segment
					   information table */
	USHORT exp_sitesize;		/* Size in bytes of a segment
					   information table entry */
	ULONG exp_ldimg;		/* File offset of the load image */
	ULONG exp_ldisize;		/* Load image size on disk */
	ULONG exp_sym;			/* File offset of the symbol table */
	ULONG exp_symsize;		/* Size in bytes of the symbol table*/
	ULONG exp_gdt;			/* GDT offset in load image */
	ULONG exp_gdtsize;		/* Size in bytes of the GDT */
	ULONG exp_ldt;			/* LDT offset in load image */
	ULONG exp_ldtsize;		/* Size in bytes of the LDT */
	ULONG exp_idt;			/* IDT offset in load image */
	ULONG exp_idtsize;		/* Size in bytes of the IDT */
	ULONG exp_tss;			/* TSS offset in load image */
	ULONG exp_tsssize;		/* Size in bytes of the TSS */

	/* Program load params, level 1 files only */

	ULONG exp_minext;		/* Minimum number of extra 
					   bytes to be allocated
					   at the end of a program when
					   it is loaded */
	ULONG exp_maxext;		/* Maximum number of extra
					   bytes to be allocated
					   at the end of a program when
					   it is loaded */
	ULONG exp_boff;			/* Base load offset */

	/* Initial register contents */

	ULONG exp_iesp;			/* Initial ESP */
	USHORT exp_iss;			/* Initial SS */
	ULONG exp_ieip;			/* Initial EIP */
	USHORT exp_ics;			/* Initial CS */
	USHORT exp_ildt;		/* Initial LDT */
	USHORT exp_itss;		/* Initial TSS */

	/* New stuff */

	USHORT exp_flags;		/* Flags */
	ULONG exp_mreq;			/* Memory requirements for the load
					   image (packed or unpacked) */
	ULONG exp_xsum32;		/* 32-bit checksum */

	/* An .EXP header is 384 bytes long.  The remaining bytes of the
	   header are always zero, to allow for future expansion. */

} EXP_HDR;

/*

Flags in exp_flags word.

*/

#define EXP_PACKED 0x0001	/* Load image is packed */
#define EXP_XSUM 0x0002		/* 32-bit checksum is present */

/*

Segment information table entry

*/

typedef struct
{
	USHORT sit_selector;		/* Selector number */
	USHORT sit_flags;		/* Flags */
	ULONG sit_boff;			/* Base offset of selector */
	ULONG sit_minext;		/* Minimum number of extra bytes
					   to be allocated to the segment */
} SIT_ENT;


/*

386|DOS-Extender run-time parameters

*/

typedef struct
{
	USHORT rtp_sign;		/* Signature = 'DX' */
	USHORT rtp_minrf;		/* Minimum number of real mode paras
					   to leave free at run-time */
	USHORT rtp_maxrf;		/* Maximum number of real mode paras
					   to leave free at run-time */
	USHORT rtp_minib;		/* Minimum interrupt buffer size in
					   K bytes */
	USHORT rtp_maxib;		/* Maximum interrupt buffer size in
					   K bytes */
	USHORT rtp_nist;		/* Number of interrupt stacks */
	USHORT rtp_istks;		/* Size in K bytes of each interrupt
					   stack */
	ULONG rtp_rbr;			/* Offset where the real mode code/
					   data ends + 1 */
	USHORT rtp_cbsize;		/* Size in K bytes of the call 
					   buffers */
	USHORT rtp_flags;		/* Flags - bit def'ns below */

	/* The run-time parameters are 128 bytes long.  The remaining
	   bytes of the header are always zero to allow for future
	   expansion. */

} DOSX_RTP;

/*
 * DOS-X flags word bit definitions
 */
#define RTP_VMM	0x0001		/* File is virtual memory manager */

/*

Values for DOS-X run-time params

*/

#define REAL_MAX	64	/* max. legal value for minrf and maxrf */
#define MINR_DEF	0	/* default for minrf */
#define MAXR_DEF	0	/* default for maxrf */

#define IBUF_MIN	1	/* min. legal value for minib and maxib */
#define IBUF_MAX	64	/* max. legal value for minib and maxib */
#define MINIB_DEF	1	/* default for minib */
#define MAXIB_DEF	64	/* default for maxib */

#define NIST_MIN	4	/* min. legal value for nist */
#define ISTKS_MIN	1	/* min. legal value for istks */
#define ISTKS_MAX	64	/* max. legal value for nist * istks */
#define NIST_DEF	4	/* default for nist */
#define ISTKS_DEF	1	/* default for istks */

#define RBRK_DEF	0	/* default for rbr */

#define CBSIZE_MAX	64	/* max. legal value for cbsize */
#define CBSIZE_DEF	0	/* default for cbsize */


/*

File signatures 

*/

#define EXP_286 0x3250		/* 286 .EXP signature (P2) */

#define EXP_386 0x3350		/* 386 .EXP signature (P3) */

#define EXP_OLD 0x504D		/* Old .EXP signature (MP) */

#define REX_OLD 0x514D		/* Old .REX signature (MQ) */

#define RTP_SIGN 0x5844		/* 386|DOS-Extender run-time parameters */


/*

File header and run-time parameter sizes 

*/

#define EXP_SIZE 384		/* Size of an .EXP header */

#define RTP_SIZE 128		/* Size of the run-time parameters */

#define SIT_ESIZE 12		/* Size of an SIT entry */


/*

Level numbers

*/

#define LVL_FLAT  1		/* Flat model file */
#define LVL_MULTI 2		/* Multisegmented file */


/*

Repeat block header (for packed files)

*/

typedef struct
{
	USHORT rpt_bcnt;		/* Byte count */
	UBYTE rpt_rptl;			/* Repeat string length */
} EXP_RPT;

#endif /* PLEXP_H */
