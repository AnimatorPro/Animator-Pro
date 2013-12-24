/* stringq.c - Implement a button that lets user edit a single line
   of text */

#include "formatf.h"
#include "memory.h"
#include "menus.h"
#include "input.h"
#include "rastext.h"

#define SDETAIL	SBLACK
#define SBLOCK	SWHITE

void init_stq_string();

typedef struct sqwork {
	Button *sqb;         /* the button */
	Stringq *sq;         /* the buttons stringq */
	Vfont *f;         /* font */
	Pixel sblock;       /* block color */
	Pixel sdetail;      /* detail color */
	SHORT textx, texty;  /* buttton relative text x and y values */ 
	SHORT spwidth;		 /* width of a "space" character */
	SHORT curson;		 /* xor cursor is on */
	Clipbox cb;
} Sqwork;

static void init_sqwork(Button *sqb, Sqwork *sw)
{
Wscreen *s = sqb->root->w.W_screen;
Stringq *sq;

	sw->sqb = sqb;
	sw->sq = sq = sqb->datme;
	sw->f = s->mufont;
	sw->sblock = s->SWHITE; 
	sw->sdetail = s->SBLACK;
	sw->spwidth = fchar_spacing(sw->f,"m");
	sw->textx = sw->spwidth/3 - 1;
	sw->texty = font_ycent_oset(sw->f,sqb->height) - 1;
	sw->curson = 0;
	mb_make_iclip(sqb,&sw->cb);
}

static void erase_tail(Sqwork *qw)
/* erases last part or drawn part of stringq should handle proportional text */
{
register Stringq *sq = qw->sq;
Button *sqb = qw->sqb;
register SHORT len;
char *strend;
SHORT tailwidth, textend;

	tailwidth = widest_char(qw->f) * 2;
	strend = sq->string + sq->dpos;
	len = strlen(strend);
	if (len >= sq->dcount)
		len = sq->dcount-1;
	if(len)
		--len;

	textend = qw->textx + fnstring_width(qw->f,strend,len);

	pj_set_rect(&qw->cb,qw->sblock,textend,
			 qw->texty, tailwidth, tallest_char(qw->f)); 
}

static void draw_stringq(Sqwork *qw)
{
char *string;
register Stringq *sq = qw->sq;

	string = sq->string+sq->dpos;
	pj_set_rast(&qw->cb, qw->sblock);
	gftext(&qw->cb,qw->f,string,qw->textx,qw->texty, 
		   qw->sdetail,TM_MASK1,qw->sblock);
	qw->curson = 0;
}


static void inactive_cursor(Sqwork *qw,Pixel color)
{
register Stringq *sq = qw->sq;
register Button *m = qw->sqb;

	pj_set_vline(&qw->cb, color, 
			qw->textx + fnstring_width(qw->f,sq->string + sq->dpos,
											 sq->cpos - sq->dpos),
			qw->texty,tallest_char(qw->f));
}
static void see_stringq(Sqwork *qw, Pixel textcol)
{
	draw_stringq(qw);
	inactive_cursor(qw,textcol);
}

static int stringq_xor_cursor(Sqwork *qw)
{
register Button *m = qw->sqb;
register Stringq *sq = qw->sq;
SHORT cwidth;

	if(sq->string[sq->cpos])
		cwidth = fchar_spacing(qw->f,(sq->string + sq->cpos));
	else
		cwidth = qw->spwidth;

	pj_xor_rect(m->root,qw->sblock^qw->sdetail, 
			 qw->cb.x + qw->textx + fnstring_width(qw->f,sq->string + sq->dpos,
											 	   sq->cpos - sq->dpos),
			 m->y+2, cwidth, m->height-4); 

	qw->curson = !(qw->curson);
	return(0);
}
static stringq_cursor_off(Sqwork *qw)
{
	if(qw->curson)
		stringq_xor_cursor(qw);
}
static void cpos_to_cursor(Sqwork *qw)
{
Stringq *sq;
int textx;

	sq = qw->sq;
	sq->cpos = sq->dpos;
	textx = qw->cb.x + qw->textx;

	for(;;) /* find character under cursor */
	{
#ifdef LATER
		if(sq->cpos >= (sq->dpos + sq->dcount - 1))
			break;
#endif /* LATER */
		textx += fchar_spacing(qw->f,sq->string + sq->cpos);
		if(textx > icb.mx)
			break;
		++sq->cpos;
	}
	if (sq->cpos > sq->ccount)
		sq->cpos = sq->ccount;
}

