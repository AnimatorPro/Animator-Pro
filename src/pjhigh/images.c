/* images.c - my collection of icons, some of them going back to 
   the Aegis Animator. */

#include "menus.h"


#define BITS8(a,b,c,d,e,f,g,h)\
	((a<<7)|(b<<6)|(c<<5)|(d<<4)|(e<<3)|(f<<2)|(g<<1)|(h))

#define BITS16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)\
	(BITS8(a,b,c,d,e,f,g,h)),(BITS8(i,j,k,l,m,n,o,p))

#define _ 0	
#define X 1


#ifdef SLUFFED
static UBYTE iinsert[] = 
	{
	0x10,0x38,0x7C,0x82,
	};
Image cinsert =	IMAGE_INIT1(ITYPE_BITPLANES,1,iinsert, 8, 4);

static UBYTE	ikill[] = {
		0x07, 0xC0, 0x0C, 0x60, 0x1E, 0x30, 0x13, 0x10, 0x11, 0x90,
		0x18, 0xF0, 0x0C, 0x60, 0x07, 0xC0, 
		};
Image ckill = IMAGE_INIT1(ITYPE_BITPLANES,1,ikill, 16, 8);
#endif /* SLUFFED */

static UBYTE izin[] = { /* 6x6 */
0xff^0x80, 0xff^0xe0, 0xff^0xe0, 0xff^0xd8, 0xff^0xb8, 0xff^0x7c, };
static Image zin = IMAGE_INIT1(ITYPE_BITPLANES,1,izin, 6, 6);

static UBYTE izout[] = { /* 6x6 */
0xff^0xf8, 0xff^0x74, 0xff^0x6c, 0xff^0x1c, 0xff^0x1c, 0xff^0x4, };
static Image zout = IMAGE_INIT1(ITYPE_BITPLANES,1,izout, 6, 6);

static UBYTE idown[] = 
	{
	0x10, 0x10, 0x10, 0x92, 0x7C, 0x38, 0x10,
	BITS8(_,_,_,X,_,_,_,_),
	BITS8(_,_,_,X,_,_,_,_),
	BITS8(_,_,_,X,_,_,_,_),
	BITS8(X,_,_,X,_,_,X,_),
	BITS8(_,X,X,X,X,X,_,_),
	BITS8(_,_,X,X,X,_,_,_),
	BITS8(_,_,_,X,_,_,_,_),
	};
Image cdown = 	IMAGE_INIT1(ITYPE_BITPLANES,1,idown, 7, 7);

static UBYTE ileft[] = 
	{
	BITS8(_,_,_,X,_,_,_,_),
	BITS8(_,_,X,_,_,_,_,_),
	BITS8(_,X,X,_,_,_,_,_),
	BITS8(X,X,X,X,X,X,X,X),
	BITS8(_,X,X,_,_,_,_,_),
	BITS8(_,_,X,_,_,_,_,_),
	BITS8(_,_,_,X,_,_,_,_),
	};
Image cleft = 	IMAGE_INIT1(ITYPE_BITPLANES,1,ileft, 8, 7);


static UBYTE iright[] = 
	{
	0x08,0x04,0x06,0xFF,0x06,0x04,0x08,
	BITS8(_,_,_,_,X,_,_,_),
	BITS8(_,_,_,_,_,X,_,_),
	BITS8(_,_,_,_,_,X,X,_),
	BITS8(X,X,X,X,X,X,X,X),
	BITS8(_,_,_,_,_,X,X,_),
	BITS8(_,_,_,_,_,X,_,_),
	BITS8(_,_,_,_,X,_,_,_),
	};
Image cright = 	IMAGE_INIT1(ITYPE_BITPLANES,1,iright, 8, 7);

static UBYTE iup[] = 
	{
	BITS8(_,_,_,X,_,_,_,_),
	BITS8(_,_,X,X,X,_,_,_),
	BITS8(_,X,X,X,X,X,_,_),
	BITS8(X,_,_,X,_,_,X,_),
	BITS8(_,_,_,X,_,_,_,_),
	BITS8(_,_,_,X,_,_,_,_),
	BITS8(_,_,_,X,_,_,_,_),
	};
Image cup = 	IMAGE_INIT1(ITYPE_BITPLANES,1,iup, 8, 7);


static UBYTE icright2[] = 
	{
	BITS16(X,_,_,_,_,X,_,_,_,_,_,_,_,_,_,_),
	BITS16(_,X,_,_,_,_,X,_,_,_,_,_,_,_,_,_),
	BITS16(_,X,X,_,_,_,X,X,_,_,_,_,_,_,_,_),
	BITS16(_,X,X,X,_,_,X,X,X,_,_,_,_,_,_,_),
	BITS16(_,X,X,_,_,_,X,X,_,_,_,_,_,_,_,_),
	BITS16(_,X,_,_,_,_,X,_,_,_,_,_,_,_,_,_),
	BITS16(X,_,_,_,_,X,_,_,_,_,_,_,_,_,_,_),
	};
Image cright2 = 	IMAGE_INIT1(ITYPE_BITPLANES,1,icright2, 9, 7);

static UBYTE ictridown[] = 
	{
	BITS8(_,X,X,X,X,X,X,_),
	BITS8(X,X,X,X,X,X,X,X),
	BITS8(_,X,X,X,X,X,X,_),
	BITS8(_,_,X,X,X,X,_,_),
	BITS8(_,_,_,X,X,_,_,_),
	};
Image ctridown = IMAGE_INIT1(ITYPE_BITPLANES,1,ictridown, 8, 5);

static UBYTE ictriup[] = 
	{
	BITS8(_,_,_,X,X,_,_,_),
	BITS8(_,_,X,X,X,X,_,_),
	BITS8(_,X,X,X,X,X,X,_),
	BITS8(X,X,X,X,X,X,X,X),
	BITS8(_,X,X,X,X,X,X,_),
	};
Image ctriup = IMAGE_INIT1(ITYPE_BITPLANES,1,ictriup, 8, 5);

static UBYTE circ3_mask[] = {
		BITS8(_,_,X,_,_,_,_,_),
		BITS8(_,X,X,X,_,_,_,_),
		BITS8(X,X,X,X,X,_,_,_),
		BITS8(_,X,X,X,_,_,_,_),
		BITS8(_,_,X,_,_,_,_,_),
		};
Image circ3_image = IMAGE_INIT1(ITYPE_BITPLANES,1,circ3_mask, 5, 5);



#undef BITS8
#undef BITS16
#undef _
#undef X

Image *leftright_arrs[2] = { &cleft, &cright };
Image *updown_arrs[2]	= { &cup, &cdown };
Image *zoutin_arrs[2]	= { &zout, &zin };

