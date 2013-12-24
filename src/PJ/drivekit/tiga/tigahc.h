/*-----------------------------------------------------------------------*/
/*                                   TIGA                                */
/*         Copyright (C) 1989-1990  Texas Instruments Incorporated.      */
/*                           All Rights Reserved                         */
/*-----------------------------------------------------------------------*/
/*  TIGA - Header file for MetaWare High C Compiler                      */
/*-----------------------------------------------------------------------*/
/*                                                                       */
/*  file            tiga.hch                                             */
/*                                                                       */
/*  description     This is the main include file defining TIGA core     */
/*                  primitives and should be included in every           */
/*                  High-C application that interfaces to TIGA.          */
/*                                                                       */
/*-----------------------------------------------------------------------*/
/*  03/10/89    Reordered command numbers                     G.Short    */
/*  04/23/90    Added _DOUBLE packet type defs                AB         */
/*-----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
/*  Reference Host-side entry points                                    */
/*----------------------------------------------------------------------*/
extern          long  tiga_set(short);
extern          long  cp_alt(short,short,...);
extern          long  cp_alt_a(short,short,...);
extern          void  cp_cmd(short,short,...);
extern          void  cp_cmd_a(short,short,...);
extern          long  cp_ret(short,short,...);
extern          long  cp_ret_a(short,short,...);
extern          short create_alm(char *,char *);
extern          short create_esym(void);
extern          void  dm_cmd(short,short,...);
extern          long  dm_palt(short,short,void *);
extern          void  dm_pcmd(short,short,...);
extern          void  dm_ipoly(short,short,...);
extern          long  dm_pget(short,short,void *);
extern          void  dm_poly(short,short,short,void *);
extern          long  dm_pret(short,short,...);
extern          void  dm_psnd(short,short,void *);
extern          void  dm_pstr(short,void *);
extern          long  dm_ptrx(short,short,void *,short,void *);
extern          long  dm_ret(short,short,...);
extern unsigned long  field_extract(unsigned long,unsigned short);
extern          void  field_insert(unsigned long,unsigned short,unsigned long);
extern          short flush_esym(void);
extern          void  get_memseg(unsigned long *,unsigned long *);
extern unsigned short get_modeinfo(short,void *);
extern unsigned short get_msg(void);
extern unsigned long  get_vector(short);
extern unsigned short get_videomode(void);
extern unsigned short get_xstate(void);
extern          short gm_is_alive(void);
extern          void  gsp_execute(unsigned long);
extern          void  gsp2host(long,void *,short,short);
extern          void  gsp2hostxy(long,long,void *,long,short,short,short,short,short,short,short,short);
extern          short handshake(void);
extern          void  host2gsp(void *,long,short,short);
extern          void  host2gspxy(void *,long,long,long,short,short,short,short,short,short,short,short);
extern          short install_alm(char *);
extern          short install_primitives(void);
extern          short install_rlm(char *);
extern          void  install_usererror(void (*)());
extern unsigned long  loadcoff(char *);
extern          void  makename(char *,char *,unsigned short,char *);
extern unsigned long  oem_init(unsigned long);
extern unsigned short read_hstadrh(void);
extern unsigned short read_hstadrl(void);
extern unsigned long  read_hstaddr(void);
extern unsigned short read_hstctl(void);
extern unsigned short read_hstdata(void);
extern unsigned short rstr_commstate(void);
extern unsigned short save_commstate(void);
extern          short set_config(short,short);
extern          void  set_curs_shape(unsigned long);
extern          void  set_curs_xy(short,short);
extern          void  set_memseg(unsigned long,unsigned long);
extern          void  set_msg(unsigned short);
extern          void  set_timeout(short);
extern unsigned long  set_vector(short,unsigned long);
extern          short set_videomode(unsigned short,unsigned short);
extern          void  set_xstate(unsigned short);
extern          void  synchronize(void);
extern          void  write_hstadrh(unsigned short);
extern          void  write_hstadrl(unsigned short);
extern          void  write_hstaddr(unsigned long);
extern          void  write_hstctl(unsigned short);
extern          void  write_hstdata(unsigned short);
extern          void  _far *aux_command(unsigned short, void *);
extern          void  setup_hostcmd( void * );
extern          void  hook_init_pal(short,short,...);
extern unsigned long  hook_get_near(short,short,...);
extern          void  hook_get_pal(short,short,...);
extern          void  hook_set_pal(short,short,...);
extern          short hook_get_pal_e(short,short,...);
extern          short hook_set_pal_e(short,short,...);

