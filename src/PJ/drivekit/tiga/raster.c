/*---------------------------------------------------------------------*\

    RASTER.C

 Raster Manipulation module for Animator/386 TIGA Driver.

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

/*-----------------------------------------------------------------------

   INCLUDES

-----------------------------------------------------------------------*/

#include "stdio.h"
#include "tiga.h"
#include "tigahc.h"
#include "extend.h"
#include "typedefs.h"

/*-----------------------------------------------------------------------

   DEFINES

-----------------------------------------------------------------------*/
#define  BPP         8
#define  XOR_MODE    10
#define  NRM_MODE    0

#define  LBUFF_SIZE  1024                 /* Size of palette (256 * 4)     */
/*-----------------------------------------------------------------------

   STRUCTURES/TYPEDEFS

-----------------------------------------------------------------------*/
typedef union
   {
   SHORT *w;
   UBYTE *ub;
   BYTE  *b;
   } SPTR;

/*-----------------------------------------------------------------------

   FUNCTION PROTOTYPE DECLARATIONS

-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------

   EXTERNALS

-----------------------------------------------------------------------*/
extern CONFIG     tigaConfig;             /* Current display mode info.    */
extern ULONG      dispPitch;              /* Display pitch for cur. mode   */
extern ULONG      dispAddr;               /* TI Address of display         */
extern ULONG      buffAddr;               /* Transfer buffer address       */
extern ULONG      buffSize;               /* Size of buffer                */
extern SHORT      moduleNum;              /* Used by the RLM function calls*/

extern   void  PrintText(char *string);   /* These routines used for debug */
extern   void  PrintHex(USHORT);
extern   void  PrintDec(USHORT);
extern   void  PrintLHex(ULONG);
extern   void  PrintLDec(ULONG);
extern   void  PrintVar(char *string, USHORT num);
extern   void  PrintHexDump(UBYTE *dataArea, USHORT numBytes);
extern   void  WaitKey();
extern   ULONG GetTIDeb(USHORT);
extern   ULONG SetTIDeb();

/*-----------------------------------------------------------------------

   STATIC VARIABLES

-----------------------------------------------------------------------*/
static struct rastlib   tiga_raster_library;
static UBYTE            localBuffer[LBUFF_SIZE]; /* Palette & transfr buffr*/

/*-----------------------------------------------------------------------

   UNINITIALIZED GLOBAL VARIABLES

-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------

   INITIALIZED GLOBAL VARIABLES

-----------------------------------------------------------------------*/

/************************************************************************

   ACTUAL CODE

************************************************************************/
/*-----------------------------------------------------------------------

   tiga_close_raster

   Closes the raster opened by tiga_open_graphics. Since
   tiga_close_graphics performs all the necessary closing of stuff,
   this function actually doesn't do anything.

-----------------------------------------------------------------------*/
Errcode tiga_close_raster(tigaRast *r)
{
#ifdef DEBUG
   printf("Raster closing after following calls:\n"
         "\tget_dot    %6d\n"
         "\tput_dot    %6d\n"
         "\tset_colors  %6d\n",
         r->hw.nm.gets, r->hw.nm.puts, r->hw.nm.colors);
#endif

   return(Success);                       /* Don't do anything             */
}

/*-----------------------------------------------------------------------

   tiga_put_dot

   Puts a dot on the display.

-----------------------------------------------------------------------*/
void tiga_put_dot(tigaRast *r, Pixel color, Coor x, Coor y)
{
#ifdef DEBUG
   ++r->hw.nm.puts;
#endif

   put_pixel(color, x, y);                /* Set the color, then the point.*/
   return;

}

/*-----------------------------------------------------------------------

   tiga_get_dot

   Gets a dot from the display.

-----------------------------------------------------------------------*/
Pixel tiga_get_dot(tigaRast *r, Coor x, Coor y)
{
#ifdef DEBUG
   ++r->hw.nm.gets;
#endif

   return(get_pixel(x, y));               /* Get the pixel                 */
}

