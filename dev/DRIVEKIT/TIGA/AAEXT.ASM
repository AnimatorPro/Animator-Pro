;-----------------------------------------------------------------------------
;
; AAEXT.ASM
;
; This file contains 1) the TIGAEXT section defining the function names for
; the TIGA linking loader to install; and 2) All necessary routines for
; the TIGA side of the AA386 TIGA driver.
;
; Copyright (C) 1991  Panacea Inc. - All Rights Reserved
;
; Panacea Inc.
; 50 Nashua Road, Suite 305
; Londonderry, New Hampshire, 03053-3444
; (603) 437-5022
;
;
; NOTICE:
;   This code may be freely distributed and modified as long as the
;   Panacea copyright notice above is maintained intact.
;
;
;Revision History:
;
;Date      Who   Description
;========  ===   ====================================================
;01/22/91  JBR   Initial implementation.
;
;-----------------------------------------------------------------------------
;
; External References
;
           .global   _run_decode
;
;   INCLUDE FILES
;
           include   gspreg.inc
           include   gspglobs.inc
           mlib      gspmac.lib
;
; Section declaration for TIGA linker
;
           .sect   ".TIGAEXT"

           .long     _SetTIBuffAddr
           .long     _Mask1Blit
           .long     _Mask2Blit
           .long     _UnSS2_Rect
           .long     _UnLCC_Rect

           .data


buffAddr   .long     0
buffSize   .long     0


