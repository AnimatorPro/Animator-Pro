;-------------------------------------------------------------------------;
;                                    TIGA                                 ;
;          Copyright (C) 1988-1990  Texas Instruments Incorporated.       ;
;                            All Rights Reserved                          ;
;-------------------------------------------------------------------------;
;   TIGA 2-D Graphics Library include file                                ;
;-------------------------------------------------------------------------;
;                                                                         ;      
;   file            gspextnd.inc (GSP Assembly version of extend.h)       ;
;                                                                         ;
;   description     This file contains references to functions contained  ;
;                   in the TIGA 2-D Graphics Library.  It should be       ;
;                   included in any application, written in TMS340        ;
;                   assembler, which invokes a graphics library function. ;
;                                                                         ;
;-------------------------------------------------------------------------;
                                                                        
;-------------------------------------------------------------------------;
;  Graphics Library function definitions                                  ;
;-------------------------------------------------------------------------;
    .globl  _bitblt
    .globl  _decode_rect
    .globl  _delete_font
    .globl  _draw_line
    .globl  _draw_oval
    .globl  _draw_ovalarc
    .globl  _draw_piearc
    .globl  _draw_point
    .globl  _draw_polyline
    .globl  _draw_polyline_a
    .globl  _draw_rect
    .globl  _encode_rect
    .globl  _fill_convex
    .globl  _fill_convex_a
    .globl  _fill_oval
    .globl  _fill_piearc
    .globl  _fill_polygon
    .globl  _fill_polygon_a
    .globl  _fill_rect
    .globl  _frame_oval
    .globl  _frame_rect
    .globl  _get_env
    .globl  _get_pixel
    .globl  _get_textattr
    .globl  _install_font
    .globl  _in_font
    .globl  _move_pixel
    .globl  _patnfill_convex
    .globl  _patnfill_convex_a
    .globl  _patnfill_oval
    .globl  _patnfill_piearc
    .globl  _patnfill_polygon
    .globl  _patnfill_polygon_a
    .globl  _patnfill_rect
    .globl  _patnframe_oval
    .globl  _patnframe_rect
    .globl  _patnpen_line
    .globl  _patnpen_ovalarc
    .globl  _patnpen_piearc
    .globl  _patnpen_point
    .globl  _patnpen_polyline
    .globl  _patnpen_polyline_a
    .globl  _pen_line
    .globl  _pen_ovalarc
    .globl  _pen_piearc
    .globl  _pen_point
    .globl  _pen_polyline
    .globl  _pen_polyline_a
    .globl  _put_pixel
    .globl  _seed_fill
    .globl  _seed_patnfill
    .globl  _select_font
    .globl  _set_draw_origin
    .globl  _set_dstbm
    .globl  _set_patn
    .globl  _set_pensize
    .globl  _set_srcbm
    .globl  _set_textattr
    .globl  _styled_line
    .globl  _styled_oval
    .globl  _styled_ovalarc
    .globl  _styled_piearc
    .globl  _swap_bm
    .globl  _text_width
    .globl  _zoom_rect
