/*-----------------------------------------------------------------------*/
/*                                   TIGA                                */
/*         Copyright (C) 1988-1990  Texas Instruments Incorporated.      */
/*                           All Rights Reserved                         */
/*-----------------------------------------------------------------------*/
/*  TIGA 2-D Graphics Library include file for PharLap DOS Extender      */
/*-----------------------------------------------------------------------*/
/*                                                                       */
/*  file            extend.pl                                            */
/*                                                                       */
/*  description     This file contains references to functions contained */
/*                  in the TIGA 2-D Graphics Library.  It must be        */
/*                  included in any application written in High-C or     */
/*                  NDP-C which invokes a TIGA graphics library          */
/*                  function.                                            */
/*                                                                       */
/*                  Note that the tiga.hch (for High-C) or tiga.ndp      */
/*                  (for NDP-C) include file must be included before     */
/*                  the extend.pl file.                                  */
/*                                                                       */
/*-----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
/*  Extended function Command Numbers                                   */
/*----------------------------------------------------------------------*/
#define SET_DSTBM           EXT_CP( 0)
#define SET_SRCBM           EXT_CP( 1)
#define STYLED_LINE         EXT_CP( 2)
#define SWAP_BM             EXT_CP( 3)
#define DRAW_POLYLINE_A     EXT_CP( 4)
#define FILL_CONVEX_A       EXT_CP( 5)
#define FILL_POLYGON_A      EXT_CP( 6)
#define PATNFILL_CONVEX_A   EXT_CP( 7)
#define PATNFILL_POLYGON_A  EXT_CP( 8)
#define PATNPEN_POLYLINE_A  EXT_CP( 9)
#define PEN_POLYLINE_A      EXT_CP(10)
#define BITBLT              EXT_DM(11)
#define DRAW_LINE           EXT_DM(12)
#define DRAW_OVAL           EXT_DM(13)
#define DRAW_OVALARC        EXT_DM(14)
#define DRAW_PIEARC         EXT_DM(15)
#define DRAW_POINT          EXT_DM(16)
#define DRAW_POLYLINE       EXT_DM(17)
#define DRAW_RECT           EXT_DM(18)
#define FILL_CONVEX         EXT_DM(19)
#define FILL_OVAL           EXT_DM(20)
#define FILL_PIEARC         EXT_DM(21)
#define FILL_POLYGON        EXT_DM(22)
#define FILL_RECT           EXT_DM(23)
#define FRAME_OVAL          EXT_DM(24)
#define FRAME_RECT          EXT_DM(25)
#define GET_PIXEL           EXT_DM(26)
#define PATNFILL_CONVEX     EXT_DM(27)
#define PATNFILL_OVAL       EXT_DM(28)
#define PATNFILL_PIEARC     EXT_DM(29)
#define PATNFILL_POLYGON    EXT_DM(30)
#define PATNFILL_RECT       EXT_DM(31)
#define PATNFRAME_OVAL      EXT_DM(32)
#define PATNFRAME_RECT      EXT_DM(33)
#define PATNPEN_LINE        EXT_DM(34)
#define PATNPEN_OVALARC     EXT_DM(35)
#define PATNPEN_PIEARC      EXT_DM(36)
#define PATNPEN_POINT       EXT_DM(37)
#define PATNPEN_POLYLINE    EXT_DM(38)
#define PEN_LINE            EXT_DM(39)
#define PEN_OVALARC         EXT_DM(40)
#define PEN_PIEARC          EXT_DM(41)
#define PEN_POINT           EXT_DM(42)
#define PEN_POLYLINE        EXT_DM(43)
#define SEED_FILL           EXT_DM(44)
#define SEED_PATNFILL       EXT_DM(45)
#define SET_DRAW_ORIGIN     EXT_DM(46)
#define SET_PENSIZE         EXT_DM(47)
#define ZOOM_RECT           EXT_DM(48)
#define SET_PATN            EXT_DM(49)
#define INSTALL_FONT        EXT_CP(50)
#define SELECT_FONT         EXT_CP(51)
#define DELETE_FONT         EXT_CP(52)
#define SET_TEXTATTR        EXT_CP(53)
#define GET_TEXTATTR        EXT_CP(54)
#define TEXT_WIDTH          EXT_CP(55)
#define GET_ENV             EXT_CP(56)
#define DECODE_RECT         EXT_CP(57)
#define ENCODE_RECT         EXT_CP(58)
#define MOVE_PIXEL          EXT_DM(59)
#define PUT_PIXEL           EXT_DM(60)
#define STYLED_OVAL         EXT_DM(61)
#define STYLED_OVALARC      EXT_DM(62)
#define STYLED_PIEARC       EXT_DM(63)
#define IN_FONT             EXT_CP(64)
                                     
