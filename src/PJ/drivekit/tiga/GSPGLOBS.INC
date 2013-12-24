;-------------------------------------------------------------------------;
;                                    TIGA                                 ;
;          Copyright (C) 1988-1990  Texas Instruments Incorporated.       ;
;                            All Rights Reserved                          ;
;-------------------------------------------------------------------------;
;   TIGA - Include file                                                   ;
;-------------------------------------------------------------------------;
;                                                                         ;      
;  file            gspglobs.inc (GSP Assembly version of gspglobs.h)      ;
;                                                                         ;
;  description     This file contains references to global variables      ;
;                  used in the Graphics Manager portion of TIGA.          ;
;                  It is needed for the gsp side of an application,       ;
;                  written in GSP-Assembler, which accesses TIGA global   ;
;                  variables.                                             ;
;                                                                         ;
;-------------------------------------------------------------------------;
    .globl  _config
    .globl  _DEFAULT_PALET
    .globl  _DefaultCursor
    .globl  _env
    .globl  _envtext
    .globl  _envcurs
    .globl  _high_water_mark
    .globl  _modeinfo
    .globl  _Module
    .globl  _monitorinfo
    .globl  _offscreen
    .globl  _page
    .globl  _palet
    .globl  _pattern
    .globl  _setup
    .globl  _sin_tbl
    .globl  _stack_size
    .globl  _sysfont
    .globl  _sys16, _sys24
    .globl  _sys_memory
    .globl  _pHCOUNT, _pHEBLNK, _pHESYNC, _pHSBLNK, _pHTOTAL
    .globl  _pVCOUNT, _pVEBLNK, _pVESYNC, _pVSBLNK, _pVTOTAL