/*----------------------------------------------------------------------*/
/*  Define "C" packet data types                                        */
/*----------------------------------------------------------------------*/
#define _WORD(a)                0,sizeof(short),(short)(a)
#define _SWORD(a)               2,sizeof(short),(short)(a)
#define _DWORD(a)               0,sizeof(long),(long)(a)
#define _DOUBLE(a)              4,sizeof(double),(double)(a)

#define _BYTE_PTR(a,b)          1,(short)((a) * sizeof(char)),(char *)(b)
#define _WORD_PTR(a,b)          1,(short)((a) * sizeof(short)),(short *)(b)
#define _DWORD_PTR(a,b)         1,(short)((a) * sizeof(long)),(long *)(b)
#define _DOUBLE_PTR(a,b)        4+1,(short)((a) * sizeof(double)),(double *)(b)
#define _STRING(a)              64+1,0,(char *)(a)

#define _ALTBYTE_PTR(a,b)       128+1,(short)((a) * sizeof(char)),(char *)(b)
#define _ALTWORD_PTR(a,b)       128+1,(short)((a) * sizeof(short)),(short *)(b)
#define _ALTDWORD_PTR(a,b)      128+1,(short)((a) * sizeof(long)),(long *)(b)
#define _ALTDOUBLE_PTR(a,b)     128+4+1,(short)((a) * sizeof (double)),(double *)(b)

/*----------------------------------------------------------------------*/
/*  Define command number modifiers                                     */
/*----------------------------------------------------------------------*/
#define CORE_CP(a)              (a) | 0x7E00
#define CORE_DM(a)              (a) | 0x3E00
#define EXT_CP(a)               (a) | 0x7C00
#define EXT_DM(a)               (a) | 0x3C00
#define USER_CP(a)              (a) | 0x4000
#define USER_DM(a)              (a) | 0x0000

