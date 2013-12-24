/*---------------------------------------------------------------------*\

    TIGA.H

 Include file for PC side of AA386 TIGA driver.

 Copyright (C) 1989, 1990  Panacea Inc.

 Panacea Inc.
 50 Nashua Road, Suite 305
 Londonderry, New Hampshire, 03053-3444
 (603) 437-5022

 NOTICE:
   This code may be freely distributed and modified as long as the
   Panacea copyright notice above is maintained intact.
------------------------------------------------------------------------

Revision History:

Date     Who   What
======== ===   =======================================================
12/14/90 JBR   Initial Version
01/03/91 JBR   Converted over to PJ.48

\*---------------------------------------------------------------------*/
/* tiga.h - header file for the tiga driver */
#ifndef tiga_H
#define tiga_H

#ifndef STDTYPES_H
   #include "stdtypes.h"
#endif

#ifndef ERRCODES_H
   #include "errcodes.h"
#endif

#ifndef RASTER_H
   #include "raster.h"
#endif

typedef struct tiga_hw
   {
   long gets;        /* the number of times get_dot is called */
   long puts;        /* the number of times put_dot is called */
   long colors;      /* the number of times set_colors is called */
   struct vdevice *dev;  /* pointer back to tiga device. */
   } tiga_hw;

typedef union tiga_n_bmap
   {
   Bmap bm;
   tiga_hw nm;
   } NuBm;

typedef struct tigarast
   {
   RASTHDR_FIELDS;
   NuBm hw;
   } tigaRast;

/* Make sure that driver's hardware specific raster is the right size.
 * (Following code will force a compiler syntax error if it's not.) */
struct _rast_error_check_
   {
   char xx[sizeof(tigaRast) == sizeof(Raster)];
   };

/* This definition - LibRast - lets the compiler use your hardware specific 
 * raster structure as the "Raster" parameter to device and raster library
 * function vectors without the compiler requiring lots and lots of
 * type-casts or complaining about type-mismatches. It must be defined
 * _before_ rastlib.h or vdevice.h are included. */

#define LibRast tigaRast

#ifndef RASTLIB_H
   #include "rastlib.h"
#endif

#ifndef VDEVICE_H
   #include "vdevice.h"
#endif


/*
   This is the start of the RLM extension information...
*/

#define MODULE_NUMBER   moduleNum

/* define command numbers */
#define SET_BUFF_ADDR   USER_CP(MODULE_NUMBER | 0)
#define TIMASK1BLIT     USER_DM(MODULE_NUMBER | 1)
#define TIMASK2BLIT     USER_DM(MODULE_NUMBER | 2)
#define TIUNSS2         USER_DM(MODULE_NUMBER | 3)
#define TIUNLCC         USER_DM(MODULE_NUMBER | 4)

/*----------------------------------------------------------------------*/
/*  ADL downloaded function definition                                  */
/*----------------------------------------------------------------------*/
#define SetTIBuffAddr(a,b)    cp_cmd(SET_BUFF_ADDR,2,_DWORD(a),_DWORD(b))
#define TIMask1Blit(a,b,c,d,e,f,g,h) \
            dm_cmd(TIMASK1BLIT,8,0,(USHORT)(a),(USHORT)(b),(USHORT)(c), \
                                   (USHORT)(d),(USHORT)(e),(USHORT)(f), \
                                   (USHORT)(g),(USHORT)(h))
#define TIMask2Blit(a,b,c,d,e,f,g,h,i) \
            dm_cmd(TIMASK2BLIT,9,0,(USHORT)(a),(USHORT)(b),(USHORT)(c), \
                                   (USHORT)(d),(USHORT)(e),(USHORT)(f), \
                                   (USHORT)(g),(USHORT)(h),(USHORT)(i))
#define TIUnSS2Rect(a,b,c,d,e) \
            dm_cmd(TIUNSS2,5,0,(USHORT)(a),(USHORT)(b),(USHORT)(c), \
                               (USHORT)(d),(USHORT)(e))
#define TIUnLCCRect(a,b,c,d,e,f) \
            dm_cmd(TIUNLCC,6,0,(USHORT)(a),(USHORT)(b),(USHORT)(c), \
                               (USHORT)(d),(USHORT)(e),(USHORT)(f))




/*#define  DEBUG    1    */               /* Used to enable printf's       */


#endif /* tiga_H */

