;-------------------------------------------------------------------------;
;                                    TIGA                                 ;
;          Copyright (C) 1988-1990  Texas Instruments Incorporated.       ;
;                            All Rights Reserved                          ;
;-------------------------------------------------------------------------;
;   TIGA - Include file                                                   ;
;-------------------------------------------------------------------------;
;                                                                         ;      
;  file            gsptype.inc (GSP Assembly version of typedefs.h)       ;
;                                                                         ;
;  description     This file contains type definitions used in both       ;
;                  the Graphics Manager and Applications Interface.       ;
;                  It is needed for the gsp side of an application,       ;
;                  written in GSP-Assembler, which accesses data of these ;
;                  types.                                                 ;
;                                                                         ;
;-------------------------------------------------------------------------;

;----------------------------------------------------------------------------;
;   Integral data type sizes                                                 ;
;----------------------------------------------------------------------------;
SB      .set    8           ;Size of byte type in bits
SW      .set    16          ;Size of word (short) type in bits
SDW     .set    32          ;Size of double word (long) type in bits
SPTR    .set    32          ;Size of pointer in bits

;----------------------------------------------------------------------------;
;   BITMAP structure definition                                              ;
;                                                                            ;
;       This structure contains information describing a bitmap.             ;
;----------------------------------------------------------------------------;
BITMAP_ADDR     .set    0                   ; PTR        Starting address 
BITMAP_PITCH    .set    BITMAP_ADDR+SPTR    ; ushort     Pitch in bits    
BITMAP_XEXT     .set    BITMAP_PITCH+SW     ; ushort     X extent         
BITMAP_YEXT     .set    BITMAP_XEXT+SW      ; ushort     Y extent         
BITMAP_PSIZE    .set    BITMAP_YEXT+SW      ; ushort     Pixel Size       
                                            
;----------------------------------------------------------------------------;
;   CURSOR structure definition                                              ;
;                                                                            ;
;       This structure contains cursor description information.              ;
;----------------------------------------------------------------------------;
CURSOR_HOT_X       .set    0                    ; short     offset from top...  
CURSOR_HOT_Y       .set    CURSOR_HOT_X+SW      ; short     ...left-hand corner 
CURSOR_WIDTH       .set    CURSOR_HOT_Y+SW      ; ushort    array width         
CURSOR_HEIGHT      .set    CURSOR_WIDTH+SW      ; ushort    array height        
CURSOR_PITCH       .set    CURSOR_HEIGHT+SW     ; ushort  
CURSOR_COLOR       .set    CURSOR_PITCH+SW      ; ulong     shape color
CURSOR_MASK_ROP    .set    CURSOR_COLOR+SDW     ; ushort    cursor mask rop     
CURSOR_SHAPE_ROP   .set    CURSOR_MASK_ROP+SW   ; ushort    cursor shape rop         
CURSOR_MASK_COLOR  .set    CURSOR_SHAPE_ROP+SW  ; ulong     mask color
CURSOR_DATA        .set    CURSOR_MASK_COLOR+SDW; PTR       pointer to cursor data 
            
;----------------------------------------------------------------------------;
;   PATTERN structure definition                                             ;
;                                                                            ;
;       This structure contains pattern description information.             ;
;----------------------------------------------------------------------------;
PATTERN_WIDTH  .set    0                  ; ushort      width of pattern    
PATTERN_HEIGHT .set    PATTERN_WIDTH+SW   ; ushort      height of pattern   
PATTERN_DEPTH  .set    PATTERN_HEIGHT+SW  ; ushort      depth (bits/pixel)  
PATTERN_DATA   .set    PATTERN_DEPTH+SW   ; PTR         pointer to data     
PATTERN_HSRV   .set    PATTERN_DATA+SPTR  ; PTR         pointer to horz fill routine 
PATTERN_SRV    .set    PATTERN_HSRV+SPTR  ; PTR         pointer to general fill routine 