colorTable8:
           .long     00000000H, 01010101H, 02020202H, 03030303H
           .long     04040404H, 05050505H, 06060606H, 07070707H
           .long     08080808H, 09090909H, 0A0A0A0AH, 0B0B0B0BH
           .long     0C0C0C0CH, 0D0D0D0DH, 0E0E0E0EH, 0F0F0F0FH
           .long     10101010H, 11111111H, 12121212H, 13131313H
           .long     14141414H, 15151515H, 16161616H, 17171717H
           .long     18181818H, 19191919H, 1A1A1A1AH, 1B1B1B1BH
           .long     1C1C1C1CH, 1D1D1D1DH, 1E1E1E1EH, 1F1F1F1FH
           .long     20202020H, 21212121H, 22222222H, 23232323H
           .long     24242424H, 25252525H, 26262626H, 27272727H
           .long     28282828H, 29292929H, 2A2A2A2AH, 2B2B2B2BH
           .long     2C2C2C2CH, 2D2D2D2DH, 2E2E2E2EH, 2F2F2F2FH
           .long     30303030H, 31313131H, 32323232H, 33333333H
           .long     34343434H, 35353535H, 36363636H, 37373737H
           .long     38383838H, 39393939H, 3A3A3A3AH, 3B3B3B3BH
           .long     3C3C3C3CH, 3D3D3D3DH, 3E3E3E3EH, 3F3F3F3FH
           .long     40404040H, 41414141H, 42424242H, 43434343H
           .long     44444444H, 45454545H, 46464646H, 47474747H
           .long     48484848H, 49494949H, 4A4A4A4AH, 4B4B4B4BH
           .long     4C4C4C4CH, 4D4D4D4DH, 4E4E4E4EH, 4F4F4F4FH
           .long     50505050H, 51515151H, 52525252H, 53535353H
           .long     54545454H, 55555555H, 56565656H, 57575757H
           .long     58585858H, 59595959H, 5A5A5A5AH, 5B5B5B5BH
           .long     5C5C5C5CH, 5D5D5D5DH, 5E5E5E5EH, 5F5F5F5FH
           .long     60606060H, 61616161H, 62626262H, 63636363H
           .long     64646464H, 65656565H, 66666666H, 67676767H
           .long     68686868H, 69696969H, 6A6A6A6AH, 6B6B6B6BH
           .long     6C6C6C6CH, 6D6D6D6DH, 6E6E6E6EH, 6F6F6F6FH
           .long     70707070H, 71717171H, 72727272H, 73737373H
           .long     74747474H, 75757575H, 76767676H, 77777777H
           .long     78787878H, 79797979H, 7A7A7A7AH, 7B7B7B7BH
           .long     7C7C7C7CH, 7D7D7D7DH, 7E7E7E7EH, 7F7F7F7FH
           .long     80808080H, 81818181H, 82828282H, 83838383H
           .long     84848484H, 85858585H, 86868686H, 87878787H
           .long     88888888H, 89898989H, 8A8A8A8AH, 8B8B8B8BH
           .long     8C8C8C8CH, 8D8D8D8DH, 8E8E8E8EH, 8F8F8F8FH
           .long     90909090H, 91919191H, 92929292H, 93939393H
           .long     94949494H, 95959595H, 96969696H, 97979797H
           .long     98989898H, 99999999H, 9A9A9A9AH, 9B9B9B9BH
           .long     9C9C9C9CH, 9D9D9D9DH, 9E9E9E9EH, 9F9F9F9FH
           .long     0A0A0A0A0H, 0A1A1A1A1H, 0A2A2A2A2H, 0A3A3A3A3H
           .long     0A4A4A4A4H, 0A5A5A5A5H, 0A6A6A6A6H, 0A7A7A7A7H
           .long     0A8A8A8A8H, 0A9A9A9A9H, 0AAAAAAAAH, 0ABABABABH
           .long     0ACACACACH, 0ADADADADH, 0AEAEAEAEH, 0AFAFAFAFH
           .long     0B0B0B0B0H, 0B1B1B1B1H, 0B2B2B2B2H, 0B3B3B3B3H
           .long     0B4B4B4B4H, 0B5B5B5B5H, 0B6B6B6B6H, 0B7B7B7B7H
           .long     0B8B8B8B8H, 0B9B9B9B9H, 0BABABABAH, 0BBBBBBBBH
           .long     0BCBCBCBCH, 0BDBDBDBDH, 0BEBEBEBEH, 0BFBFBFBFH
           .long     0C0C0C0C0H, 0C1C1C1C1H, 0C2C2C2C2H, 0C3C3C3C3H
           .long     0C4C4C4C4H, 0C5C5C5C5H, 0C6C6C6C6H, 0C7C7C7C7H
           .long     0C8C8C8C8H, 0C9C9C9C9H, 0CACACACAH, 0CBCBCBCBH
           .long     0CCCCCCCCH, 0CDCDCDCDH, 0CECECECEH, 0CFCFCFCFH
           .long     0D0D0D0D0H, 0D1D1D1D1H, 0D2D2D2D2H, 0D3D3D3D3H
           .long     0D4D4D4D4H, 0D5D5D5D5H, 0D6D6D6D6H, 0D7D7D7D7H
           .long     0D8D8D8D8H, 0D9D9D9D9H, 0DADADADAH, 0DBDBDBDBH
           .long     0DCDCDCDCH, 0DDDDDDDDH, 0DEDEDEDEH, 0DFDFDFDFH
           .long     0E0E0E0E0H, 0E1E1E1E1H, 0E2E2E2E2H, 0E3E3E3E3H
           .long     0E4E4E4E4H, 0E5E5E5E5H, 0E6E6E6E6H, 0E7E7E7E7H
           .long     0E8E8E8E8H, 0E9E9E9E9H, 0EAEAEAEAH, 0EBEBEBEBH
           .long     0ECECECECH, 0EDEDEDEDH, 0EEEEEEEEH, 0EFEFEFEFH
           .long     0F0F0F0F0H, 0F1F1F1F1H, 0F2F2F2F2H, 0F3F3F3F3H
           .long     0F4F4F4F4H, 0F5F5F5F5H, 0F6F6F6F6H, 0F7F7F7F7H
           .long     0F8F8F8F8H, 0F9F9F9F9H, 0FAFAFAFAH, 0FBFBFBFBH
           .long     0FCFCFCFCH, 0FDFDFDFDH, 0FEFEFEFEH, 0FFFFFFFFH

           .text

           .globl    _set_ppop
           .globl    _transp_on
           .globl    _transp_off