/*----------------------------------------------------------------------*/
/*  C-Packet function definitions                                       */
/*----------------------------------------------------------------------*/
#define set_dstbm(a,b,c,d,e)        \
        cp_cmd(SET_DSTBM,5,_DWORD(a),_WORD(b),_WORD(c),_WORD(d),_WORD(e))
#define set_srcbm(a,b,c,d,e)        \
        cp_cmd(SET_SRCBM,5,_DWORD(a),_WORD(b),_WORD(c),_WORD(d),_WORD(e))
#define styled_line(a,b,c,d,e,f)    \
        cp_cmd(STYLED_LINE,6,_SWORD(a),_SWORD(b),_SWORD(c),_SWORD(d),_DWORD(e),_WORD(f))
#define swap_bm()                   \
        cp_cmd(SWAP_BM,0)
#define draw_polyline_a(a,b)        \
        cp_cmd_a(DRAW_POLYLINE_A,2,_WORD(a),_WORD_PTR(2*(a),b))
#define fill_convex_a(a,b)          \
        cp_cmd_a(FILL_CONVEX_A,2,_WORD(a),_WORD_PTR(2*(a),b))
#define fill_polygon_a(a,b)         \
        cp_cmd_a(FILL_POLYGON_A,2,_WORD(a),_WORD_PTR(2*(a),b))
#define patnfill_convex_a(a,b)      \
        cp_cmd_a(PATNFILL_CONVEX_A,2,_WORD(a),_WORD_PTR(2*(a),b))
#define patnfill_polygon_a(a,b)     \
        cp_cmd_a(PATNFILL_POLYGON_A,2,_WORD(a),_WORD_PTR(2*(a),b))
#define patnpen_polyline_a(a,b)     \
        cp_cmd_a(PATNPEN_POLYLINE_A,2,_WORD(a),_WORD_PTR(2*(a),b))
#define pen_polyline_a(a,b)         \
        cp_cmd_a(PEN_POLYLINE_A,2,_WORD(a),_WORD_PTR(2*(a),b))
#define install_font(a)             \
        (short)cp_ret(INSTALL_FONT,1,_DWORD(a))
#define select_font(a)              \
        (short)cp_ret(SELECT_FONT,1,_WORD(a))
#define delete_font(a)              \
        (short)cp_ret(DELETE_FONT,1,_WORD(a))
#define set_textattr(a,b,c)         \
        (short)cp_ret(SET_TEXTATTR,3,_STRING(a),_WORD(b),_WORD_PTR(b,c))
#define get_textattr(a,b,c)         \
        (short)cp_alt(GET_TEXTATTR,3,_STRING(a),_WORD(b),_ALTWORD_PTR(b,c))
#define text_width(a)               \
        (short)cp_ret(TEXT_WIDTH,1,_STRING(a))
#define get_env(a)                  \
        (void)cp_alt(GET_ENV,1,_ALTBYTE_PTR(sizeof(ENVIRONMENT),a))
#define decode_rect(a,b,c)          \
        (short)cp_ret(DECODE_RECT,3,_WORD(a),_WORD(b),_DWORD(c))
#define encode_rect(a,b,c,d,e,f,g)  \
        cp_ret(ENCODE_RECT,7,_WORD(a),_WORD(b),_WORD(c),_WORD(d),   \
                             _DWORD(e),_DWORD(f),_WORD(g))