;----------------------------------------------------------------------------;
;   PALET structure definition                                               ;
;                                                                            ;
;       This structure contains the rgb and intensity components for a       ;
;       palette entry.                                                       ;
;----------------------------------------------------------------------------;
PALET_R     .set    0               ; uchar   
PALET_G     .set    PALET_R+SB      ; uchar   
PALET_B     .set    PALET_G+SB      ; uchar   
PALET_I     .set    PALET_B+SB      ; uchar   

;----------------------------------------------------------------------------;
;   OFFSCREEN structure definitions                                          ;
;                                                                            ;
;       These structures contain definitions of offscreen work areas         ;
;----------------------------------------------------------------------------;
OFFSCREEN_ADDR    .set    0                     ; PTR     address in gsp space of an offscreen work area
OFFSCREEN_XEXT    .set    OFFSCREEN_ADDR+SPTR   ; ushort  x extension in the current screen pixel size  
OFFSCREEN_YEXT    .set    OFFSCREEN_XEXT+SW     ; ushort  y extension using default screen pitch        

;----------------------------------------------------------------------------;
;   CONFIG structure definition                                              ;
;                                                                            ;
;       This structure contains the current hardware operating               ;
;       configuration.                                                       ;
;----------------------------------------------------------------------------;
CONFIG_VERSION_NUMBER      .set    0                                  ; ushort
CONFIG_COMM_BUFF_SIZE      .set    CONFIG_VERSION_NUMBER+SW           ; ulong 
CONFIG_SYS_FLAGS           .set    CONFIG_COMM_BUFF_SIZE+SDW          ; ulong 
CONFIG_DEVICE_REV          .set    CONFIG_SYS_FLAGS+SDW               ; ulong 
CONFIG_NUM_MODES           .set    CONFIG_DEVICE_REV+SDW              ; ushort
CONFIG_CURRENT_MODE        .set    CONFIG_NUM_MODES+SW                ; ushort
CONFIG_PROGRAM_MEM_START   .set    CONFIG_CURRENT_MODE+SW             ; ulong 
CONFIG_PROGRAM_MEM_END     .set    CONFIG_PROGRAM_MEM_START+SDW       ; ulong 
CONFIG_DISPLAY_MEM_START   .set    CONFIG_PROGRAM_MEM_END+SDW         ; ulong 
CONFIG_DISPLAY_MEM_END     .set    CONFIG_DISPLAY_MEM_START+SDW       ; ulong 
CONFIG_STACK_SIZE          .set    CONFIG_DISPLAY_MEM_END+SDW         ; ulong 
CONFIG_SHARE_MEM_SIZE      .set    CONFIG_STACK_SIZE+SDW              ; ulong 
CONFIG_SHARE_HOST_ADDR     .set    CONFIG_SHARE_MEM_SIZE+SDW          ; PTR   
CONFIG_SHARE_GSP_ADDR      .set    CONFIG_SHARE_HOST_ADDR+SPTR        ; PTR   

