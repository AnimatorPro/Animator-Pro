;-------------------------------------------------------------------------;
;                                    TIGA                                 ;
;          Copyright (C) 1988-1990  Texas Instruments Incorporated.       ;
;                            All Rights Reserved                          ;
;-------------------------------------------------------------------------;
;  TIGA - Header file                                                     ;
;-------------------------------------------------------------------------;
;                                                                         ;
;  file            gspreg.inc                                             ;
;                                                                         ;
;  description     This is a GSP assembly language header file that       ;
;                  defines symbolic names for A- and B-file registers,    ;
;                  and for I/O registers in the 34010 and 34020.  It      ;
;                  can be included in the GSP assembly source code for    ;
;                  a GSP-resident code module.                            ;
;                                                                         ;
;-------------------------------------------------------------------------;

;-------------------------------------------------------------------------;
; Names of special A- and B-file registers                                ;
;-------------------------------------------------------------------------;
FP      .set      A13                 ;register A13 - frame pointer, C prog stack
STK     .set      A14                 ;register A14 - C program stack pointer
SADDR   .set      B0                  ;register B0 - source address
SPTCH   .set      B1                  ;register B1 - source pitch
DADDR   .set      B2                  ;register B2 - destination address
DPTCH   .set      B3                  ;register B3 - destination pitch
OFFSET  .set      B4                  ;register B4 - XY offset
WSTART  .set      B5                  ;register B5 - window start
WEND    .set      B6                  ;register B6 - window end
DYDX    .set      B7                  ;register B7 - delta X/delta Y
COLOR0  .set      B8                  ;register B8 - color 0
COLOR1  .set      B9                  ;register B9 - color 1
MADDR   .set      B10                 ;register B10 - 34020 mask address
MPTCH   .set      B11                 ;register B11 - 34020 mask pitch
PATTRN  .set      B13                 ;register B13 - 34020 pattern

;-------------------------------------------------------------------------;
; Define base location of TMS340s on-chip I/O register file.              ;
;-------------------------------------------------------------------------;
IO_BASE .set      0C0000000h          ;address of 1st I/O register

;-------------------------------------------------------------------------;
; Define names of I/O registers common to 34010 and 34020.                ;
;-------------------------------------------------------------------------;
DPYCTL  .set      0C0000080h          ;display control
DPYSTRT .set      0C0000090h          ;display start address
DPYINT  .set      0C00000A0h          ;display interrupt vertical count
CONTROL .set      0C00000B0h          ;control
HSTDATA .set      0C00000C0h          ;host interface data
HSTADRL .set      0C00000D0h          ;host interface address low
HSTADRH .set      0C00000E0h          ;host interface address high
HSTCTLL .set      0C00000F0h          ;host interface control low
HSTCTLH .set      0C0000100h          ;host interface control high
INTENB  .set      0C0000110h          ;interrupt enable
INTPEND .set      0C0000120h          ;interrupt pending
CONVSP  .set      0C0000130h          ;conversion factor, source pitch
CONVDP  .set      0C0000140h          ;conversion factor, dest'n pitch
PSIZE   .set      0C0000150h          ;pixel size
PMASK   .set      0C0000160h          ;plane mask
DPYTAP  .set      0C00001B0h          ;display tap point
DPYADR  .set      0C00001E0h          ;display address
REFCNT  .set      0C00001F0h          ;DRAM refresh count

;-------------------------------------------------------------------------;
; Define names of display interrupt control bits common to 34010/20.      ;
;-------------------------------------------------------------------------;
DIE     .set      (INTENB+10)         ;display interrupt enable bit
DIP     .set      (INTPEND+10)        ;display interrupt pending bit

;-------------------------------------------------------------------------;
; Masks for I/O register fields common to 34010/20                        ;
;-------------------------------------------------------------------------;
TRN_MASK        .set    00020h  ; Mask for T field in CONTROL
WDW_MASK        .set    000C0h  ; Mask for W field in CONTROL
PBH_MASK        .set    00100h  ; Mask for Pixblt Horiz. Dir. in CONTROL
PBV_MASK        .set    00200h  ; Mask for Pixblt Vert.  Dir. in CONTROL
PPOP_MASK       .set    07C00h  ; Mask for PPOP in CONTROL
SRT_MASK        .set    00800h  ; Mask for SRT bit in DPYCTL
SRE_MASK        .set    01000h  ; Mask for SRT bit in DPYCTL
ENV_MASK        .set    08000h  ; Mask for SRT bit in DPYCTL
DI_MASK         .set    00400h  ; Mask for DI bit in INTPEND/INTENA
                        
;-------------------------------------------------------------------------;
; Masks for I/O register fields specific to 34020                         ;
;-------------------------------------------------------------------------;
TM_MASK         .set    00007h  ; Mask for TM field in CONTROL

;-------------------------------------------------------------------------;
; 34010-specific I/O register names.                                      ;
;-------------------------------------------------------------------------;
HESYNC10        .set    0C0000000h      ;horizontal end sync
HEBLNK10        .set    0C0000010h      ;horizontal end blank
HSBLNK10        .set    0C0000020h      ;horizontal start blank
HTOTAL10        .set    0C0000030h      ;horizontal total
VESYNC10        .set    0C0000040h      ;vertical end sync
VEBLNK10        .set    0C0000050h      ;vertical end blank
VSBLNK10        .set    0C0000060h      ;vertical start blank
VTOTAL10        .set    0C0000070h      ;vertical total
HCOUNT10        .set    0C00001C0h      ;horizontal count
VCOUNT10        .set    0C00001D0h      ;vertical count

;-------------------------------------------------------------------------;
; 34020-specific I/O register names.                                      ;
;-------------------------------------------------------------------------;
VESYNC20        .set    0C0000000h      ;vertical end sync
HESYNC20        .set    0C0000010h      ;horizontal end sync
VEBLNK20        .set    0C0000020h      ;vertical end blank
HEBLNK20        .set    0C0000030h      ;horizontal end blank
VSBLNK20        .set    0C0000040h      ;vertical start blank
HSBLNK20        .set    0C0000050h      ;horizontal start blank
VTOTAL20        .set    0C0000060h      ;vertical total
HTOTAL20        .set    0C0000070h      ;horizontal total
CONVMP20        .set    0C0000180h      ;conversion factor, mask pitch
CONTROL20       .set    0C0000190h      ;control
CONFIG20        .set    0C00001A0h      ;configuration
VCOUNT20        .set    0C00001C0h      ;vertical count
HCOUNT20        .set    0C00001D0h      ;horizontal count
DPYST20         .set    0C0000200h      ;display start (32-bit)
DPYNX20         .set    0C0000220h      ;display next (32-bit)
DINC20          .set    0C0000240h      ;display increment (32-bit)
HESERR20        .set    0C0000270h      ;horizontal end serration
SCOUNT20        .set    0C00002C0h      ;shift clock count
BSFLTST20       .set    0C00002D0h      ;bus fault status
DPYMASK20       .set    0C00002E0h      ;display mask
SETVCNT20       .set    0C0000300h      ;set vertical count
SETHCNT20       .set    0C0000310h      ;set horizontal count
BSFLTD20        .set    0C0000320h      ;bus fault data

;-------------------------------------------------------------------------;
; GSP part number fields in code produced by REV instruction              ;
;-------------------------------------------------------------------------;
REV_34010   .set  3         ; REV bit number if 34010
REV_34020   .set  4         ; REV bit number if 34020
REV_MASK    .set  0FFF8h    ; GSP part number mask
