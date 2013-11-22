/* stringq.c - Implement a button that lets user edit a single line
   of text */

#include "jimk.h"
#include "flicmenu.h"
#include "commonst.h"

#define SDETAIL	sblack
#define SBLOCK	swhite

static
erase_last(m, sq)
Flicmenu *m;
register Stringq *sq;
{
register WORD len;

len = strlen( sq->string+sq->dpos );
if (len >= sq->dcount)
	len = sq->dcount-1;
stext(cst_space, m->x+sq->pxoff+len*CH_WIDTH, m->y+sq->pyoff, SDETAIL,SBLOCK);
}

static
draw_stringq(m, sq)
Flicmenu *m;
register Stringq *sq;
{
register WORD len;
char *string;
char save;

string = sq->string+sq->dpos;
len = strlen(string);
if (len > sq->dcount)
	len = sq->dcount;
save = string[len];
string[len] = 0;
stext(string, m->x+sq->pxoff, m->y+sq->pyoff, SDETAIL,SBLOCK);
string[len] = save;
}


static
see_stringq(textcol, m)
WORD textcol;
register Flicmenu *m;
{
register Stringq *sq;
WORD count;


sq = (Stringq *)m->text;
draw_stringq(m, sq);
vline(m->x + sq->pxoff + (sq->cpos-sq->dpos)*CH_WIDTH, 
	m->y + 2, m->y + m->height - 2, textcol);
}

static
stringq_xor_cursor(m, sq)
register Flicmenu *m;
register Stringq *sq;
{
xorrop(SBLOCK^SDETAIL, m->x + sq->pxoff + (sq->cpos-sq->dpos)*CH_WIDTH,
	m->y+2, CH_WIDTH-1, m->height-4); 
}

static
feel_stringq(textcol, blockcol, m)
WORD textcol, blockcol;
Flicmenu *m;
{
Stringq *sq;
char *string;
WORD count;
WORD chars_right;
WORD pos;
WORD inside;

inside = 1;
sq = (Stringq *)m->text;
string = sq->string;
if (sq->undo)
	strcpy(sq->undo, string);
vline(m->x + sq->pxoff + sq->cpos*CH_WIDTH, 
	m->y + 2, m->y + m->height - 2, blockcol);
stringq_xor_cursor(m, sq);
for (;;)
	{
	wait_input();
	if (key_hit)
		{
		stringq_xor_cursor(m, sq);
		switch (key_in&0xff)
			{
			case 0:
				switch (key_in)
					{
					case LARROW:
						if (sq->cpos > 0)
							{
							sq->cpos-=1;
							if (sq->cpos - sq->dpos < 0)
								{
								sq->dpos -= 1;
								}
							}
						break;
					case RARROW:
						if (sq->cpos < sq->ccount)
							{
							sq->cpos+=1;
							if (sq->cpos - sq->dpos >= sq->dcount)
								{
								sq->dpos += 1;
								}
							}
						break;
					case UARROW:
						sq->cpos = sq->dpos = 0;
						break;
					case DARROW:
						sq->cpos=sq->ccount;
						if (sq->ccount >= sq->dcount)
							sq->dpos = sq->ccount - sq->dcount + 1;
						break;
					}
				break;
			case 0x1b:	/* escape */
				string[0] = 0;
				init_stq_string(sq);
				colrop(blockcol, m->x+1, m->y+1, m->width-2, m->height-2);
				break;
			case 0x7f:	/* delete */
				if (sq->ccount > 0)
					{
					count = sq->ccount - sq->cpos;
					if (count == 0)
						{
						sq->cpos -= 1;
						count += 1;
						}
					copy_bytes(string+sq->cpos+1, string+sq->cpos,
						count);
					sq->ccount -= 1;
					if (sq->cpos - sq->dpos < 0)
						sq->dpos -= 1;
					}
				break;
			case '\b':
				if (sq->ccount > 0 && sq->cpos > 0)
					{
					copy_bytes(string+sq->cpos, string+sq->cpos-1,
						sq->ccount - sq->cpos + 1);
					sq->cpos -= 1;
					sq->ccount -= 1;
					if (sq->cpos - sq->dpos < 0)
						sq->dpos -= 1;
					}
				break;
			case '\r':
				stringq_xor_cursor(m, sq);
				goto ACCEPTED_STRING;
			default: /* insert a character */
				if (sq->ccount < sq->bcount)
					{
					back_copy_bytes(string+sq->cpos, string+sq->cpos+1,
						sq->ccount - sq->cpos+1);
					string[sq->cpos] = key_in;
					sq->cpos += 1;
					sq->ccount += 1;
					if (sq->cpos - sq->dpos >= sq->dcount)
						sq->dpos += 1;
					}
				break;
			}
		/* next 2 lines aren't always necessary.  Code is shorter if
		   slower this way. */
		erase_last(m, sq);
		draw_stringq(m, sq);
		stringq_xor_cursor(m, sq);
		}
	else if (PJSTDN || RJSTDN)
		{
		if (in_menu(m))
			{
			stringq_xor_cursor(m, sq);
			sq->cpos = sq->dpos + (uzx-m->x-sq->pxoff)/CH_WIDTH;
			if (sq->cpos >= sq->dpos + sq->dcount)
				sq->cpos = sq->dpos + sq->dcount - 1;
			if (sq->cpos > sq->ccount)
				sq->cpos = sq->ccount;
			stringq_xor_cursor(m, sq);
			}
		else
			{
			reuse_input();
			inside = 0;
			goto ACCEPTED_STRING;
			}
		}
	}
ACCEPTED_STRING:
stringq_xor_cursor(m, sq);
vline(m->x + sq->pxoff + (sq->cpos-sq->dpos)*CH_WIDTH, 
	m->y + 2, m->y + m->height - 2, textcol);
return(inside);
}

feel_string_req(m)
Flicmenu *m;
{
return(feel_stringq(SDETAIL, SBLOCK, m));
}

see_string_req(m)
Flicmenu *m;
{
a_block(SBLOCK, m);
a_frame(sgrey, m);
see_stringq(SDETAIL, m);
}

init_stq_string(stq)
register Stringq *stq;
{
stq->ccount = stq->cpos = strlen(stq->string);
stq->dpos = 0;
if (stq->cpos >= stq->dcount)
	stq->cpos = stq->dcount-1;
}

undo_stringq(m, stq_item)
Flicmenu *m;
Flicmenu *stq_item;
{
Stringq *stq;

stq = (Stringq *)stq_item->text;
hilight(m);
exchange_buf(stq->string, stq->undo, stq->bcount);
init_stq_string(stq);
draw_sel(stq_item);
wait_a_jiffy(2);
draw_sel(m);
}

upd_stringq(name, stringq)
register char *name;
register Stringq *stringq;
{
if (stringq->undo != NULL)
	strcpy(stringq->undo, stringq->string);
strcpy(stringq->string, name);
init_stq_string(stringq);
}