/*-----------------------------------------------------------------------

   tiga_set_colors

   Sets the TIGA color palette.

-----------------------------------------------------------------------*/
void tiga_set_colors(tigaRast *r, LONG start, LONG count, unsigned char *table)
{
   USHORT      i;
   UBYTE       *tmpPtr = (UBYTE *)localBuffer;

#ifdef DEBUG
   ++r->hw.nm.colors;
#endif

   if (count <= 0)
      return;                             /* Ignore invalid count          */

   for (i = 0; i < count; i++)
      {
      *tmpPtr++ = *table++;               /* Copy red value                */
      *tmpPtr++ = *table++;               /* Copy green value              */
      *tmpPtr++ = *table++;               /* Copy blue value               */
      *tmpPtr++;                          /* Account for TI's "i" value    */
      }

   set_palet(count, start, (PALET *)localBuffer); /* Set the palette       */

   return;
}

/*-----------------------------------------------------------------------

   tiga_put_hseg

   Puts a one-line horiz. chunk of data onto the display. The only ways
   to possibly make this faster would be to either use board-specific
   transfer methods, or use an on-board routine to take the source
   word data and align it on the display properly. It's likely that
   this latter approach is already used within the TIGA core.

-----------------------------------------------------------------------*/
void tiga_put_hseg(tigaRast *r, void *srcBuffer, Ucoor x, Ucoor y, Ucoor w)
{
   /*
      Use a TI function to send the strip of data to the board.
      Parameter sequence:

      srcBuffer      - Source address in PC memory.
      w              - Arbitrary source bitmap pitch (it would count
                       if more than one line of data were being sent).
      dispAddr       - TI Memory address of the active display page.
      dispPitch      - Pitch (in bits) of the active TI display page.
      0, 0           - Source position.
      x, y           - Destination position.
      w              - X extent
      1              - Y extent (one line only)
      BPP            - Number of bits per pixel
      0              - Don't swizzle (invert order of bits)
   */

   host2gspxy(srcBuffer, w, dispAddr, dispPitch, 0, 0, x, y, w, 1, BPP, 0);
   return;

}

/*-----------------------------------------------------------------------

   tiga_get_hseg

   Gets a one-line horiz. chunk of data from the display. The only ways
   to possibly make this faster would be to either use board-specific
   transfer methods, or use an on-board routine to take the source
   word data and align properly for transmission to the system. It's
   likely that this latter approach is already used within the TIGA core.

-----------------------------------------------------------------------*/
void tiga_get_hseg(tigaRast *r, void *dstBuffer, Ucoor x, Ucoor y, Ucoor w)
{
   /*
      Use a TI function to get the strip of data from the board.
      Parameter sequence:

      dispAddr       - TI Memory address of the active display page.
      dispPitch      - Pitch (in bits) of the active TI display page.
      dstBuffer      - Destination address in PC memory.
      w              - Arbitrary destination bitmap pitch (it would count
                       if more than one line of data were being sent).
      x, y           - Source position.
      0, 0           - Destination position.
      w              - X extent
      1              - Y extent (one line only)
      BPP            - Number of bits per pixel
      0              - Don't swizzle (invert order of bits)
   */

   gsp2hostxy(dispAddr, dispPitch, dstBuffer, w, x, y, 0, 0, w, 1, BPP, 0);
   return;

}

/*-----------------------------------------------------------------------

   tiga_put_vseg

   Puts a one-line vert. chunk of data onto the display. The only ways
   to possibly make this faster would be to either use board-specific
   transfer methods, or use an on-board routine to take the source
   word data and align it on the display properly. It's likely that
   this latter approach is already used within the TIGA core.

-----------------------------------------------------------------------*/
void tiga_put_vseg(tigaRast *r, void *srcBuffer, Ucoor x, Ucoor y, Ucoor h)
{
   /*
      Use a TI function to send the strip of data to the board.
      Parameter sequence:

      srcBuffer      - Source address in PC memory.
      1              - Source bitmap pitch.
      dispAddr       - TI Memory address of the active display page.
      dispPitch      - Pitch (in bits) of the active TI display page.
      0, 0           - Source position.
      x, y           - Destination position.
      1              - X extent (one pixel wide)
      h              - Y extent
      BPP            - Number of bits per pixel
      0              - Don't swizzle (invert order of bits)
   */

   host2gspxy(srcBuffer, 1, dispAddr, dispPitch, 0, 0, x, y, 1, h, BPP, 0);
   return;

}