;
; First, we need code that does color expansion blits from a 
; monochrome data buffer in TI memory. All "1" bits are expanded
; to color1, all "0" bits are expanded to color0. Note that the
; chip transparency information is set external to this routine,
; so that it's conceivable that all bits of one type could actually
; end up as transparent after the expansion. Note that TI expects
; the most significant pixel in a word to be Bit 0. This means all
; PC binary data must be "swizzled" prior to use here.
;
; All registers are preserved.
;
; Input Parameters:
;
;    A4 = Address of Binary data
;    A5 = Pitch of Binary data
;    A6 = Source position of data within Binary block (Y:X)
;    A7 = Dimensions (Y:X) of Binary blit data, in bits
;    A8 = Destination on display (Y:X)
;
;
; The following are used as indices into the parameter block passed by
; the PC for the mask blits.
;
;
bytePitch  .set      0
srcX       .set      bytePitch+16
srcY       .set      srcX+16
dstX       .set      srcY+16
dstY       .set      dstX+16
dimX       .set      dstY+16
dimY       .set      dimX+16
oneColor   .set      dimY+16
zeroColor  .set      oneColor+16
;
;
; Algorithm:
;
;      First, we need to calculate the source and destination
;      linear addresses. Then it's just a matter of using the
;      TI PIXBLT B,L function.
;
;      SrcAddr = Base + Y * SrcPitch + X
;      DstAddr = DisplayBase + Y * DisplayPitch + X * BitsPerPixel
;
;      Note that these calculations could be done by the 34020 directly
;      if one was certain that only the 34020 would be used with the
;      code.
;
BlitExpand:
           mmtm      SP,A0,A1,A2,A4,A5,A6,A7,A8,A9
           mmtm      SP,B0,B1,B2,B7

           setf      16,0,0
           move      A6,A0             ; Get Binary Y:X
           srl       16,A0             ; Strip lower 16 bits to get Y coor
           move      A5,A1
           mpyu      A0,A1             ; Multiply by pitch
           add       A1,A4             ; Add to binary address base
           sll       16,A6
           srl       16,A6             ; Strip upper 16 bits to get X coor
           add       A6,A4             ; Binary start address
           move      A4,SADDR          ; Set TI SADDR register
           move      A5,SPTCH          ; Set source pitch

           move      OFFSET,A0         ; Get base address of display.
           move      DPTCH,A1          ; Get display pitch
           move      A8,A2
           srl       16,A2             ; Get destination Y value
           mpyu      A2,A1             ; Get Y * DisplayPitch
           add       A1,A0
           sll       16,A8
           srl       13,A8             ; Strip out X portion and * 8 (bpp)
           add       A8,A0
           move      A0,DADDR          ; Store destination address

           move      A7,DYDX
           pixblt    B,L               ; Do the pixel blit

           mmfm      SP,B0,B1,B2,B7
           mmfm      SP,A0,A1,A2,A4,A5,A6,A7,A8,A9
           rets


;
; SetTIBuffAddr(buffAddr, buffSize);
;
; This routine is used to set the address and size of the
; on-board transfer buffer allocated in TI memory from the
; PC. Called from PC side as a CP_CMD type function.
;
;
_SetTIBuffAddr:
           move      *-STK,A8,1
           move      A8,@buffAddr,1    ; Get pointer to data area.
           move      *-STK,A8,1
           move      A8,@buffSize,1    ; Get size of data area.
           rets      2


