;-------------------------------------------------------------------------;
;                                    TIGA                                 ;
;          Copyright (C) 1988-1990  Texas Instruments Incorporated.       ;
;                            All Rights Reserved                          ;
;-------------------------------------------------------------------------;
;   TIGA - Include file                                                   ;
;-------------------------------------------------------------------------;
;                                                                         ;      
;  file            gsptiga.inc (GSP Assembler version of tiga.h)          ;
;                                                                         ;
;  description     This file contains references to TIGA core functions   ;
;                  It is needed for the gsp side of an application,       ;
;                  written in GSP-Assembler, which invokes TIGA core      ;
;                  functions.                                             ;
;                                                                         ;
;-------------------------------------------------------------------------;

;-------------------------------------------------------------------------;
;  Reference Core Primitives                                              ;
;-------------------------------------------------------------------------;
    .globl  _clear_frame_buffer
    .globl  _clear_page
    .globl  _clear_screen
    .globl  _cop2gsp
    .globl  _cpw
    .globl  _cvxyl
    .globl  _flush_extended
    .globl  _flush_module
    .globl  _function_implemented
    .globl  _get_colors
    .globl  _get_config
    .globl  _get_curs_state
    .globl  _get_curs_xy
    .globl  _get_fontinfo
    .globl  _get_isr_priorities
    .globl  _get_nearest_color
    .globl  _get_offscreen_memory
    .globl  _get_palet
    .globl  _get_palet_entry
    .globl  _get_pmask
    .globl  _get_ppop
    .globl  _get_text_xy
    .globl  _get_transp
    .globl  _get_vector
    .globl  _get_windowing
    .globl  _get_wksp
    .globl  _gsp2cop
    .globl  _gsp2gsp
    .globl  _gsph_alloc
    .globl  _gsph_calloc
    .globl  _gsph_compact
    .globl  _gsph_deref
    .globl  _gsph_falloc
    .globl  _gsph_fcalloc
    .globl  _gsph_findhandle
    .globl  _gsph_findmem
    .globl  _gsph_free
    .globl  _gsph_init
    .globl  _gsph_maxheap
    .globl  _gsph_memtype
    .globl  _gsph_realloc
    .globl  _gsph_totalfree
    .globl  _gsp_calloc
    .globl  _gsp_free
    .globl  _gsp_malloc
    .globl  _gsp_maxheap
    .globl  _gsp_minit
    .globl  _gsp_realloc
    .globl  _init_palet
    .globl  _init_text
    .globl  _lmo
    .globl  _page_busy
    .globl  _page_flip
    .globl  _peek_breg
    .globl  _poke_breg
    .globl  _rmo
    .globl  _set_bcolor
    .globl  _set_clip_rect
    .globl  _set_colors
    .globl  _set_config
    .globl  _set_curs_shape
    .globl  _set_curs_state
    .globl  _set_cursattr
    .globl  _set_fcolor
    .globl  _set_interrupt
    .globl  _set_module_state
    .globl  _set_palet
    .globl  _set_palet_entry
    .globl  _set_pmask
    .globl  _set_ppop
    .globl  _set_text_xy
    .globl  _set_transp
    .globl  _set_vector
    .globl  _set_windowing
    .globl  _set_wksp
    .globl  _sym_flush
    .globl  _text_out
    .globl  _text_outp
    .globl  _transp_off
    .globl  _transp_on
    .globl  _wait_scan

;-------------------------------------------------------------------------;
; Colors of the default TIGA palet                                        ;
;-------------------------------------------------------------------------;
BLACK           .set    0
BLUE            .set    1
GREEN           .set    2
CYAN            .set    3
RED             .set    4
MAGENTA         .set    5
BROWN           .set    6
LIGHT_GRAY      .set    7
DARK_GRAY       .set    8
LIGHT_BLUE      .set    9
LIGHT_GREEN     .set    10
LIGHT_CYAN      .set    11
LIGHT_RED       .set    12
LIGHT_MAGENTA   .set    13
YELLOW          .set    14
WHITE           .set    15

;----------------------------------;
; Memory System Equates            ;
;----------------------------------;
BLK_DELETABLE   Equ     01h   ; Block can be deleted
BLK_LOCKED      Equ     02h   ; Block is locked
BLK_FUNCMOVE    Equ     04h   ; Call block function on move
BLK_FUNCDELETE  Equ     08h   ; Call block function on purge
BLK_SECURED     Equ     10h   ; Block is allocated
BLK_INUSE       Equ     20h   ; Block was allocated by system

;----------------------------------;
; Extended primitives module id    ;
; - parameter to sym_flush         ;
;----------------------------------;
GRAPHICS_LIB_ID Equ     03C00h