/*-----------------------------------------------------------------------

   tiga_get_vseg

   Gets a one-line vert. chunk of data from the display. The only ways
   to possibly make this faster would be to either use board-specific
   transfer methods, or use an on-board routine to take the source
   word data and align properly for transmission to the system. It's
   likely that this latter approach is already used within the TIGA core.

-----------------------------------------------------------------------*/
void tiga_get_vseg(tigaRast *r, void *dstBuffer, Ucoor x, Ucoor y, Ucoor h)
{
   /*
      Use a TI function to get the strip of data from the board.
      Parameter sequence:

      dispAddr       - TI Memory address of the active display page.
      dispPitch      - Pitch (in bits) of the active TI display page.
      dstBuffer      - Destination address in PC memory.
      1              - Destination bitmap pitch
      x, y           - Source position.
      0, 0           - Destination position.
      1              - X extent (one pixel wide)
      h              - Y extent
      BPP            - Number of bits per pixel
      0              - Don't swizzle (invert order of bits)
   */

   gsp2hostxy(dispAddr, dispPitch, dstBuffer, 1, x, y, 0, 0, 1, h, BPP, 0);
   return;

}

/*-----------------------------------------------------------------------

   tiga_put_rectpix

   Puts a rectangular chunk of data onto the display. The only ways
   to possibly make this faster would be to either use board-specific
   transfer methods, or use an on-board routine to take the source
   word data and align it on the display properly. It's likely that
   this latter approach is already used within the TIGA core.

-----------------------------------------------------------------------*/
void tiga_put_rectpix(tigaRast *r, void *srcBuffer,
                      Coor x, Coor y, Ucoor w, Ucoor h)
{
   /*
      Use a TI function to send the strip of data to the board.
      Parameter sequence:

      srcBuffer      - Source address in PC memory.
      w              - Source bitmap pitch
      dispAddr       - TI Memory address of the active display page.
      dispPitch      - Pitch (in bits) of the active TI display page.
      0, 0           - Source position.
      x, y           - Destination position.
      w              - X extent
      h              - Y extent
      BPP            - Number of bits per pixel
      0              - Don't swizzle (invert order of bits)
   */

   host2gspxy(srcBuffer, w, dispAddr, dispPitch, 0, 0, x, y, w, h, BPP, 0);
   return;

}

/*-----------------------------------------------------------------------

   tiga_get_rectpix

   Gets a rectangular chunk of data from the display. The only ways
   to possibly make this faster would be to either use board-specific
   transfer methods, or use an on-board routine to take the source
   word data and align properly for transmission to the system. It's
   likely that this latter approach is already used within the TIGA core.

-----------------------------------------------------------------------*/
void tiga_get_rectpix(tigaRast *r, void *dstBuffer,
                      Coor x, Coor y, Ucoor w, Ucoor h)
{
   /*
      Use a TI function to get the strip of data from the board.
      Parameter sequence:

      dispAddr       - TI Memory address of the active display page.
      dispPitch      - Pitch (in bits) of the active TI display page.
      dstBuffer      - Destination address in PC memory.
      w              - Destination bitmap pitch.
      x, y           - Source position.
      0, 0           - Destination position.
      w              - X extent
      h              - Y extent
      BPP            - Number of bits per pixel
      0              - Don't swizzle (invert order of bits)
   */

   gsp2hostxy(dispAddr, dispPitch, dstBuffer, w, x, y, 0, 0, w, h, BPP, 0);
   return;

}