;
; Mask1Blit(parmPtr)
;
; parmPtr + 0  :     Byte pitch of binary data
; parmPtr + 16 :     X start into binary data
; parmPtr + 32 :     Y start into binary data
; parmPtr + 48 :     X destination on screen
; parmPtr + 64 :     Y destination on screen
; parmPtr + 80 :     Width in pixels/bits of source data
; parmPtr + 96 :     Height in lines of source data
; parmPtr + 112:     Color for color 1
;
;
; Performs the AA386 Mask1Blit operation, where binary source data
; from our communications buffer is expanded with 0s transparent and
; 1s set to the passed color.
;
;
_Mask1Blit:
           mmtm      SP,A0,A1,A4,A5,A6,A7,A9
           mmtm      SP,B8,B9

           move      *-STK,A1,1        ; Pop parameter pointer

           move      STK,-*SP,1
           clr       A0
           move      A0,*STK+,1        ; Put 0 on stack - SRC=DST
           calla     _set_ppop         ; Set raster op
           calla     _transp_on        ; Enable ROP=0 transparency
           setf      16,0,0            ; TI calls screw up field 0

           move      @buffAddr,A4,1    ; Get buffer address
           move      *A1(bytePitch),A5 ; Byte pitch of data
           sll       3,A5              ;  is now bit pitch.
           move      *A1(srcX),A6,1    ; Load Src Y into high word, X in low
           move      *A1(dstX),A8,1    ; Load Dst Y into high word, X in low
           move      *A1(dimX),A7,1    ; Load Dim Y into high word, X in low

           move      A0,B8             ; Clear color 0
           move      *A1(oneColor),A0  ; Get color 1
           move      A0,B9
           andi      0FFH,B9           ; Strip all but lower 8 bits
           sll       5,B9              ; Make the color into a 32-bit index
           addi      colorTable8,B9    ; Get pointer to color
           move      *B9,B9,1          ; Expand the color to full 32 bits

           calla     BlitExpand        ; Do the operation.
           
           calla     _transp_off       ; Disable ROP=0 transparency

           mmfm      SP,B8,B9
           mmfm      SP,A0,A1,A4,A5,A6,A7,A9
           rets      2                 ; We're done!
;
;
; Mask2Blit(parmPtr)
;
; parmPtr + 0  :     Byte pitch of binary data
; parmPtr + 16 :     X start into binary data
; parmPtr + 32 :     Y start into binary data
; parmPtr + 48 :     X destination on screen
; parmPtr + 64 :     Y destination on screen
; parmPtr + 80 :     Width in pixels/bits of source data
; parmPtr + 96 :     Height in lines of source data
; parmPtr + 112:     Color for color 1
; parmPtr + 128:     Color for Color 0
;
;
; Performs the AA386 Mask1Blit operation, where binary source data
; from our communications buffer is expanded with 0s in color 0 and
; 1s set to color 1.
;
;
_Mask2Blit:
           mmtm      SP,A0,A1,A4,A5,A6,A7,A9
           mmtm      SP,B8,B9

           move      *-STK,A1,1        ; Pop parameter pointer

           move      STK,-*SP,1
           clr       A0
           move      A0,*STK+,1        ; Put 0 on stack - SRC=DST
           calla     _set_ppop         ; Set raster op
           setf      16,0,0            ; TI calls screw up field 0

           move      @buffAddr,A4,1    ; Get buffer address
           move      *A1(bytePitch),A5 ; Byte pitch of data
           sll       3,A5              ;  is now bit pitch.
           move      *A1(srcX),A6,1    ; Load Src Y into high word, X in low
           move      *A1(dstX),A8,1    ; Load Dst Y into high word, X in low
           move      *A1(dimX),A7,1    ; Load Dim Y into high word, X in low

           move      *A1(oneColor),A0  ; Get color 1
           move      A0,B9
           andi      0FFH,B9           ; Strip all but lower 8 bits
           sll       5,B9              ; Make the color into a 32-bit index
           addi      colorTable8,B9    ; Get pointer to color
           move      *B9,B9,1          ; Expand the color to full 32 bits

           move      *A1(zeroColor),A0 ; Get color 0
           move      A0,B8
           andi      0FFH,B8           ; Strip all but lower 8 bits
           sll       5,B8              ; Make the color into a 32-bit index
           addi      colorTable8,B8    ; Get pointer to color
           move      *B8,B8,1          ; Expand the color to full 32 bits

           calla     BlitExpand        ; Do the operation.
           
           mmfm      SP,B8,B9
           mmfm      SP,A0,A1,A4,A5,A6,A7,A9
           rets      2                 ; We're done!