#define in_font(a,b)                \
        (short)cp_ret(IN_FONT,2,_WORD(a),_WORD(b))

/*----------------------------------------------------------------------*/
/*  Direct-Mode function defintions                                     */
/*----------------------------------------------------------------------*/
#define bitblt(a,b,c,d,e,f)         \
        dm_cmd(BITBLT,6,0,(short)(a),(short)(b),(short)(c),\
                             (short)(d),(short)(e),(short)(f))
#define draw_line(a,b,c,d)          \
        dm_cmd(DRAW_LINE,4,0,(short)(a),(short)(b),(short)(c),(short)(d))
#define draw_oval(a,b,c,d)          \
        dm_cmd(DRAW_OVAL,4,0,(short)(a),(short)(b),(short)(c),(short)(d))
#define draw_ovalarc(a,b,c,d,e,f)   \
        dm_cmd(DRAW_OVALARC,6,0,(short)(a),(short)(b),(short)(c), \
                             (short)(d),(short)(e),(short)(f))
#define draw_piearc(a,b,c,d,e,f)    \
        dm_cmd(DRAW_PIEARC,6,0,(short)(a),(short)(b),(short)(c), \
                             (short)(d),(short)(e),(short)(f))
#define draw_point(a,b)             \
        dm_cmd(DRAW_POINT,2,0,(short)(a),(short)(b))
#define draw_polyline(a,b)          \
        dm_psnd(DRAW_POLYLINE,(short)(4*(a)),(short *)(b))
#define draw_rect(a,b,c,d)          \
        dm_cmd(DRAW_RECT,4,0,(short)(a),(short)(b),(short)(c),(short)(d))
#define fill_convex(a,b)            \
        dm_psnd(FILL_CONVEX,(short)(4*(a)),(short *)b)
#define fill_oval(a,b,c,d)          \
        dm_cmd(FILL_OVAL,4,0,(short)(a),(short)(b),(short)(c),(short)(d))
#define fill_piearc(a,b,c,d,e,f)    \
        dm_cmd(FILL_PIEARC,6,0,(short)(a),(short)(b),(short)(c), \
                             (short)(d),(short)(e),(short)(f))
#define fill_polygon(a,b)           \
        dm_psnd(FILL_POLYGON,(short)(4*(a)),(short *)(b))
#define fill_rect(a,b,c,d)          \
        dm_cmd(FILL_RECT,4,0,(short)(a),(short)(b),(short)(c),(short)(d))
#define frame_oval(a,b,c,d,e,f)     \
        dm_cmd(FRAME_OVAL,6,0,(short)(a),(short)(b),(short)(c), \
                             (short)(d),(short)(e),(short)(f))
#define frame_rect(a,b,c,d,e,f)     \
        dm_cmd(FRAME_RECT,6,0,(short)(a),(short)(b),(short)(c), \
                             (short)(d),(short)(e),(short)(f))
#define get_pixel(a,b)              \
        dm_ret(GET_PIXEL,2,0,(short)(a),(short)(b))
#define patnfill_convex(a,b)        \
        dm_psnd(PATNFILL_CONVEX,4*(a),(short *)b)
#define patnfill_oval(a,b,c,d)      \
        dm_cmd(PATNFILL_OVAL,4,0,(short)(a),(short)(b),(short)(c),(short)(d))
#define patnfill_piearc(a,b,c,d,e,f)\
        dm_cmd(PATNFILL_PIEARC,6,0,(short)(a),(short)(b),(short)(c), \
                             (short)(d),(short)(e),(short)(f))
#define patnfill_polygon(a,b)       \
        dm_psnd(PATNFILL_POLYGON,(short)(4*(a)),(short *)(b))
#define patnfill_rect(a,b,c,d)      \
        dm_cmd(PATNFILL_RECT,4,0,(short)(a),(short)(b),(short)(c),(short)(d))
#define patnframe_oval(a,b,c,d,e,f) \
        dm_cmd(PATNFRAME_OVAL,6,0,(short)(a),(short)(b),(short)(c), \
                             (short)(d),(short)(e),(short)(f))