/*-----------------------------------------------------------------------

   tiga_set_hline

   Draws a horizontal line on the display.

-----------------------------------------------------------------------*/
void tiga_set_hline(tigaRast *r, Pixel color, Coor x, Coor y, Ucoor w)
{
   set_fcolor(color);                     /* Set the drawing color         */
   draw_line(x, y, x+w-1, y);             /* Draw the line                 */
   return;
}

/*-----------------------------------------------------------------------

   tiga_set_vline

   Draws a vertical line on the display.

-----------------------------------------------------------------------*/
void tiga_set_vline(tigaRast *r, Pixel color, Coor x, Coor y, Ucoor h)
{
   set_fcolor(color);                     /* Set the drawing color         */
   draw_line(x, y, x, y+h-1);             /* Draw the line                 */
   return;
}

/*-----------------------------------------------------------------------

   tiga_set_rect

   Draws a filled rectangle on the display.

-----------------------------------------------------------------------*/
void tiga_set_rect(tigaRast *r, Pixel color,
                   Coor x, Coor y, Ucoor w, Ucoor h)
{
   set_fcolor(color);                     /* Set the drawing color         */
   fill_rect(w, h, x, y);                 /* Draw the rectangle            */
   return;
}

/*-----------------------------------------------------------------------

   tiga_set_rast

   Clears the display to a given color.

-----------------------------------------------------------------------*/
void tiga_set_rast(tigaRast *r, Pixel color)
{
   clear_frame_buffer(color);             /* Clear the whole frame buffer  */
   return;
}

/*-----------------------------------------------------------------------

   tiga_xor_rect

   Draws a filled XOR'd rectangle on the display.

-----------------------------------------------------------------------*/
void tiga_xor_rect(tigaRast *r, Pixel color,
                   Coor x, Coor y, Ucoor w, Ucoor h)
{
   set_fcolor(color);                     /* Set the drawing color         */
   set_ppop(XOR_MODE);                    /* Set the XOR raster op.        */
   fill_rect(w, h, x, y);                 /* Draw the rectangle            */
   set_ppop(NRM_MODE);                    /* Set the DST=SRC mode(restore!)*/
   return;
}

/*-----------------------------------------------------------------------

   tiga_blit_in_card

   Does an on-board BitBLT.

-----------------------------------------------------------------------*/
void tiga_blit_in_card(tigaRast *rSrc, Coor srcX, Coor srcY,
                       tigaRast *rDst, Coor dstX, Coor dstY,
                       Coor width, Coor height)
{
   /* Ignore multiple raster case for now, assume rSrc = rDst. */

   bitblt(width, height, srcX, srcY, dstX, dstY);

   return;
}

/*-----------------------------------------------------------------------

   tiga_blit_to_ram

   Blit a section of display memory to a PC memory buffer.

-----------------------------------------------------------------------*/
void tiga_blit_to_ram(tigaRast *rSrc, Coor srcX, Coor srcY,
                      tigaRast *rDst, Coor dstX, Coor dstY,
                      Coor width, Coor height)
{
   gsp2hostxy(dispAddr, dispPitch, rDst->hw.bm.bp[0], rDst->hw.bm.bpr,
               srcX, srcY, dstX, dstY, width, height, BPP, 0);

   return;
}

/*-----------------------------------------------------------------------

   tiga_blit_from_ram

   Blit a section of display memory from a PC memory buffer.

-----------------------------------------------------------------------*/
void tiga_blit_from_ram(tigaRast *rSrc, Coor srcX, Coor srcY,
                        tigaRast *rDst, Coor dstX, Coor dstY,
                        Coor width, Coor height)
{
   host2gspxy(rSrc->hw.bm.bp[0], rSrc->hw.bm.bpr, dispAddr, dispPitch,
               srcX, srcY, dstX, dstY, width, height, BPP, 0);

   return;
}