;
;
; _UnSS2_Rect(parmPtr)
;
; parmPtr + 0  :     X start (on screen)
; parmPtr + 16 :     Y start
; parmPtr + 32 :     Width
; parmPtr + 48 :     Height
; parmPtr + 64 :     Line Count
;
; The buffer contains the following:
;
; buffBase + 80 :     Op Word - See documentation
;
;
; Modern AA386-style delta decompression code.
;
;

unXStart   .set      0
unYStart   .set      unXStart+16
unWidth    .set      unYStart+16
unHeight   .set      unWidth+16
unLineCnt  .set      unHeight+16

;
; Register Usage:
;
; A0 = Parm Ptr
; A1 = Line Counter
; A2 = Display Pitch
; A4 = Current display data linear address (start of a line)
; A5 = Current data stream pointer
; A6 = Display linear address (within a line)
; A7 = Current Op Word
;

_UnSS2_Rect:
           mmtm      SP,A0,A1,A2,A3,A4,A5,A6,A7,A9

           move      *-STK,A0,1        ; Pop parameter pointer
           setf      16,0,0

           move      *A0(unLineCnt),A1 ; Get line counter
           move      *A0(unYStart),A3  ; Get Y Start

           move      DPTCH,A2          ; Get display pitch
           move      OFFSET,A4         ; Get display base address
           mpyu      A2,A3             ; Y * DPTCH
           add       A3,A4             ; Start position of line YStart

           move      @buffAddr,A5,1    ; Point to first op word

SS2_LineLoop:
           move      A1,A1
           jrz       SS2_Done          ; If no lines left, we're done!

           setf      16,1,0
           move      *A5+,A7           ; Get Op Word
           setf      16,0,0
           btst      15,A7             ; Test Bit 15
           jrz       SS2_LoadX

           btst      14,A7             ; Test Bit 14
           jrz       SS2_LowByteOp
                                       ; Both Bits 15 and 14 were set
           neg       A7                ; Negate Op Word
           mpyu      A2,A7             ; Get bit count to next line
           add       A7,A4             ; Update screen address
           jruc      SS2_LineLoop

SS2_LowByteOp:
           move      *A0(unXStart),A3  ; Get X
           move      *A0(unWidth),A8   ; Get width
           add       A8,A3             ; Add the two
           dec       A3                ; (X + Width - 1)
           sll       3,A3              ; Convert X to bit position
           add       A4,A3             ; Actual screen position
           movb      A7,*A3            ; Set the byte
           jruc      SS2_LineLoop

SS2_LoadX:
           move      *A0(unXStart),A3  ; Get X start position
           move      A4,A6             ; Get local screen pointer
           sll       3,A3
           add       A3,A6             ; Start at X position

SS2_DeltaLoop:
           move      A7,A7
           jrnz      SS2_SkipByte      ; Is it end of line?
           add       A2,A4             ; Skip to next line
           dec       A1                ; Decrement line counter
           jruc      SS2_LineLoop      ; Go to next line...

SS2_SkipByte:
           setf      8,0,0             ; Now reading 8 bits at a time
           move      *A5+,A8           ; Get the skip byte amount
           sll       3,A8
           add       A8,A6             ; Update current screen ptr.

           setf      8,1,0
           move      *A5+,A8           ; Get Op Byte (sign extend)
           setf      16,0,0
           jrnn      SS2_Copy          ; Check if replicate or copy

           move      *A5+,A9           ; Get replication word
           neg       A8                ; Get a positive count

