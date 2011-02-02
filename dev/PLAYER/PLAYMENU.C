/* playmenu.c */

#include "player.h"
#include "softmenu.h"

static SHORT jiffies;
static Qslider speed_sl = QSL_INIT1(0, 120, &jiffies, 0, NULL, leftright_arrs);


static void see_speed_sl(Button *b)
{
	jiffies = millisec_to_jiffies(pcb.speed);
	see_qslider(b);
}
static void feel_speed_sl(Button *b)
{
SHORT ojiffies;

	ojiffies = jiffies = millisec_to_jiffies(pcb.speed);
	feel_qslider(b);
	if(jiffies != ojiffies)
		pcb.speed = jiffies_to_millisec(jiffies);
}

static Button pla_spdsl_sel = MB_INIT1(
	NONEXT,
	NOCHILD,
	148,11,90,19,
	&speed_sl,
	see_speed_sl,
	feel_speed_sl,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);
static Button pla_spdtag_sel = MB_INIT1(
	&pla_spdsl_sel,
	NOCHILD,
	65,9,14,20,
	NODATA, /* "Play speed", */
	black_ctext,
	NOFEEL,
	NOOPT,
	NOGROUP,0,
	NOKEY,
	0
	);
static Button pla_tslider_sel = MB_INIT1(
	&pla_spdtag_sel,
	&timeslider_sel,
	0,0,49,3,
	NODATA,
	hang_children,
	NOFEEL,
	NOOPT,
	&playfli_data,0,
	NOKEY,
	0
	);

static Button pla_title_sel = MB_INIT1(
	&pla_tslider_sel,
	NOCHILD,
	43,11,4,3,
	NODATA, /* "Frames", */
	see_titlebar,
	mb_clipmove_menu,
	mb_menu_to_bottom,
	NOGROUP,0,
	NOKEY,
	0
	);

Menuhdr player_menu = {
	{244,34,0,0},	/* width, height, x, y */
	0,   				/* id */
	PANELMENU,			/* type */
	&pla_title_sel,		/* buttons */
	SCREEN_FONT, 		/* font */
	SCREEN_CURSOR,		/* cursor */
	seebg_white, 		/* seebg */
	NULL,				/* dodata */
	NULL,				/* domenu */
	(MBPEN|MBRIGHT|KEYHIT), /* ioflags */
};

static Smu_button_list pla_smblist[] = {
	{ "speed", &pla_spdtag_sel },
	{ "title", &pla_title_sel },
};

Errcode load_play_panel(void **ss)
{
	return(soft_buttons("play_panel", pla_smblist, Array_els(pla_smblist),ss));
}

Boolean check_lock_key()
/* returns 0 if nothing done 
   returns > 0 if key eaten 1 if no delay 2 if delayed by requestor */

{
SHORT inkey;

	if(!ISDOWN(KBCTRL) 
		|| ((inkey = icb.inkey) == CTRL_C))
	{
		if(pcb.lock_key)
			return(1);
		return(0);
	}

	if(pcb.lock_key == 0)
	{
		if((UBYTE)inkey < 0x1A) 
		{
			if(soft_yes_no_box("!%c", "play_lock", '@'+(UBYTE)inkey))
				pcb.lock_key = inkey;
			return(2);
		}
	}
	else if(pcb.lock_key == inkey)
	{
		soft_continu_box("play_unlock");
		pcb.lock_key = 0;
		return(2);
	}
	return(1);
}