/*-----------------------------------------------------------------------

   tiga_mask1blit

   Perform a monochrome to color blit expansion with 0 bits transparent.
   Uses on-board function to perform the operation. Data is passed into
   a buffer first, then the on-board function is called.

-----------------------------------------------------------------------*/
void tiga_mask1blit(UBYTE *monoData, Coor monoBytePitch, Coor mX, Coor mY,
                    tigaRast *r, Coor rX, Coor rY, Ucoor width, Ucoor height,
                    Pixel onColor)
{

   USHORT   numLines, linesLeft, i, byteWidth;
   UBYTE    *lPtr = monoData, *dPtr = localBuffer;
   ULONG    bufferAddr = buffAddr, temp;

   if (monoBytePitch <= 64)
      {
      /*
         Send full blocks of data - this means skip mY lines of binary
         data, and then copy a complete block of data into the buffer
      */

      lPtr += mY * monoBytePitch;         /* Start at line mY              */

      linesLeft = height;

      temp = buffSize / monoBytePitch; /* Get # of lines buffer can hold*/

      while (linesLeft)
         {
         if (temp < linesLeft)            /* If less than desired...       */
            numLines = temp;              /*   Copy only amount possible   */
         else
            numLines = linesLeft;         /* Or do all lines if possible   */
                                          /* Copy block up to the board    */
         synchronize();                   /* Synchronize with board        */
         host2gsp(lPtr, buffAddr, monoBytePitch * numLines, 1);

         TIMask1Blit(monoBytePitch, mX, 0, rX, rY, width, numLines, onColor);

         lPtr += numLines * monoBytePitch;
         linesLeft -= numLines;           /* Diminish number of lines left */
         rY += numLines;                  /* Bump screen position down     */
         }
      }
   else
      {
      /* Only copy necessary portions */

      lPtr += mY * monoBytePitch + mX >> 3;/* Start at line mY, position mX*/

      linesLeft = height;
      byteWidth = (width + 7 + (mX & 7)) >> 3;  /* Get width of block      */

      if (byteWidth * height <= LBUFF_SIZE)
         {                                /* If small enough, then let's   */
         i = height;                      /*  copy everything into a local */
         while (i--)                      /*  buffer and make only one     */
            {                             /*  TI transfer call.            */
            memcpy(dPtr, lPtr, byteWidth);/* Copy one line locally @ a time*/
            lPtr += monoBytePitch;
            dPtr += byteWidth;            /* Increase buffer pointers      */
            }
         synchronize();                   /* Synchronize with board        */
                                          /* Copy the block up to the board*/
         host2gsp(localBuffer, bufferAddr, byteWidth * height, 1);
                                          /* Perform the Mask Blit oper.   */
         TIMask1Blit(byteWidth, (mX & 7), 0,
                     rX, rY, width, height, onColor);
         }
      else
         {
         temp = buffSize / monoBytePitch; /* Get # of lines buffer can hold*/

         while (linesLeft)
            {
            if (temp < linesLeft)         /* If less than desired...       */
               numLines = temp;           /*   Copy only amount possible   */
            else
               numLines = linesLeft;      /* Or do all lines if possible   */
                                          /* Copy blocks up to the board   */
            i = numLines;
            synchronize();                /* Synchronize with board        */
            while (i--)
               {
               host2gsp(lPtr, bufferAddr, byteWidth, 1);
               lPtr += monoBytePitch;
               bufferAddr += byteWidth << 3; /* Increase buffer pointer    */
               }
                                          /* Perform the Mask Blit oper.   */
            TIMask1Blit(byteWidth, (mX & 7), 0,
                        rX, rY, width, numLines, onColor);

            linesLeft -= numLines;        /* Diminish number of lines left */
            rY += numLines;               /* Bump screen position down     */
            }
         }
      }


   return;
}