/*----------------------------------------------------------------------*/
/*  Core functions                                                      */
/*----------------------------------------------------------------------*/
#define ADD_INTERRUPT         CORE_CP( 0) 
#define DEL_INTERRUPT         CORE_CP( 1) 
#define FUNCTION_IMPLEMENTED  CORE_CP( 2) 
#define GET_COLORS            CORE_CP( 3) 
#define GET_CONFIG            CORE_CP( 4) 
#define SET_MODULE_STATE      CORE_CP( 5)
#define GSP_MAXHEAP           CORE_CP( 6) 
#define GET_OFFSCREEN_MEMORY  CORE_CP( 7) 
#define GET_PALET_ENTRY       CORE_CP( 8) 
#define GET_PMASK             CORE_CP( 9) 
#define GET_PPOP              CORE_CP(10) 
#define GET_TRANSP            CORE_CP(11) 
#define GET_WINDOWING         CORE_CP(12) 
#define GSP_CALLOC            CORE_CP(13) 
#define GSP_FREE              CORE_CP(14) 
#define GSP_MALLOC            CORE_CP(15) 
#define GSP_MINIT             CORE_CP(16) 
#define GSP_REALLOC           CORE_CP(17) 
#define INIT_CURSOR           CORE_CP(18) 
#define SET_CONFIG            CORE_CP(20) 
#define PAGE_FLIP             CORE_CP(21) 
#define SET_PALET             CORE_CP(22) 
#define TRANSP_OFF            CORE_CP(23) 
#define TRANSP_ON             CORE_CP(24) 
#define VIDEO_ENABLE          CORE_CP(25) 
#define GET_STATE             CORE_CP(26) 
#define CLEAR_SCREEN          CORE_DM(27) 
#define GET_NEAREST_COLOR     CORE_DM(28) 
#define GET_PALET             CORE_CP(29) 
#define GSP2GSP               CORE_DM(30) 
#define INIT_PALET            CORE_DM(31) 
#define LMO                   CORE_DM(32) 
#define PALLOC                CORE_DM(33) 
#define PEEK_BREG             CORE_DM(34) 
#define POKE_BREG             CORE_DM(35) 
#define RMO                   CORE_DM(36) 
#define SET_BCOLOR            CORE_DM(37) 
#define SET_ALLCOLORS         CORE_DM(38)  /* Changed because of AA386 prob */
#define SET_FCOLOR            CORE_DM(39) 
#define SET_INTERRUPT         CORE_CP(40) 
#define SET_PALET_ENTRY       CORE_DM(41) 
#define SET_PMASK             CORE_DM(42) 
#define SET_PPOP              CORE_DM(43) 
#define SET_WINDOWING         CORE_DM(44) 
#define SET_CLIP_RECT         CORE_DM(45) 
#define WAIT_SCAN             CORE_CP(46) 
#define SET_WKSP              CORE_DM(47) 
#define ADD_MODULE            CORE_CP(48) 
#define DEL_MODULE            CORE_CP(49) 
#define DEL_ALL_MODULES       CORE_CP(50) 
#define GET_ISR_PRIORITIES    CORE_CP(51) 
#define INIT_INTERRUPTS       CORE_CP(52) 
#define SET_CURS_SHAPE        CORE_DM(53) 
#define SET_CURS_STATE        CORE_DM(54) 
#define GET_CURS_STATE        CORE_CP(55) 
#define GET_CURS_XY           CORE_CP(56) 
#define FLUSH_EXTENDED        CORE_CP(57) 
#define PAGE_BUSY             CORE_CP(58) 
#define CLEAR_FRAME_BUFFER    CORE_DM(59) 
#define CLEAR_PAGE            CORE_DM(60) 
#define GET_WKSP              CORE_CP(61) 
#define CPW                   CORE_DM(62) 
#define INIT_TEXT             CORE_CP(63) 
#define TEXT_OUT              CORE_CP(64) 
#define GET_FONTINFO          CORE_CP(65) 
#define GET_MODULE            CORE_CP(66)
#define CVXYL                 CORE_DM(67)
#define SET_CURSATTR          CORE_DM(68)
#define COP2GSP               CORE_CP(69)
#define GSP2COP               CORE_CP(70)
#define SET_TRANSP            CORE_DM(71)
#define GET_TEXT_XY           CORE_CP(72)
#define SET_TEXT_XY           CORE_DM(73)
#define TEXT_OUTP             CORE_DM(74)
#define HBM_DEREF             CORE_CP(76)
#define HBM_SETMEMTYPE        CORE_CP(77)
#define HBM_ALLOC             CORE_CP(78)
#define HBM_FALLOC            CORE_CP(79)
#define HBM_FREE              CORE_CP(80)
#define HBM_TOTALFREE         CORE_CP(81)
#define HBM_MAXHEAP           CORE_CP(82)
#define HBM_COMPACT           CORE_CP(83)
#define HBM_REALLOC           CORE_CP(84)
#define HBM_FINDMEM           CORE_CP(85)
#define HBM_INIT              CORE_CP(86)
#define HBM_FINDHANDLE        CORE_CP(87)
#define SYM_OPEN              CORE_CP(90)
#define SYM_CLOSE             CORE_CP(91)
#define SYM_PUT               CORE_CP(92)
#define SYM_GET               CORE_CP(93)
#define SYM_FLUSH             CORE_CP(94)
#define SYM_INIT              CORE_CP(96)
#define HBM_CALLOC            CORE_CP(97)  
#define HBM_FCALLOC           CORE_CP(98)
#define FLUSH_MODULE          CORE_CP(99)

/*----------------------------------------------------------------------*/
/*  Reference Core C-Packet function definitions                        */
/*----------------------------------------------------------------------*/
#define add_interrupt(a,b)          \
        (short)cp_ret(ADD_INTERRUPT,2,_WORD(a),_DWORD(b))
#define del_interrupt(a)            \
        cp_ret(DEL_INTERRUPT,1,_WORD(a))
#define set_interrupt(a,b,c,d)      \
        (short)cp_ret(SET_INTERRUPT,4,_WORD(a),_WORD(b),_WORD(c),_SWORD(d))
#define function_implemented(a)     \
        (short)cp_ret(FUNCTION_IMPLEMENTED,1,_WORD(a))
#define get_colors(a,b)             \
        (void)cp_alt(GET_COLORS,2,_ALTDWORD_PTR(1,a),_ALTDWORD_PTR(1,b))
#define get_config(a)               \
        (void)cp_alt(GET_CONFIG,1,_ALTBYTE_PTR(sizeof(CONFIG),a))
#define gsp_maxheap()               \
        cp_ret(GSP_MAXHEAP,0)
#define get_offscreen_memory(a,b)   \
        (void)cp_alt(GET_OFFSCREEN_MEMORY,2,_WORD(a),_ALTBYTE_PTR((a)*sizeof(OFFSCREEN_AREA),b))