CONFIG_DISP_PITCH          .set    CONFIG_SHARE_GSP_ADDR+SPTR         ; ulong   display pitch y::x                   
CONFIG_DISP_VRES           .set    CONFIG_DISP_PITCH+SDW              ; ushort  vertical resolution (pixels)         
CONFIG_DISP_HRES           .set    CONFIG_DISP_VRES+SW                ; ushort  horizontal resolution (pixels)       
CONFIG_SCREEN_WIDE         .set    CONFIG_DISP_HRES+SW                ; short   screen width                         
CONFIG_SCREEN_HIGH         .set    CONFIG_SCREEN_WIDE+SW              ; short   screen height                        
CONFIG_DISP_PSIZE          .set    CONFIG_SCREEN_HIGH+SW              ; ushort  pixel size                           
CONFIG_PIXEL_MASK          .set    CONFIG_DISP_PSIZE+SW               ; ulong   pixel mask                           
CONFIG_PALET_GUN_DEPTH     .set    CONFIG_PIXEL_MASK+SDW              ; ushort  # of bits per gun                    
CONFIG_PALET_SIZE          .set    CONFIG_PALET_GUN_DEPTH+SW          ; ulong   number of palette entries            
CONFIG_PALET_INSET         .set    CONFIG_PALET_SIZE+SDW              ; short   offset from frame start to data      
CONFIG_NUM_PAGES           .set    CONFIG_PALET_INSET+SW              ; ushort  number of display pages              
CONFIG_NUM_OFFSCRN_AREAS   .set    CONFIG_NUM_PAGES+SW                ; short   number off offscreen work areas      
CONFIG_WKSP_ADDR           .set    CONFIG_NUM_OFFSCRN_AREAS+SW        ; ulong   address of offscreen workspace       
CONFIG_WKSP_PITCH          .set    CONFIG_WKSP_ADDR+SDW               ; ulong   pitch of offscreen workspace         
CONFIG_SILICON_CAPABILITY  .set    CONFIG_WKSP_PITCH+SDW              ; ushort  flag for VRAM block write etc.
CONFIG_COLOR_CLASS         .set    CONFIG_SILICON_CAPABILITY+SW       ; ushort  Color Type Flags
CONFIG_RED_MASK            .set    CONFIG_COLOR_CLASS+SW              ; ulong   Red Mask                             
CONFIG_GREEN_MASK          .set    CONFIG_RED_MASK+SDW                ; ulong   Red Mask                             
CONFIG_BLUE_MASK           .set    CONFIG_GREEN_MASK+SDW              ; ulong   Red Mask                             
CONFIG_X_ASPECT            .set    CONFIG_BLUE_MASK+SDW               ; ushort  Windows X Aspect                     
CONFIG_Y_ASPECT            .set    CONFIG_X_ASPECT+SW                 ; ushort  Windows X Aspect                     
CONFIG_DIAGONAL_ASPECT     .set    CONFIG_Y_ASPECT+SW                 ; ushort  Windows Diagonal Aspect              
                         
;----------------------------------------------------------------------------;
;   ENVIRONMENT structure definition                                         ;
;                                                                            ;
;       This structure contains the current values of environment global     ;
;       variables.                                                           ;
;----------------------------------------------------------------------------;
ENVIRONMENT_XYORIGIN        .set    0                           ; ulong xy drawing origin               
ENVIRONMENT_PENSIZE         .set    ENVIRONMENT_XYORIGIN+SDW    ; ulong pensize height::width   
ENVIRONMENT_SRCBM           .set    ENVIRONMENT_PENSIZE+SDW     ; PTR   source bitmap struct    
ENVIRONMENT_DSTBM           .set    ENVIRONMENT_SRCBM+SPTR      ; PTR     dest bitmap struct    
ENVIRONMENT_STYLEMASK       .set    ENVIRONMENT_DSTBM+SPTR      ; ulong styled line patn mask   

;----------------------------------------------------------------------------;
;   PACKET structure definition                                              ;
;                                                                            ;
;       This structure is used to store lists of heap packets                ;
;----------------------------------------------------------------------------;
PACKET_SIZE .set    0               ; ulong
PACKET_PTR  .set    PACKET_SIZE+SDW ; PTR                       
            
;------------------------------------------------------------------------;
;   MODULE structure definition                                          ;
;                                                                        ;
;   This structure defines the data associated with a function module.   ;
;------------------------------------------------------------------------;
MODULE_LISTADDR     .set    0                       ; PTR     
MODULE_MALLOCADDR   .set    MODULE_LISTADDR+SPTR    ; PTR     
MODULE_MALLOCBYTES  .set    MODULE_MALLOCADDR+SPTR  ; ulong   
MODULE_NUMFUNCS     .set    MODULE_MALLOCBYTES+SDW  ; ushort  
MODULE_FLAGS        .set    MODULE_NUMFUNCS+SW      ; ushort  
MODULE_LENGTH       .set    MODULE_FLAGS+SW         ; length of MODULE structure
                    