/*-----------------------------------------------------------------------

   tiga_mask2blit

   Perform a monochrome to color blit expansion with 0 bits transparent.
   Uses on-board function to perform the operation. Data is passed into
   a buffer first, then the on-board function is called.

-----------------------------------------------------------------------*/
void tiga_mask2blit(UBYTE *monoData, Coor monoBytePitch, Coor mX, Coor mY,
                    tigaRast *r, Coor rX, Coor rY, Ucoor width, Ucoor height,
                    Pixel onColor, Pixel offColor)
{

   USHORT   numLines, linesLeft, i, byteWidth;
   UBYTE    *lPtr = monoData, *dPtr = localBuffer;
   ULONG    bufferAddr = buffAddr, temp;

   if (monoBytePitch <= 64)
      {
      /*
         Send full blocks of data - this means skip mY lines of binary
         data, and then copy a complete block of data into the buffer
      */

      lPtr += mY * monoBytePitch;         /* Start at line mY              */

      linesLeft = height;

      temp = buffSize / monoBytePitch; /* Get # of lines buffer can hold*/

      while (linesLeft)
         {
         if (temp < linesLeft)            /* If less than desired...       */
            numLines = temp;              /*   Copy only amount possible   */
         else
            numLines = linesLeft;         /* Or do all lines if possible   */
                                          /* Copy block up to the board    */
         synchronize();                   /* Synchronize with board        */
         host2gsp(lPtr, buffAddr, monoBytePitch * numLines, 1);

         TIMask2Blit(monoBytePitch, mX, 0, rX, rY, width, numLines,
                     onColor, offColor);

         lPtr += numLines * monoBytePitch;
         linesLeft -= numLines;           /* Diminish number of lines left */
         rY += numLines;                  /* Bump screen position down     */
         }
      }
   else
      {
      /* Only copy necessary portions */

      lPtr += mY * monoBytePitch + mX >> 3;/* Start at line mY, position mX*/

      linesLeft = height;
      byteWidth = (width + 7 + (mX & 7)) >> 3;  /* Get width of block      */

      if (byteWidth * height <= LBUFF_SIZE)
         {                                /* If small enough, then let's   */
         i = height;                      /*  copy everything into a local */
         while (i--)                      /*  buffer and make only one     */
            {                             /*  TI transfer call.            */
            memcpy(dPtr, lPtr, byteWidth);/* Copy one line locally @ a time*/
            lPtr += monoBytePitch;
            dPtr += byteWidth;            /* Increase buffer pointers      */
            }
         synchronize();                   /* Synchronize with board        */
                                          /* Copy the block up to the board*/
         host2gsp(localBuffer, bufferAddr, byteWidth * height, 1);
                                          /* Perform the Mask Blit oper.   */
         TIMask2Blit(byteWidth, (mX & 7), 0, rX, rY,
                     width, height, onColor, offColor);
         }
      else
         {
         temp = buffSize / monoBytePitch; /* Get # of lines buffer can hold*/

         while (linesLeft)
            {
            if (temp < linesLeft)         /* If less than desired...       */
               numLines = temp;           /*   Copy only amount possible   */
            else
               numLines = linesLeft;      /* Or do all lines if possible   */
                                          /* Copy blocks up to the board   */
            i = numLines;
            synchronize();                /* Synchronize with board        */
            while (i--)
               {
               host2gsp(lPtr, bufferAddr, byteWidth, 1);
               lPtr += monoBytePitch;
               bufferAddr += byteWidth << 3; /* Increase buffer pointer    */
               }
                                          /* Perform the Mask Blit oper.   */
            TIMask2Blit(byteWidth, (mX & 7), 0,
                        rX, rY, width, numLines, onColor, offColor);

            linesLeft -= numLines;        /* Diminish number of lines left */
            rY += numLines;               /* Bump screen position down     */
            }
         }
      }

   return;
}

/*-----------------------------------------------------------------------

   tiga_unss2_rect

   New style Animator delta-decompression.

   Since no length count of the source data is available, we need to
   parse the decode data ourselves without actually performing the
   decompression.
      
-----------------------------------------------------------------------*/
void tiga_unss2_rect(tigaRast *r, void *dataBuff, LONG pixSize,
                     Coor x, Coor y, Ucoor width, Ucoor height)