#define get_palet_entry(a,b,c,d,e)  \
        hook_get_pal_e(GET_PALET_ENTRY,5,_DWORD(a),_ALTBYTE_PTR(1,b), \
                         _ALTBYTE_PTR(1,c),_ALTBYTE_PTR(1,d),_ALTBYTE_PTR(1,e))
#define get_palet(a,b)              \
        hook_get_pal(GET_PALET,2,_WORD(a),_ALTBYTE_PTR((a)*4,(b)))
#define get_pmask()                 \
        cp_ret(GET_PMASK,0)
#define get_ppop()                  \
        (unsigned short)cp_ret(GET_PPOP,0)
#define get_transp()                 \
        (short)cp_ret(GET_TRANSP,0)
#define get_windowing()             \
        (short)cp_ret(GET_WINDOWING,0)
#define gsp_calloc(a,b)             \
        cp_ret(GSP_CALLOC,2,_DWORD(a),_DWORD(b))
#define gsp_free(a)                 \
        (short)cp_ret(GSP_FREE,1,_DWORD(a))
#define gsp_malloc(a)               \
        cp_ret(GSP_MALLOC,1,_DWORD(a))
#define gsp_minit(a)                \
        cp_cmd(GSP_MINIT,1,_DWORD(a))
#define gsp_realloc(a,b)            \
        cp_ret(GSP_REALLOC,2,_DWORD(a),_DWORD(b))
#define init_cursor()               \
        cp_ret(INIT_CURSOR,0)
#define page_flip(a,b)              \
        cp_cmd(PAGE_FLIP,2,_WORD(a),_WORD(b))
#define set_palet(a,b,c)            \
        hook_set_pal(SET_PALET,3,_DWORD(a),_DWORD(b),_BYTE_PTR(4*(a),c))
#define transp_off()                \
        cp_cmd(TRANSP_OFF,0)
#define transp_on()                 \
        cp_cmd(TRANSP_ON,0)
#define add_module(a,b)             \
        cp_ret(ADD_MODULE,2,_SWORD(a),_BYTE_PTR(sizeof(MODULE),b))
#define get_state()                 \
        cp_ret(GET_STATE,0)
#define del_module(a)               \
        cp_ret(DEL_MODULE,1,_WORD(a))
#define del_all_modules()           \
        cp_cmd(DEL_ALL_MODULES,0)
#define wait_scan(a)                \
        (void)cp_ret(WAIT_SCAN,1,_WORD(a))
#define get_isr_priorities(a,b)     \
        (void)cp_alt(GET_ISR_PRIORITIES,2,_WORD(a),_ALTWORD_PTR(a,b))
#define get_curs_state()            \
        (short)cp_ret(GET_CURS_STATE,0)
#define get_curs_xy(a,b)            \
        (void)cp_alt(GET_CURS_XY,2,_ALTWORD_PTR(1,a),_ALTWORD_PTR(1,b))
#define flush_extended()            \
        cp_cmd(FLUSH_EXTENDED,0)
#define page_busy()                 \
        (short)cp_ret(PAGE_BUSY,0)
#define get_wksp(a,b)               \
        (short)cp_alt(GET_WKSP,2,_ALTDWORD_PTR(1,a),_ALTDWORD_PTR(1,b))
#define init_text()                 \
        cp_cmd(INIT_TEXT,0)
#define text_out(a,b,c)             \
        (short)cp_ret(TEXT_OUT,3,_SWORD(a),_SWORD(b),_STRING(c))
#define get_fontinfo(a,b)         \
        (short)cp_alt(GET_FONTINFO,2,_SWORD(a),_ALTBYTE_PTR(sizeof(FONTINFO),b))
#define get_module(a,b)             \
        cp_alt(GET_MODULE,2,_WORD(a),_ALTBYTE_PTR(sizeof(MODULE),b))
#define get_text_xy(a,b)            \
        (void)cp_alt(GET_TEXT_XY,2,_ALTWORD_PTR(1,a),_ALTWORD_PTR(1,b))
#define gsph_deref(a)                 \
        cp_ret(HBM_DEREF,1,_WORD(a))
#define gsph_memtype(a,b)          \
        cp_cmd(HBM_SETMEMTYPE,2,_WORD(a),_WORD(b))
#define gsph_alloc(a)                 \
        (unsigned short)cp_ret(HBM_ALLOC,1,_DWORD(a))
