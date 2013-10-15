#define DEFAULT_KBD_NOTICE 1  /* ie, 'yes', do notice key strokes */
#define MAX_PAUSE  14400  /* 14,400 seconds or 4 hours */
#define INFINITE_LOOP  999

extern char notice_keys;

#define R_FADEIN    30  /* reserved words */  
#define R_FADEOUT   31
#define R_CUT       32  /* this value important in vbat.c */

#define USE_SET_SPEED -1  /* ie, use the speed that is set in the fli file */
#define DEFAULT_SPEED USE_SET_SPEED  
#define DEFAULT_PAUSE 0
#define DEFAULT_GIF_PAUSE  5  /* 5 seconds */
#define DEFAULT_LOOP  1/* for flis & bat files the default is 1 loop */
#define DEFAULT_IN_TRANS   R_CUT
#define DEFAULT_OUT_TRANS  R_CUT
#define SL_MAX_SPEED 120
#define DEFAULT_TRANSIT_CYCLE 10

#define BREAK_PROCESS  	283  /* scan code of the escape key */
#define PLUS_KEY 	3371
#define NUM_PLUS_KEY 	20011
#define MINUS_KEY 	3117
#define NUM_MINUS_KEY 	18989
#define IS_PLUS(x)   ((x)==PLUS_KEY  || (x)==NUM_PLUS_KEY)
#define IS_MINUS(x)  ((x)==MINUS_KEY || (x)==NUM_MINUS_KEY)

#define F1 15104
#define F2 15360
#define F3 15616
#define F4 15872
#define F5 16128
#define F6 16384
#define F7 16640
#define F8 16896
#define F9 17152
#define F10 17408
#define F11 17664
#define F12 17920
#define is_funckey(x)  ( (x)>=F1 && (x)<=F10 )


advance_frame(Video_form *screen,int hdware_update);
load_frame1(char *name,Video_form *screen,int hdware_update,int close_f);
goto_frame(int old_val,int new_val);