;-------------------------------------------------------------------------;
;  params data structure definition                                       ;
;                                                                         ;
;    Data structure for storage of parameters for filled pie arc.         ;
;                                                                         ;
;-------------------------------------------------------------------------;
ARC_WIDTH   .set    0               ; short
ARC_HEIGHT  .set    ARC_WIDTH+SW    ; short     dimensions of rect. containing oval                
ARC_XNORM   .set    ARC_HEIGHT+SW   ; short
ARC_YNORM   .set    ARC_XNORM+SW    ; short     normalized arc starting coordinates
ARC_XCOUNT  .set    ARC_YNORM+SW    ; short
ARC_YCOUNT  .set    ARC_XCOUNT+SW   ; short     counters for movement in x and y 
ARC_XCOORD  .set    ARC_YCOUNT+SW   ; short
ARC_YCOORD  .set    ARC_XCOORD+SW   ; short     actual arc starting coordinates 
ARC_DELTAX  .set    ARC_YCOORD+SW   ; short
ARC_DELTAY  .set    ARC_DELTAX+SW   ; short     x and y increments 
ARC_PATN    .set    ARC_DELTAY+SW   ; short     1 => pattern fill, 0 => solid fill 

;------------------------------------------------------------------------;
;   FONT structure definition                                            ;
;                                                                        ;
;   This structure defines the data associated with a font               ;
;------------------------------------------------------------------------;
FONT_MAGIC     .set    0                      ;(short) font type code
FONT_LENGTH    .set    FONT_MAGIC+SW          ;(long) font size in bytes
FONT_FACENAME  .set    FONT_LENGTH+SDW        ;(char [30]) facename
FONT_DEFLT     .set    FONT_FACENAME+(SB*30)  ;(short) ASCII code of default character
FONT_FIRST     .set    FONT_DEFLT+(SW)        ;(short) ASCII code of first character
FONT_LAST      .set    FONT_FIRST+SW          ;(short) ASCII code of last character
FONT_MAXWIDE   .set    FONT_LAST+SW           ;(short) maximum character width
FONT_MAXKERN   .set    FONT_MAXWIDE+SW        ;(short) maximum character kerning amount
FONT_CHARWIDE  .set    FONT_MAXKERN+SW        ;(short) width of characters (0=proportional)
FONT_AVGWIDE   .set    FONT_CHARWIDE+SW       ;(short) average width of characters
FONT_CHARHIGH  .set    FONT_AVGWIDE+SW        ;(short) character height
FONT_ASCENT    .set    FONT_CHARHIGH+SW       ;(short) ascent (how far above base line)
FONT_DESCENT   .set    FONT_ASCENT+SW         ;(short) descent (how far below base line)
FONT_LEADING   .set    FONT_DESCENT+SW        ;(short) leading (row bottom to next row top)
FONT_ROWPITCH  .set    FONT_LEADING+SW        ;(long) bits per row of char patterns
FONT_OPATNTBL  .set    FONT_ROWPITCH+SDW      ;(long) offset to pattern table
FONT_OLOCTBL   .set    FONT_OPATNTBL+SDW      ;(long) offset to loc table
FONT_OOWTBL    .set    FONT_OLOCTBL+SDW       ;(long) offset to offset/width table