static Boolean right_scroll(Sqwork *qw)
{
Stringq *sq = qw->sq;
Button *b = qw->sqb;
Boolean ret = FALSE;

/* Might have to do this more than once if character scrolling off left edge
 * is narrower than new character */
while (fnstring_width(qw->f, sq->string+sq->dpos,
	sq->cpos - sq->dpos + 1) >= b->width-2*MB_IBORDER-qw->textx)
	{
	sq->dpos += 1;
	ret = TRUE;
	}
return(ret);
}

static int feel_stringq(Sqwork *qw, SHORT textcol,SHORT blockcol)

/* returns 0 if unaltered and non enter key hit
 * STQ_ENTER is set if enter is exit key, STQ_ALTERED is set if altered */
{
Stringq *sq;
char *string;
Button *b;
SHORT count;
int ret;
Wiostate ios;
char lastkey, thiskey;
char first = TRUE;

	save_wiostate(&ios);
	load_wndo_iostate((Wndo *)get_button_wndo(qw->sqb));
	b = qw->sqb;
	ret = 0;
	sq = qw->sq;
	string = sq->string;
	if(sq->undo != NULL)
		strcpy(sq->undo, string);

	if( icb.iowndo != NULL
	   && (Menuwndo *)(icb.iowndo) == get_button_wndo(b)
	   && ptin_button(b,icb.mx,icb.my))
	{
		cpos_to_cursor(qw);
	}

	inactive_cursor(qw,blockcol);
	erase_tail(qw);
	draw_stringq(qw);
	thiskey = 0;

	for (;;)
	{
		anim_wait_input(KEYHIT|MBPEN|MBRIGHT,0,20,stringq_xor_cursor,qw);

		if(JSTHIT(KEYHIT))
		{
			stringq_cursor_off(qw); /* turn off cursor */
			lastkey = thiskey;
			thiskey = icb.inkey;
			switch(thiskey)
			{
				case 0:
				{
					switch (icb.inkey)
					{
						case LARROW:
						{
							if(sq->cpos > 0)
							{
								sq->cpos -= 1;
								if(sq->cpos - sq->dpos < 0)
								{
									sq->dpos -= 1;
									goto UNALT_REDRAWIT;
								}
							}
							goto NOT_FIRST;
						}
						case RARROW:
						{
							if (sq->cpos < sq->ccount)
							{
								sq->cpos += 1;
								if (right_scroll(qw))
									goto UNALT_REDRAWIT;
							}
							goto NOT_FIRST;
						}
						case UARROW:
						case HOMEKEY:
							sq->cpos = sq->dpos = 0;
							goto UNALT_REDRAWIT;
						case DARROW:
						case ENDKEY:
							sq->cpos=sq->ccount;
							if (sq->ccount >= sq->dcount)
								sq->dpos = sq->ccount - sq->dcount + 1;
							goto UNALT_REDRAWIT;
						case DELKEY:	/* delete */
						{
							if (sq->ccount > 0)
							{
								if(ISDOWN(KBSHIFT))
									goto clear_string;
								count = sq->ccount - sq->cpos;
								if (count == 0)
								{
									sq->cpos -= 1;
									count += 1;
								}
								pj_copy_bytes(string+sq->cpos+1, 
											  string+sq->cpos,
										   	  count);
								sq->ccount -= 1;
								if (sq->cpos - sq->dpos < 0)
									sq->dpos -= 1;

								goto REDRAWIT;
							}
							break;
						}
					}
					goto NOT_FIRST;
				}
				case ESCKEY:
				clear_string:
				{
					if(string[0] == 0 && lastkey == ESCKEY)
					{
						reuse_input(); /* may have hit something else !! */
						ret |= STQ_ESCAPE;
						goto EXIT_STRING;
					}
					string[0] = 0;
					init_stq_string(sq);
					color_block1(qw->sqb,qw->sblock);
					goto NOT_FIRST;
				}
				case '\b':
				{
					if (sq->ccount > 0 && sq->cpos > 0)
					{
						pj_copy_bytes(string+sq->cpos, string+sq->cpos-1,
							sq->ccount - sq->cpos + 1);
						sq->cpos -= 1;
						sq->ccount -= 1;
						if (sq->cpos - sq->dpos < 0)
							sq->dpos -= 1;
						goto REDRAWIT;
					}
					goto NOT_FIRST;
				}
				case '\t':
				{
					mb_set_tabnext(b,b->group);
					ret |= STQ_TAB;
					if(b->group)
						reuse_input();
					goto EXIT_STRING;
				}
				case '\r': /* enter key */
				{
					ret |= STQ_ENTER;
					goto EXIT_STRING;
				}
				default: /* insert a character */
				{
					if (first)	/* if first character is normal clear buf */
					{
						string[0] = 0;
						init_stq_string(sq);
						color_block1(qw->sqb,qw->sblock);
					}
					if (sq->ccount < sq->bcount)
					{
						back_copy_mem(string+sq->cpos, string+sq->cpos+1,
							sq->ccount - sq->cpos+1);
						string[sq->cpos] = icb.inkey;
						sq->cpos += 1;
						sq->ccount += 1;
						right_scroll(qw);
						goto REDRAWIT;
					}
					goto NOT_FIRST;
				}
			}
	    REDRAWIT:
			{
				ret |= STQ_ALTERED;
		UNALT_REDRAWIT:
				erase_tail(qw);
				draw_stringq(qw);
			}
		}
		else /* if(JSTHIT(MBPEN|MBRIGHT)) */
		{
			if(ptin_button(b,icb.mx,icb.my))
			{
				stringq_cursor_off(qw);
				cpos_to_cursor(qw);
				goto NOT_FIRST;
			}
			else
			{
				reuse_input(); /* may have hit something else !! */
				goto EXIT_STRING;
			}
		}
NOT_FIRST:
	first = FALSE;
	}

EXIT_STRING:
	stringq_cursor_off(qw);
	inactive_cursor(qw,textcol);
	rest_wiostate(&ios);
	return(ret);
}