SS2_DupWord:
           move      A9,*A6+
           dsjs      A8,SS2_DupWord    ; Duplicate the word, update pointer

           dec       A7                ; Decrement the Op Word
           jruc      SS2_DeltaLoop

SS2_Copy:
           move      *A5+,*A6+         ; Copy a word
           dsjs      A8,SS2_Copy       ; Op Byte times

           dec       A7                ; Do next packet
           jruc      SS2_DeltaLoop

SS2_Done:
           mmfm      SP,A0,A1,A2,A3,A4,A5,A6,A7,A9
           rets      2                 ; We're done!

                      
;
;
; _UnLCC_Rect(parmPtr)
;
; parmPtr + 0  :     X start (on screen)
; parmPtr + 16 :     Y start
; parmPtr + 32 :     Width
; parmPtr + 48 :     Height
; parmPtr + 64 :     Line Skip Count
; parmPtr + 80 :     Line Use Count
;
; Old fashioned AA-style delta decompression code.
;
;

lcXStart   .set      0
lcYStart   .set      lcXStart+16
lcWidth    .set      lcYStart+16
lcHeight   .set      lcWidth+16
lcLineSkip .set      lcHeight+16
lcLineCnt  .set      lcLineSkip+16

;
; Register Usage:
;
; A0 = Parm Ptr
; A1 = Line Counter
; A2 = Display Pitch
; A4 = Current display data linear address (start of a line)
; A5 = Current data stream pointer
; A6 = Display linear address (within a line)
; A7 = Current Op Word
;

_UnLCC_Rect:
           mmtm      SP,A0,A1,A2,A3,A4,A5,A6,A7,A9,A10

           move      *-STK,A0,1        ; Pop parameter pointer
           setf      16,0,0

           move      *A0(lcLineCnt),A1 ; Get line counter
           move      *A0(lcLineSkip),A3 ;Get line skip amount
           move      *A0(lcYStart),A2  ; Get Y Top
           add       A2,A3             ; Get start of decompression

           move      DPTCH,A2          ; Get display pitch
           move      OFFSET,A4         ; Get display base address
           mpyu      A2,A3             ; LineSkip * DPTCH
           add       A3,A4             ; Start position of line YStart

           move      @buffAddr,A5,1    ; Point to first word (packet cnt)

LCC_LineLoop:
           move      A1,A1
           jrz       LCC_Done          ; If no lines left, we're done!

           setf      16,0,0
           move      *A0(lcXStart),A3  ; Get X start position
           move      A4,A6             ; Get local screen pointer
           sll       3,A3
           add       A3,A6             ; Start at X position

           setf      8,0,0
           move      *A5+,A7           ; Get packet count

LCC_InLine:
           move      A7,A7             ; Any packets left on this line?
           jrnz      LCC_NextPkt       ; If no packets left...
           add       A2,A4             ; Skip to next line
           dec       A1                ; Decrement line counter
           jruc      LCC_LineLoop
LCC_NextPkt:
           move      *A5+,A8           ; Get Skip count
           sll       3,A8
           add       A8,A6             ; Skip X * 8 bits
           setf      8,1,0
           move      *A5+,A8           ; Get signed byte size count
           setf      8,0,0
           jrnn      LCC_Copy

           move      *A5+,A9           ; Get replication byte
           neg       A8                ; Get a positive count

LCC_DupByte:
           move      A9,*A6+
           dsjs      A8,LCC_DupByte    ; Duplicate the word, update pointer

           dec       A7                ; Decrement the packet count
           jruc      LCC_InLine

LCC_Copy:
           move      *A5+,*A6+         ; Copy a byte
           dsjs      A8,LCC_Copy       ; Byte size times

           dec       A7                ; Do next packet
           jruc      LCC_InLine
          
LCC_Done:
           mmfm      SP,A0,A1,A2,A3,A4,A5,A6,A7,A9,A10
           rets      2                 ; We're done!
           
           
           .end