;-----------------------------------------------------------------------;
;   FONTINFO   structure definition                                     ;
;-----------------------------------------------------------------------;
FONTINFO_FACENAME  .set    0              
FONTINFO_DEFLT     .set    FONTINFO_FACENAME+(SB*30)  ;ASCII code of dflt character                 
FONTINFO_FIRST     .set    FONTINFO_DEFLT+SW          ;ASCII code of first character                 
FONTINFO_LAST      .set    FONTINFO_FIRST+SW          ;ASCII code of last character                  
FONTINFO_MAXWIDE   .set    FONTINFO_LAST+SW           ;maximum character width                       
FONTINFO_AVGWIDE   .set    FONTINFO_MAXWIDE+SW        ;Average width of characters                   
FONTINFO_MAXKERN   .set    FONTINFO_AVGWIDE+SW        ;Max character kerning amount                  
FONTINFO_CHARWIDE  .set    FONTINFO_MAXKERN+SW        ;Width of characters (0=proportional)          
FONTINFO_CHARHIGH  .set    FONTINFO_CHARWIDE+SW       ;character height                              
FONTINFO_ASCENT    .set    FONTINFO_CHARHIGH+SW       ;ascent (how far above base line)              
FONTINFO_DESCENT   .set    FONTINFO_ASCENT+SW         ;descent (how far below base line)             
FONTINFO_LEADING   .set    FONTINFO_DESCENT+SW        ;leading (row bottom to next row top)          
FONTINFO_FONTPTR   .set    FONTINFO_LEADING+SW        ;address of font in gsp memory                
FONTINFO_ID        .set    FONTINFO_FONTPTR+SPTR      ;id of font (set at install time)             

;-----------------------------------------------------------------------;
;  ENVTEXT structure definition                                         ;
;                                                                       ;
;  This structure defines the data associated with the text environment ;
;-----------------------------------------------------------------------;
ENVTEXT_INSTALLED .set    0                     ;Number of fonts installed                
ENVTEXT_ALLOCATED .set    ENVTEXT_INSTALLED+SW  ;Number of slots allocated                
ENVTEXT_FONT      .set    ENVTEXT_ALLOCATED+SW  ;Pointer to installed fonts           
ENVTEXT_SELECTED  .set    ENVTEXT_FONT+SDW      ;Currently selected font address           
ENVTEXT_ALIGN     .set    ENVTEXT_SELECTED+SDW  ;Alignment (relative to baseline or topleft)   
ENVTEXT_CHAREXTRA .set    ENVTEXT_ALIGN+SW      ;Additional intercharacter spacing            
ENVTEXT_EFFECTS   .set    ENVTEXT_CHAREXTRA+SW  ;Special effects                  
ENVTEXT_XPOSN     .set    ENVTEXT_EFFECTS+SDW   ;Current X text position
ENVTEXT_YPOSN     .set    ENVTEXT_XPOSN+SW      ;Current Y text position

;-----------------------------------------------------------------------;
;  ENVCURS structure definition                                         ;
;                                                                       ;
;  This structure defines the data associated with the cursor env       ;
;-----------------------------------------------------------------------;
ENVCURS_X         .set    0                   ;ushort   Cursor x position 
ENVCURS_Y         .set    ENVCURS_X+SW        ;ushort   Cursor y position 
ENVCURS_STATE     .set    ENVCURS_Y+SW        ;ushort   Cursor state; 0=disabled, 1=enabled
ENVCURS_SHAPE     .set    ENVCURS_STATE+SW    ;PTR      current cursor shape 
ENVCURS_SAVE      .set    ENVCURS_SHAPE+SPTR  ;PTR      cursor save bitmap    

;----------------------------------------------------------------------------;
;   MONITORINFO10 structure definition                                       ;
;                                                                            ;
;       Note this structure is specific to the 34010                         ;
;       This structure contains video timing parameters.                     ;
;----------------------------------------------------------------------------;
MONITOR_HESYNC10        .set   0                           ; ushort  
MONITOR_HEBLNK10        .set   MONITOR_HESYNC10+SW         ; ushort  
MONITOR_HSBLNK10        .set   MONITOR_HEBLNK10+SW         ; ushort  
MONITOR_HTOTAL10        .set   MONITOR_HSBLNK10+SW         ; ushort  
MONITOR_VESYNC10        .set   MONITOR_HTOTAL10+SW         ; ushort  
MONITOR_VEBLNK10        .set   MONITOR_VESYNC10+SW         ; ushort  
MONITOR_VSBLNK10        .set   MONITOR_VEBLNK10+SW         ; ushort  
MONITOR_VTOTAL10        .set   MONITOR_VSBLNK10+SW         ; ushort  
MONITOR_DPYCTL10        .set   MONITOR_VTOTAL10+SW         ; ushort  
MONITOR_SCREEN_DELAY10  .set   MONITOR_DPYCTL10+SW         ; ushort  
MONITOR_FLAGS10         .set   MONITOR_SCREEN_DELAY10+SW   ; ushort  
                        