#define patnframe_rect(a,b,c,d,e,f) \
        dm_cmd(PATNFRAME_RECT,6,0,(short)(a),(short)(b),(short)(c), \
                             (short)(d),(short)(e),(short)(f))
#define patnpen_line(a,b,c,d)       \
        dm_cmd(PATNPEN_LINE,4,0,(short)(a),(short)(b),(short)(c),(short)(d))
#define patnpen_ovalarc(a,b,c,d,e,f)\
        dm_cmd(PATNPEN_OVALARC,6,0,(short)(a),(short)(b),(short)(c), \
                             (short)(d),(short)(e),(short)(f))
#define patnpen_piearc(a,b,c,d,e,f) \
        dm_cmd(PATNPEN_PIEARC,6,0,(short)(a),(short)(b),(short)(c), \
                             (short)(d),(short)(e),(short)(f))
#define patnpen_point(a,b)          \
        dm_cmd(PATNPEN_POINT,2,0,(short)(a),(short)(b))
#define patnpen_polyline(a,b)       \
        dm_psnd(PATNPEN_POLYLINE,(short)(4*(a)),(short *)(b))
#define pen_line(a,b,c,d)           \
        dm_cmd(PEN_LINE,4,0,(short)(a),(short)(b),(short)(c),(short)(d))
#define pen_ovalarc(a,b,c,d,e,f)    \
        dm_cmd(PEN_OVALARC,6,0,(short)(a),(short)(b),(short)(c), \
                             (short)(d),(short)(e),(short)(f))
#define pen_piearc(a,b,c,d,e,f)     \
        dm_cmd(PEN_PIEARC,6,0,(short)(a),(short)(b),(short)(c), \
                             (short)(d),(short)(e),(short)(f))
#define pen_point(a,b)              \
        dm_cmd(PEN_POINT,2,0,(short)(a),(short)(b))
#define pen_polyline(a,b)           \
        dm_psnd(PEN_POLYLINE,(short)(4*(a)),(short *)(b))
#define seed_fill(a,b,c,d)          \
        dm_cmd(SEED_FILL,5,0,(short)(a),(short)(b),(long)(c),(short)(d))
#define seed_patnfill(a,b,c,d)      \
        dm_cmd(SEED_PATNFILL,5,0,(short)(a),(short)(b),(long)(c),(short)(d))
#define set_draw_origin(a,b)        \
        dm_cmd(SET_DRAW_ORIGIN,2,0,(short)(a),(short)(b))
#define set_pensize(a,b)            \
        dm_cmd(SET_PENSIZE,2,0,(short)(a),(short)(b))
#define zoom_rect(a,b,c,d,e,f,g,h,i)\
        dm_cmd(ZOOM_RECT,10,256,(short)(a),(short)(b),(short)(c), \
                              (short)(d),(short)(e),(short)(f), \
                              (short)(g),(short)(h),(long)(i))
#define set_patn(a)                 \
        dm_psnd(SET_PATN,(short)(sizeof(PATTERN)),(char *)(a))
#define move_pixel(a,b,c,d)         \
        dm_cmd(MOVE_PIXEL,4,0,(short)(a),(short)(b),(short)(c),(short)(d))
#define put_pixel(a,b,c)            \
        dm_cmd(PUT_PIXEL,4,1,(unsigned long)(a),(short)(b),(short)(c))
#define styled_oval(a,b,c,d,e,f)    \
        dm_cmd(STYLED_OVAL,7,16,(short)(a),(short)(b),(short)(c),(short)(d), \
                             (unsigned long)(e),(short)(f))
#define styled_ovalarc(a,b,c,d,e,f,g,h) \
        dm_cmd(STYLED_OVALARC,9,64,(short)(a),(short)(b),(short)(c),    \
                (short)(d),(short)(e),(short)(f),(unsigned long)(g),(short)(h))
#define styled_piearc(a,b,c,d,e,f,g,h)  \
        dm_cmd(STYLED_PIEARC, 9,64,(short)(a),(short)(b),(short)(c),    \
                (short)(d),(short)(e),(short)(f),(unsigned long)(g),(short)(h))