#define gsph_calloc(a,b)              \
        (unsigned short)cp_ret(HBM_CALLOC,2,_DWORD(a),_DWORD(b))
#define gsph_falloc(a,b,c)            \
        (unsigned short)cp_ret(HBM_FALLOC,3,_DWORD(a),_DWORD(b),_WORD(c))
#define gsph_fcalloc(a,b,c,d)         \
        (unsigned short)cp_ret(HBM_FCALLOC,4,_DWORD(a),_DWORD(b),_DWORD(c),_WORD(d))
#define gsph_free(a)                  \
        cp_cmd(HBM_FREE,1,_WORD(a))
#define gsph_maxheap()                \
        cp_ret(HBM_MAXHEAP,0)
#define gsph_compact(a)               \
        cp_cmd(HBM_COMPACT,1,_WORD(a))
#define gsph_realloc(a,b)             \
        cp_ret(HBM_REALLOC,2,_WORD(a),_DWORD(b))
#define gsph_findmem(a,b,c)               \
        (short)cp_alt(HBM_FINDMEM,3,_DWORD(a),_ALTWORD_PTR(1,b),_ALTBYTE_PTR(1,c))
#define gsph_init()                   \
        cp_cmd(HBM_INIT,0)
#define gsph_findhandle(a)            \
        (unsigned short)cp_ret(HBM_FINDHANDLE,1,_DWORD(a))
#define gsph_totalfree()              \
        cp_ret(HBM_TOTALFREE,0)
#define set_module_state(a,b)         \
        (short)cp_ret(SET_MODULE_STATE,2,_WORD(a),_WORD(b))
#define sym_init()                  \
        cp_cmd(SYM_INIT, 0)
#define sym_open(a)                 \
        cp_ret(SYM_OPEN, 1, _SWORD(a))
#define sym_close()                 \
        cp_cmd(SYM_CLOSE, 0)
#define sym_put(a,b)                \
        cp_ret(SYM_PUT, 2, _STRING(a), _DWORD(b))
#define sym_get(a)                  \
        cp_ret(SYM_GET, 1, _STRING(a))
#define sym_flush(a)                \
        (short)cp_ret(SYM_FLUSH, 1, _SWORD(a))
#define cop2gsp(a,b,c,d)    \
        cp_cmd(COP2GSP,4,_WORD(a),_DWORD(b),_DWORD(c),_DWORD(d))
#define gsp2cop(a,b,c,d)    \
        cp_cmd(GSP2COP,4,_WORD(a),_DWORD(b),_DWORD(c),_DWORD(d))
#define flush_module(a)     \
        (short)cp_ret(FLUSH_MODULE,1,_WORD(a))

/*----------------------------------------------------------------------*/
/* Direct Mode Flags Identifier                                         */
/*                                                                      */
/* 0 bit = 16 Bit TiGA Stack Value                                      */
/* 1 bit = 32 Bit TiGA Stack Value                                      */
/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
/*  Reference Core Direct-Mode function definitions                     */
/*----------------------------------------------------------------------*/
#define clear_screen(a)             \
        dm_cmd(CLEAR_SCREEN,2,1,(long)(a))
#define get_nearest_color(a,b,c,d)  \
        hook_get_near(GET_NEAREST_COLOR,4,0,(short)(a),(short)(b),(short)(c),(short)(d))
#define init_palet()                \
        hook_init_pal(INIT_PALET,0)
#define lmo(a)                      \
        (short)dm_ret(LMO,2,1,(long)(a))
#define peek_breg(a)                \
        dm_ret(PEEK_BREG,1,0,(short)(a))
#define poke_breg(a,b)              \
        dm_cmd(POKE_BREG,3,2,(short)(a),(long)(b))
#define rmo(a)                      \
        (short)dm_ret(RMO,2,1,(long)(a))
#define set_bcolor(a)               \
        dm_cmd(SET_BCOLOR,2,1,(long)(a))
#define set_colors(a,b)             \
        dm_cmd(SET_ALLCOLORS,4,3,(long)(a),(long)(b))
#define set_fcolor(a)               \
        dm_cmd(SET_FCOLOR,2,1,(long)(a))
#define set_palet_entry(a,b,c,d,e)  \
        hook_set_pal_e(SET_PALET_ENTRY,6,1,(long)(a),(short)(b),(short)(c),\
                             (short)(d),(short)(e))