static Boolean do_seek_keys()
{
	switch(icb.inkey)
	{
		case LARROW:
			pla_seek_frame(pcb.frame_ix - 1);
			goto key_seek;
		case RARROW:
			pla_seek_frame(pcb.frame_ix + 1);
			goto key_seek;
		case HOMEKEY:
		case UARROW:
			pla_seek_first();
			goto key_seek;
		case ENDKEY:
			pla_seek_last();
		key_seek:
			return(TRUE);
		default:
			return(FALSE);
	}
}
static Boolean wait_in_pause()
{
Boolean ret;

	for(;;)
	{
		wait_input(KEYHIT);
		switch((UBYTE)icb.inkey)
		{
			case ((UBYTE)CTRL_C):
				if(icb.inkey != CTRL_C)
					continue;
			case ESCKEY:
				goto reuse_out;
			case ' ':
				goto ok_out;
			default:
				if(icb.inkey == DARROW)
					goto ok_out;
				do_seek_keys();
				continue;
		}
	}

reuse_out:
	reuse_input();
	ret = TRUE;
	goto out;
ok_out:
	ret = FALSE;
out:
	pcb.cktime = pj_clock_1000(); /* reset counter */
	return(ret);
}
Boolean check_script_keys()
{
int ret;
int speed;
SHORT inkey;

	inkey = icb.inkey;

	if(pcb.script_mode)
	{
		if((ret = check_lock_key()) == 2)
			goto check_speed;
		if(ret == 1)
			goto eaten;
	}

	switch(toupper((UBYTE)inkey))
	{
		case ' ':
			goto enter_pause_mode;
		case '+':
			speed = 1;
			goto add_speed;
		case '-':
			speed = -1;
		add_speed:
			speed += millisec_to_jiffies(pcb.speed);
			goto new_speed;
		case 0:
			switch(inkey)
			{
				case FKEY1:
					speed = 0;
					goto new_speed;
				case FKEY2:
					speed = 3;
					goto new_speed;
				case FKEY3:
					speed = 6;
					goto new_speed;
				case FKEY4:
					speed = 9;
					goto new_speed;
				case FKEY5:
					speed = 12;
					goto new_speed;
				case FKEY6:
					speed = 18;
					goto new_speed;
				case FKEY7:
					speed = 24;
					goto new_speed;
				case FKEY8:
					speed = 36;
					goto new_speed;
				case FKEY9:
					speed = 48;
					goto new_speed;
				case FKEY10:
					pcb.speed = pcb.flif.hdr.speed;
					goto set_new_speed;
				case DARROW:
					goto enter_pause_mode;
				default:
					if(do_seek_keys())
						goto enter_pause_mode;
					goto not_eaten;
			}
		new_speed:
			if(speed < 0)
				speed = 0;
			pcb.speed = jiffies_to_millisec(speed);
		set_new_speed:
			draw_buttontop(&pla_spdsl_sel);
			goto check_speed;
		default:
			goto not_eaten;
	}

enter_pause_mode:

	if(!pcb.script_mode)
		goto not_eaten;
	if(wait_in_pause())
		goto not_eaten;

check_speed: /* a long delay or a change in speed will make this neccessary
			  * so things appear to go as expected */

	if((pj_clock_1000() - pcb.cktime) > pcb.speed)
		pcb.cktime = pj_clock_1000();
eaten:
	return(TRUE);
not_eaten:
	return(FALSE);
}
static Boolean rclick_on_screen()
{
	return((JSTHIT(MBRIGHT)
			&& ((pcb.dcel && pcb.dcel->type == RT_WINDOW
						  && curson_wndo((Wndo *)pcb.dcel))
				|| curson_wndo(&vb.screen->wndo))));
}
static Boolean check_toggle_menu(void)
{
	if(rclick_on_screen())
	{
		toggle_menu();
		return(TRUE);
	}
	else
		return(FALSE);
}
Boolean player_do_keys()
{
	if(!JSTHIT(KEYHIT))
		return(check_toggle_menu());

	switch(tolower((UBYTE)icb.inkey))
	{
		case ESCKEY:
		case 'q':
			return_to_main(PRET_QUIT);
			goto eaten;
		case ' ':
			toggle_menu();
			goto eaten;
		case 0:
			if(icb.inkey == DARROW)
				play_fli();
			if(!do_seek_keys())
				break;
			update_time_sel(&pla_tslider_sel);
			goto eaten;
	}
	return(check_script_keys());
eaten:
	return(TRUE);
}