{
   SPTR     sPtr;
   UBYTE    *lastPtr;
   UBYTE    *lastLine;
   USHORT   lineCnt, tempLineCnt, lastY = y;
   SHORT    opWord;
   SHORT    sizeCnt;
   USHORT   copySize;


   sPtr.w = dataBuff;
   lineCnt = *sPtr.w++;                   /* Get Line count                */
   lastPtr = sPtr.ub;                     /* Save pointer to current start */

   tempLineCnt = lineCnt;                 /* Keep original line count      */

   while (tempLineCnt)
      {
      lastLine = sPtr.ub;                 /* Save for later                */
      opWord = *sPtr.w++;
      if (opWord < 0)
         {
         if (opWord & 0x4000)
            y -= opWord;                  /* Add negative # of lines       */
         }
      else                                /* opWord is non-negative        */
         {
         while (opWord)
            {
            sPtr.ub++;                    /* Ignore the X Skip value       */
            sizeCnt = *sPtr.b++;
            if (sizeCnt < 0)              /* Check if dupe or copy...      */
               {                          /* Duplicate                     */
               sPtr.w++;                  /* Skip the dupe byte            */
               opWord--;
               }
            else
               {                          /* Copy data                     */
               sPtr.w += sizeCnt;         /* Bump up the pointer           */
               opWord--;
               }
            }
         copySize = sPtr.ub - lastPtr + 1;/* Get size of current data      */
         if (copySize > buffSize)         /* Is there too much data?       */
            {
            copySize = lastLine - lastPtr + 1;/* Get a valid copy size     */
            synchronize();
            host2gsp(lastPtr, buffAddr, copySize, 0);
            TIUnSS2Rect(x, lastY, width, height, lineCnt - tempLineCnt);
            lastPtr = lastLine;           /* Set last to new position      */
            lineCnt = tempLineCnt;
            lastY = y;
            }
         tempLineCnt--;
         y++;
         }
      }

   copySize = sPtr.ub - lastPtr + 1;      /* Get size of current data      */
   synchronize();                         /* Do last part                  */
   host2gsp(lastPtr, buffAddr, copySize, 0);
   TIUnSS2Rect(x, lastY, width, height, lineCnt);



   return;
}

/*-----------------------------------------------------------------------

   tiga_unlccomp_rect

   Old style Animator delta-decompression.

   Since no length count of the source data is available, we need to
   parse the decode data ourselves without actually performing the
   decompression.
      
-----------------------------------------------------------------------*/
void tiga_unlccomp_rect(tigaRast *r, void *dataBuff, LONG pixSize,
                        Coor x, Coor y, Ucoor width, Ucoor height)