#define set_pmask(a)                \
        dm_cmd(SET_PMASK,2,1,(long)(a))
#define set_ppop(a)                 \
        dm_cmd(SET_PPOP,1,0,(short)(a))
#define set_windowing(a)            \
        dm_cmd(SET_WINDOWING,1,0,(short)(a))
#define set_clip_rect(a,b,c,d)      \
        dm_cmd(SET_CLIP_RECT,4,0,(short)(a),(short)(b),(short)(c),(short)(d))
#define set_wksp(a,b)               \
        dm_cmd(SET_WKSP,4,3,(unsigned long)(a),(unsigned long)(b))
#define gsp2gsp(a,b,c)              \
        dm_cmd(GSP2GSP,6,7,(unsigned long)(a),(unsigned long)(b),(unsigned long)(c))
#define set_curs_state(a)           \
        dm_cmd(SET_CURS_STATE,1,0,(short)(a))
#define clear_frame_buffer(a)       \
        dm_cmd(CLEAR_FRAME_BUFFER,2,1,(long)(a))
#define clear_page(a)               \
        dm_cmd(CLEAR_PAGE,2,1,(long)(a))
#define cpw(a,b)                    \
        (short)dm_ret(CPW,2,0,(short)(a),(short)(b))
#define cvxyl(a,b)                  \
        dm_ret(CVXYL,2,0,(short)(a),(short)(b))
#define set_cursattr(a,b,c,d)       \
        dm_cmd(SET_CURSATTR,6,3,(unsigned long)(a),(unsigned long)(b),(unsigned short)(c),(unsigned short)(d))
#define set_transp(a)               \
        dm_cmd(SET_TRANSP,1,0,(short)(a))
#define set_text_xy(a,b)            \
        dm_cmd(SET_TEXT_XY,2,0,(short)(a),(short)(b))
#define text_outp(a)                \
        dm_pstr(TEXT_OUTP,(char *)(a))

/*----------------------------------------------------------------------*/
/*  Arguments for tiga_set()                                           */
/*----------------------------------------------------------------------*/
#define CD_CLOSE         0
#define CD_OPEN          1
#define CD_STATUS        2
#define CD_NOT_INSTALLED -4

/*----------------------------------------------------------------------*/
/*  Arguments for set_videomode()                                      */
/*----------------------------------------------------------------------*/
/* Legal video modes */
#define TIGA            -1      
#define PREVIOUS        0        /* restore screen to original mode */
#define MDA             1
#define HERCULES        2
#define CGA             3
#define EGA             4
#define VGA             5
#define AI_8514         6
#define OFF_MODE        7           
/* Legal styles */
#define NO_INIT         0        /* generate entry to TSR only */
#define INIT_GLOBALS    1        /* initialize global variables, retain heap */
#define INIT            3        /* initialize globals and free all heap...  */
                                 /* including downloaded functions           */
#define INIT_GM         7        /* Re-initialize GM                         */
#define NO_ENABLE       0x4000   /* Do not enable the 340 display.           */
#define CLR_SCREEN      0x8000   /* clear the screen on entry to the mode    */

/*----------------------------------*/
/* Colors of the default TIGA palet */
/*----------------------------------*/
#define  BLACK          0
#define  BLUE           1
#define  GREEN          2
#define  CYAN           3
#define  RED            4
#define  MAGENTA        5
#define  BROWN          6
#define  LIGHT_GRAY     7
#define  DARK_GRAY      8
#define  LIGHT_BLUE     9
#define  LIGHT_GREEN    10
#define  LIGHT_CYAN     11
#define  LIGHT_RED      12
#define  LIGHT_MAGENTA  13
#define  YELLOW         14
#define  WHITE          15

/*----------------------------------*/
/* Memory System Equates            */
/*----------------------------------*/
#define TRUE    1
#define FALSE   0
#define BLK_DELETABLE   0x01    /* Block can be deleted */
#define BLK_LOCKED      0x02    /* Block is locked */
#define BLK_FUNCMOVE    0x04    /* Call block function on move */
#define BLK_FUNCDELETE  0x08    /* Call block function on purge */
#define BLK_SECURED     0x10    /* Block is secured memory */
#define BLK_INUSE       0x20    /* Block is allocated */

/*----------------------------------*/
/* Extended primitives module id    */
/* - parameter to sym_flush         */
/*----------------------------------*/
#define GRAPHICS_LIB_ID 0x3C00
