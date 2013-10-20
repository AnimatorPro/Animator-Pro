#include <time.h>  /* for time functions */
#ifndef SLUFF
#include <dos.h>  /* for sound test */
#endif SLUFF

#include "jimk.h"
#include "fli.h"
#include "flicmenu.h"
#include "prjctor.h"

extern char cbrk_hit;

struct fli_head fh;
long frame1off;
int cur_frame_num;
int user_defined_speed;
int orig_speed;
char is_gif;

extern char notice_keys;
extern char stop_bat;
extern int sys5;
extern int frame_val;
extern char file_is_loaded;
extern Video_form alt_vf;
extern unsigned char alt_cmap[COLORS*3];
extern int loaded_file_fd;
extern char global_file_name[];
extern struct qslider speed_sl;
extern long get80hz();
extern WORD mouse_connected;


char ctrl_hit;
int ascii_value;
int ctrl_lock;
#define CTRL  0x0004

long clock1;

view_fli_loop(name, screen, num_loops) 
char *name;
Video_form *screen;
int num_loops;
{
int fd;
int i, k, last_loop=0;

/* screen->p  = VGA_SCREEN; */ /* test */

if (num_loops >= INFINITE_LOOP) num_loops=INFINITE_LOOP;

if ((fd = read_fli_head(name, &fh)) == 0)
	return(0);
clock1 = get80hz();
mouse_on = 0;
if (!read_next_frame(name,fd,screen,1))	/* read in frame 0 */
	goto END;
/* non-seek just to get position */
frame1off = jseek(fd, 0L, 1/*from current position*/);

orig_speed = fh.speed;
if (user_defined_speed!=USE_SET_SPEED) fh.speed=user_defined_speed;
k=0;
for (;;)
	{
	if (num_loops!=INFINITE_LOOP)
		{
		last_loop = (k==num_loops-1) ? 1:0;
		if (k >= num_loops) break;
		k++;
		}
	if (jseek(fd, frame1off, 0) == -1)	/* seek to frame 1 */
		break;
	for (i=1; i<=fh.frame_count; i++)
		{
		if (last_loop && i==fh.frame_count) continue; /* dont end on first frame */
		if (!read_next_frame(name,fd,screen,1))
			goto END;  /*break; */
		clock1 += fh.speed;
		if (!wait_til2(clock1))
			goto END;  /* break; */
		/* in case we took longer than fh.speed ticks to load and
		   display it, need next line so don't try to rush next frame
		   to make up for it... */
		if (clock1 > get80hz())
			clock1 = get80hz();
		}
	}
END:
jclose(fd);
mouse_on = 1;
}



#ifndef EVER

#define UP 	0
#define ZEROFLAG	64

get_key()
{
union regs r;

key_hit = 0;
r.b.ah = 0x1;
if (!(sysint(0x16,&r,&r)&ZEROFLAG))
	{
	key_hit = 1;
	r.b.ah = 0;
	sysint(0x16,&r,&r);
	key_in = r.w.ax;
	}
/** ldg */
/*  function 16H ah=2; returns in al the ROM BIOS keyobard flags */
ctrl_hit=UP;   /* ldg  ????????????????? */
r.b.ah = 0x02;
sysint(0x16, &r, &r);
ctrl_hit = (r.b.al & 0x04) ? 1: 0;
}
#endif EVER




wait_til2(time)
long time;
{
for (;;)
	{
	check_button();
	if (notice_keys && RJSTDN)
		return(0);

	get_key();
	if (key_hit) /* a keystroke is waiting */
		{
		ctrl_hit = (bioskey(2) & CTRL);	/* if was control key */
		ascii_value = ((key_in<<8)>>8);

		/* REPALCXE BIOSKEY 2 */

		return(key_effect(key_in));
		}
	else key_hit=0;		

	if (get80hz() >= time)
		return(1);
	}
}


key_effect(key)
int key;  /* scancode of key hit */
{
		/* interrupt if the ctrl key is held down with keystroke */
if (notice_keys && (key==BREAK_PROCESS))
	{
	stop_bat=1;
	return(0);	
	}
if (ctrl_hit && 1<=ascii_value && ascii_value<=26)
	{
/*	DUMPINT("ctrl hit of key in range of A to Z",0); */
	if (notice_keys)
		{
		ctrl_lock=key;
/*		DUMPINT("ctrl lock set to keystroke--no notice keys",ctrl_lock); */
		notice_keys=0;
		}
	else /* dont notice keys is effective */
		{
/*		DUMPINT("DONT notice keys is in effect--",0); */
		if (key==ctrl_lock)
			{
			notice_keys=1;
/*			DUMPINT("... but the key hit makes us agaion notice keys",0); */
			}
		else 
			{
/*			DUMPINT("returing 0 here",0); */
			return(1); /*			return(0); */
			}
		}
	return(1);
	}
if (notice_keys) 
	{
/*	DUMPINT("its notice keys II",0); */
	if (key==BACKSPACE)
		{
		freeze_frame();
		return(1);
	}
	if (!is_gif)
           	{
		if (is_funckey(key) || IS_PLUS(key) || IS_MINUS(key) ) 
			{
			do_speed_change(key);
			return(1);
			}
		else return(0);  
		}
	else return(0);
	}
return(1);
}