{
   UBYTE    *lastPtr;
   UBYTE    *lastLine;
   UBYTE    *bPtr;
   USHORT   *wPtr;
   USHORT   lineCnt, lineSkip, tempLineCnt;
   USHORT   packetCnt, lastY = y;
   SHORT    sizeCnt;
   USHORT   copySize;


   wPtr = dataBuff;

   lineSkip = *wPtr++;                    /* Get line skip value           */
   lineCnt = *wPtr++;                     /* Get Line count                */

   bPtr = (UBYTE *) wPtr;                 /* Point to Packet data          */
   lastPtr = bPtr;                        /* Save pointer to current start */

   tempLineCnt = lineCnt;                 /* Keep original line count      */

   while (tempLineCnt--)
      {
      lastLine = bPtr;                    /* Save for later                */
      packetCnt = *bPtr++;
      while (packetCnt > 0)
         {
         bPtr++;                          /* Ignore the X Skip value       */
         sizeCnt = *(BYTE *)bPtr++;       /* Get signed value              */
         if (sizeCnt < 0)                 /* Check if dupe or copy...      */
            {                             /* Duplicate                     */
            bPtr++;                       /* Skip the dupe byte            */
            packetCnt--;
            }
         else
            {                             /* Copy data                     */
            bPtr += sizeCnt;              /* Bump up the pointer           */
            packetCnt--;
            }
         }
      copySize = bPtr - lastPtr + 1;      /* Get size of current data      */
      if (copySize > buffSize)            /* Is there too much data?       */
         {
         copySize = lastLine - lastPtr + 1;/* Get a valid copy size         */
         synchronize();
         host2gsp(lastPtr, buffAddr, copySize, 0);
         TIUnLCCRect(x, lastY, width, height,
                     lineSkip, (lineCnt - tempLineCnt - 1));
         lastPtr = lastLine;              /* Set last to new position      */
         lineCnt = tempLineCnt + 1;       /* Compensate for extra dec      */
         lastY = y;
         }
      y++;
      }

   copySize = bPtr - lastPtr + 1;         /* Get size of current data      */
   synchronize();                         /* Do last part                  */

   host2gsp(lastPtr, buffAddr, copySize, 0);
   TIUnLCCRect(x, lastY, width, height, lineSkip, lineCnt);

   return;
}

/*-----------------------------------------------------------------------

   tiga_get_rlib

   Returns the address of the TIGA raster function library structure.
   On the first call, it also initializes the structure.

-----------------------------------------------------------------------*/
struct rastlib *tiga_get_rlib(Vdevice *dev, int mode, tigaRast *r)
{
static got_lib = FALSE;

if (!got_lib)
   {
   tiga_raster_library.close_raster = tiga_close_raster;
   tiga_raster_library.put_dot = tiga_put_dot;
   tiga_raster_library.get_dot = tiga_get_dot;
   tiga_raster_library.set_colors = tiga_set_colors;
   tiga_raster_library.put_hseg = tiga_put_hseg;
   tiga_raster_library.get_hseg = tiga_get_hseg;
   tiga_raster_library.put_vseg = tiga_put_vseg;
   tiga_raster_library.get_vseg = tiga_get_vseg;
   tiga_raster_library.put_rectpix = tiga_put_rectpix;
   tiga_raster_library.get_rectpix = tiga_get_rectpix;
   tiga_raster_library.set_hline = tiga_set_hline;
   tiga_raster_library.set_vline = tiga_set_vline;
   tiga_raster_library.set_rect = tiga_set_rect;
   tiga_raster_library.set_rast = tiga_set_rast;
   tiga_raster_library.xor_rect = tiga_xor_rect;
   tiga_raster_library.blitrect[0] = tiga_blit_in_card;
   tiga_raster_library.blitrect[2] = tiga_blit_from_ram;
   tiga_raster_library.mask1blit = tiga_mask1blit;
   tiga_raster_library.mask2blit = tiga_mask2blit;
   tiga_raster_library.unlccomp_rect = tiga_unlccomp_rect;
   tiga_raster_library.unss2_rect = tiga_unss2_rect;

/*

   The following reports errors in TORTURE for unknown reasons:

      tiga_raster_library.blitrect[1] = tiga_blit_to_ram;


   The following routines have not been implemented.

      tiga_raster_library.swaprect[0] = tiga_swap_in_card;
      tiga_raster_library.swaprect[1] = tiga_swap_to_ram;
      tiga_raster_library.swaprect[2] = tiga_swap_from_ram;
      tiga_raster_library.tblitrect[0] = tiga_tblit_in_card;
      tiga_raster_library.tblitrect[1] = tiga_tblit_to_ram;
      tiga_raster_library.tblitrect[2] = tiga_tblit_from_ram;
      tiga_raster_library.unbrun_rect = tiga_unbrun_rect;
      tiga_raster_library.uncc256 = tiga_uncc256;
      tiga_raster_library.uncc64 = tiga_uncc64;
*/
   got_lib = TRUE;
   }
return(&tiga_raster_library);
}