/********* external calls ******/


int feel_string_req(Button *b)
/* returns 0 if unaltered and non enter key hit
 * STQ_ENTER is set if enter is exit key, STQ_ALTERED is set if altered */
{
Sqwork qw;
Wscreen *s = b->root->w.W_screen;

	init_sqwork(b,&qw);
	return(feel_stringq(&qw,s->SDETAIL,s->SBLOCK));
}

void see_string_req(Button *b)
{
Sqwork qw;

	init_sqwork(b,&qw);
	color_block1(b,qw.sblock);
	mc_frame(b,MC_GREY);
	see_stringq(&qw,qw.sdetail);
}

void init_stq_string(Stringq *stq)
{
	stq->ccount = stq->cpos = strlen(stq->string);
	stq->dpos = 0;
	if(stq->cpos >= stq->dcount)
		stq->cpos = stq->dcount-1;
}
void set_stq_string(Stringq *stq, char *buf)
{
	stq->string = buf;
	init_stq_string(stq);
}

void stringq_revert_to_undo(Stringq *stq)
{
	strcpy(stq->string, stq->undo);
	init_stq_string(stq);
}

void undo_stringq(Button *m,Button *stq_item)
{
Stringq *stq;

	stq = (Stringq *)stq_item->datme;
	hilight(m);
	stringq_revert_to_undo(stq);
	draw_buttontop(stq_item);
	wait_a_jiffy(2);
	draw_buttontop(m);
}

void setf_stringq(Button *sqb,int drawit,char *fmt,...)
/* Sets a stringq button buffer with value in fmt and args like sprintf 
 * puts old value in undo buffer also clips string to length in stq->bcount */
{
Formatarg fa;
register Stringq *stq = sqb->datme;

	start_formatarg(fa,fmt);
	if(stq->undo != NULL)
		strcpy(stq->undo, stq->string);
	fa_sprintf(stq->string,stq->bcount+1,&fa);
	init_stq_string(stq);
	if(drawit)
		draw_buttontop(sqb);
	end_formatarg(fa);
}

/***** some stuff for a numq *****/

static void init_numq_stq(Numq *nq, Stringq *sq)
{
	clear_struct(sq);
	sq->dcount = 6;
	sq->bcount = 30;
}
static void set_nqbuf(Button *b, SHORT val)
{
	setf_stringq(b,FALSE,"%d",val);
}
void see_numq(Button *b)
{
Numq *nq;
char cbuf[32];
Stringq sq;

	nq = b->datme;
	init_numq_stq(nq,&sq);
	sq.string = cbuf;
	b->datme = &sq;
	set_nqbuf(b,*((SHORT *)nq->val));
	see_string_req(b);
	b->datme = nq;
}
Boolean feel_numq(Button *b)
{
Numq *nq;
Boolean hit_enter;
Stringq sq;
char cbuf_b[32];
char cbuf_a[32];
LONG lval;

	nq = b->datme;
	init_numq_stq(nq,&sq);
	sq.string = cbuf_a;
	b->datme = &sq;
	set_nqbuf(b,*((SHORT *)nq->val));
	hit_enter = feel_string_req(b);

	lval = atol(cbuf_a);
	if(lval != (SHORT)lval) /* overflow !! */
		lval = 0;
	sq.string = cbuf_b; /* switch strings */
	set_nqbuf(b,(SHORT)lval);
	if(strcmp(cbuf_a, cbuf_b)) /* compare set string with input string */
	{
		hit_enter = 0;
		lval = atol(cbuf_b);
	}
	*((SHORT *)nq->val) = lval;
	see_string_req(b);
	b->datme = nq;
	return(hit_enter);
}