break_key()
{
check_button();
if (notice_keys && RDN)
	return(0);
if (bioskey(1)) /* a keystroke is waiting */
	{
	key_hit=1;
	key_in=bioskey(0); /* get the key */
	ctrl_hit = (bioskey(2) & CTRL);	/* if was control key */
	ascii_value = ((key_in<<8)>>8);
	return(key_effect(key_in));
	}
else 	
	{
	key_hit=0;		
	return(1);
	}
}





freeze_frame()
{
int old_state = mouse_on;

if (mouse_connected) mouse_on=0;
wait_click();
clock1 = get80hz();
mouse_on=old_state;
}





do_speed_change(key)
int key;
{
switch(key)
	{
	case PLUS_KEY:
	case NUM_PLUS_KEY:
		fh.speed =( (fh.speed+1 < speed_sl.max) ? fh.speed+1: speed_sl.max);
		break;
	case MINUS_KEY:
	case NUM_MINUS_KEY:
		fh.speed =( (fh.speed-1 > speed_sl.min) ? fh.speed-1: speed_sl.min);
		break;
	case F1:
		fh.speed = 0;
		break;
	case F2:
		fh.speed = 3;
		break;
	case F3:
		fh.speed = 6;
		break;
	case F4:
		fh.speed = 9;
		break;
	case F5:
		fh.speed = 12;
		break;
	case F6:
		fh.speed = 18;
		break;
	case F7:
		fh.speed = 24;
		break;
	case F8:
		fh.speed = 36;
		break;
	case F9:
		fh.speed = 48;
		break;
	case F10:
		fh.speed = orig_speed;
		break;
	default:
		break;
	}
}






play_gif(gif_name,pause_val,trans_in,trans_out)
char gif_name[];
double pause_val;
int trans_in, trans_out;
{
long  start_time, cur_time;

if (pause_val==0) pause_val=DEFAULT_GIF_PAUSE;
 
load_gif(gif_name,&vf);  /* was alt_vf  */

time(&start_time);
cur_time=start_time;
is_gif=1; /* global flag for changing break_key behavior */
while ( difftime(cur_time, start_time)  < pause_val)  
	{
	if (!break_key()) break;
	time(&cur_time);
	}
if (trans_out==R_FADEOUT) transit_to_white(DEFAULT_TRANSIT_CYCLE,2); 

return(0);
}




play_pic(pic_file,speed1,pause1,num_loops,trans_in,trans_out, use_defs)
char pic_file[];
int use_defs;  /* if 1 then use default valuse */
int speed1,pause1,num_loops,trans_in,trans_out;
{
if (suffix_in(pic_file, ".GIF"))
	{
	if (use_defs)
		play_gif(pic_file,(double)DEFAULT_PAUSE, DEFAULT_IN_TRANS,
	    	DEFAULT_OUT_TRANS);
	else
		play_gif(pic_file,(double)pause1,trans_in,trans_out);
	}
else if (suffix_in(pic_file, ".FLI"))
	{
	/* notice_keys=1; */ 
	if (use_defs)
        	play_fli(pic_file,DEFAULT_SPEED,(double)DEFAULT_PAUSE,
		INFINITE_LOOP,DEFAULT_IN_TRANS,DEFAULT_OUT_TRANS);
	else
        	play_fli(pic_file,speed1,(double)pause1,num_loops, trans_in, trans_out);
	}
else return(0);
return(1);
}




load_frame1(name,screen,hdware_update,close_f)
Video_form *screen;
char *name;
int hdware_update;
int close_f;  /* if 1 then close the file after loading */
{
if (file_is_loaded) close_file();
if ((loaded_file_fd = read_fli_head(name, &fh)) == 0)
	return(0);
file_is_loaded=1;
if (!read_next_frame(name,loaded_file_fd,screen,hdware_update))	/* read in frame 0 */
	goto END;

frame1off = jseek(loaded_file_fd, 0L, 1/*from current position*/);
if (jseek(loaded_file_fd, frame1off, 0) == -1) /* seek to frame 1 */
	goto END;	

set_frame_val(1);
if (close_f) close_file();
return(1);  /* a good load */

END:;
close_file();
return(0);
}