;----------------------------------------------------------------------------;
;   PAGE10 structure definitions                                             ;
;                                                                            ;
;       Note this structure is specific to the 34010                         ;
;       These structures contain definitions used to implement multiple      ;
;       display pages.  The information in these structures is used to       ;
;       display a given page.                                                ;
;----------------------------------------------------------------------------;
PAGE_BASEADDR10 .set    0                       ; ulong         Base address of start of page                     
PAGE_DPYSTART10 .set    PAGE_BASEADDR10+SDW     ; ushort        Value loaded in Display Start i/o register 
PAGE_DUMMYPAD10 .set    PAGE_DPYSTART10+SW      ; short         16 bits to pad structure to power of 2 size

;----------------------------------------------------------------------------;
;   MONITORINFO20 structure definition                                       ;
;                                                                            ;
;       Note this structure is specific to the 34020                         ;
;       This structure contains video timing parameters.                     ;
;----------------------------------------------------------------------------;
MONITOR_VESYNC20        .set   0                           ; ushort  
MONITOR_HESYNC20        .set   MONITOR_VESYNC20+SW         ; ushort  
MONITOR_VEBLNK20        .set   MONITOR_HESYNC20+SW         ; ushort  
MONITOR_HEBLNK20        .set   MONITOR_VEBLNK20+SW         ; ushort  
MONITOR_VSBLNK20        .set   MONITOR_HEBLNK20+SW         ; ushort  
MONITOR_HSBLNK20        .set   MONITOR_VSBLNK20+SW         ; ushort  
MONITOR_VTOTAL20        .set   MONITOR_HSBLNK20+SW         ; ushort  
MONITOR_HTOTAL20        .set   MONITOR_VTOTAL20+SW         ; ushort  
MONITOR_DPYCTL20        .set   MONITOR_HTOTAL20+SW         ; ushort  
MONITOR_DINCL20         .set   MONITOR_DPYCTL20+SW         ; ushort  
MONITOR_DINCH20         .set   MONITOR_DINCL20+SW          ; ushort  
MONITOR_HESERR20        .set   MONITOR_DINCH20+SW          ; ushort  
MONITOR_DPYMSK20        .set   MONITOR_HESERR20+SW         ; ushort  
MONITOR_SCREEN_DELAY20  .set   MONITOR_DPYMSK20+SW         ; ushort  
MONITOR_FLAGS20         .set   MONITOR_SCREEN_DELAY20+SW   ; ushort  

;----------------------------------------------------------------------------;
;   PAGE20 structure definitions                                             ;
;                                                                            ;
;       Note this structure is specific to the 34020                         ;
;       These structures contain definitions used to implement multiple      ;
;       display pages.  The information in these structures is used to       ;
;       display a given page.                                                ;
;----------------------------------------------------------------------------;
PAGE_BASEADDR20 .set    0                       ; ulong         Base address of start of page                     
PAGE_DPYSTART20 .set    PAGE_BASEADDR20+SDW     ; ulong         Value loaded in Display Start i/o register 

;----------------------------------------------------------------------------;
;                                                                            ;
;   Memory map structure                                                     ;
;                                                                            ;
;----------------------------------------------------------------------------;
MM_END_TIGAGM   .set    0                       ; dword
MM_NUM_DRAMS    .set    MM_END_TIGAGM+SDW       ; word
MM_NUM_VRAMS    .set    MM_NUM_DRAMS+SW         ; word
MM_DRAM_START   .set    MM_NUM_VRAMS+SW         ; dword  
MM_DRAM_SIZE    .set    MM_DRAM_START+SDW       ; dword    