advance_frame(screen,hdware_update) 
Video_form *screen;
int hdware_update;
{
/*notice_keys=0; */
if (cur_frame_num < fh.frame_count)
	{
	if (!read_next_frame(global_file_name,loaded_file_fd,screen,
							hdware_update))
		return(0);
	set_frame_val(cur_frame_num+1);
	}
else   /* loops around */
	{
	set_frame_val(1);
	read_next_frame(global_file_name,loaded_file_fd,screen,hdware_update);
	jseek(loaded_file_fd, frame1off, 0);  /* seek to frame 1 */
	}
return(1);
}


goto_frame(old_val,new_val)
int old_val;
int new_val;
{
int i;
int val;

if (!file_is_loaded) return;
if (new_val < 0 || new_val+1==cur_frame_num) return;

if (new_val==old_val+1)  /* optimize--case where its a single advance */
	{
	advance_frame(&vf,1);
	return;
	}
new_val++;

copy_form(&vf, &alt_vf);
#ifdef OLDWAY
copy_cmap(vf.cmap,alt_vf.cmap);
copy_structure(vf.p, alt_vf.p, SCREEN_SIZE);
#endif OLDWAY

if (new_val > cur_frame_num)
	{
	val=new_val - cur_frame_num;
	for (i=1; i<= val; i++)
		advance_frame(&alt_vf,0);
/*	advance_frame(&alt_vf); */
	wait_vblank(); 
	copy_form(&alt_vf,&vf);
	see_cmap();
	}
else   /* (new_val < cur_frame_num) so go backwards */
	{
	val=fh.speed;
	close_file();
	load_frame1(global_file_name,&alt_vf,0,0);

	for (i=1; i< new_val; i++)
		advance_frame(&alt_vf,0);
	wait_vblank();
	copy_form(&alt_vf, &vf);
	see_cmap();
#ifdef OLDWAY
	copy_structure(alt_vf.p,vf.p, SCREEN_SIZE);
#endif OLDWAY
	fh.speed=val;
	}
}



prev_frame()
{
int new_val;
int old_val;

if (!file_is_loaded) return;
old_val=frame_val;
if (frame_val-1 < 0)
	new_val=fh.frame_count-1;
else
	new_val=frame_val-1;
hide_mp();
goto_frame(old_val,new_val);
draw_mp();
}



play_fli(pic_file,speed1,pause1,num_loops,trans_in,trans_out)
char pic_file[];
int speed1,num_loops,trans_in,trans_out;
double pause1;
{
long  start_time, cur_time;

user_defined_speed=speed1; 
if (trans_in==R_FADEIN) transit_from_white(pic_file,DEFAULT_TRANSIT_CYCLE,2);

view_fli_loop(pic_file, &vf,num_loops); 
if (pause1)
	{
	time(&start_time);
	cur_time=start_time;
        is_gif=0;
	while ( difftime(cur_time, start_time)  < pause1)  
		{
		if (!break_key()) break;
		time(&cur_time);
		}
	}
if (trans_out==R_FADEOUT) transit_to_white(DEFAULT_TRANSIT_CYCLE,2); 
}






transit_from_white(file_name, cycles, wait_time)
char *file_name;
int cycles;
int wait_time;
{
UBYTE  *d, *s;
int i, k;

d=vf.cmap;
for (i=0; i < COLORS * 3; i++) *d++ = 63;  /* set screen to white */
wait_sync();
jset_colors(0, COLORS, vf.cmap);

load_frame1(file_name,&alt_vf,0,1);
copy_structure(alt_vf.p,vf.p, SCREEN_SIZE);  /* copy picture to screen */

for (k=cycles; k >= 1; k--)
	{
	s=vf.cmap;   /* start map & map that goes to screen each cycle */
	d=alt_cmap;  /* destination of color transition */
	for (i=0; i < COLORS * 3; i++, s++, d++) *s = *s - (*s - *d )/k;
	delay(wait_time);	 
	wait_sync();
	jset_colors(0, COLORS, vf.cmap);
	}
}


transit_to_white(cycle,wait_time)
/* take the vf.cmp and graduate to white (= 63,63,63) */
int wait_time; /* # of milliseconds to wait between transitions */
int cycle;     /* the number of cycles over which to make the transition */
{
UBYTE *d;
int i,k;

copy_cmap(vf.cmap,alt_cmap);

for (k=cycle; k >= 1; k--)
	{
	d=alt_cmap;
	for (i=0; i < COLORS * 3; i++) *d++ = *d + (63 - *d )/k; 	
	delay(wait_time);
	wait_sync();
	jset_colors(0, COLORS, alt_cmap);
	}
}



/*
beep()
{
sound(70);
delay(1500);
nosound();
}
*/


close_file()
{
file_is_loaded=0;
jclose(loaded_file_fd);
}
