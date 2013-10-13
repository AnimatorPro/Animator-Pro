/* vbat.c  -- batch file processing for stand alone player */

#include <stdio.h>
#include <ctype.h> 
#include "jimk.h"
#include "prjctor.h"
#include "vbat.str"

#define MAX_SYM_CHAR 32
#define CTRL_Z  0x1A
#define MAX_FILE_CHAR 85
#define MAX_CONTINU_CHAR 46

char leftover[MAX_SYM_CHAR+1];
int linecount;
int symline; /* line that a symbol was found on */
FILE *bat_file;
char bat_file_name[MAX_FILE_CHAR];

int sym, sym_ptr;
int g_num;  /* global number */
char symchars[MAX_SYM_CHAR+1];
char public_buffer[121];

int file_loop;
int file_cur_loop;

char theres_more;  
char file_loop_found;
char in_loop_cycle;
char stop_bat;
char bat_closed;
char exit_word;  /* true if we are to exit after playing batfile */
char just_checkin_bat;
char was_eof;

#define COMMENT_CHAR  ';'
#define EOF_SYM	-1
#define UNDEFINED 0
#define FLI_FNAME 1
#define GIF_FNAME 2
#define NAME_SYM  3
#define INT_NUM   4

#define RESV_OFFSET 20  /* offset of reserved words */

#define R_EXIT   20
#define R_LINK   21
#define R_LOOP   22
#define R_NOEXIT 23
#define R_NOTICE_KEYS 24
#define R_DONT_NOTICE_KEYS 25
char *resv_words[]=
	{
	"EXITTODOS", /* exitToDos ie when finished */
	"LINK",
	"LOOP",
	"EXITTOPLAYER",/* exitToPlayer */
#ifdef OLD_FEATURE
	"KEYSON",     /* keysOn */
	"KEYSOFF", /* keysOff */
#endif OLD_FEATURE
	};
#define MAX_RESV_WORD 3  /* was 5 */


#define RESV_LINE_OFFSET 30  /* offset of reserved words */
#define R_FADEIN    30  /* reserved words */  
#define R_FADEOUT   31
#define R_CUT       32

char *resv_line_words[]=
	{
	"FADEIN",
	"FADEOUT",
	"CUT",
	};
#define MAX_RESV_LINE_WORD 2


#define SW_OFFSET 10
#define SW_SPEED  10  /* switches */
#define SW_PAUSE  11
#define SW_LOOP   12
#define SW_TRANS  13

char *switches[]=
	{
	"-S",  /* speed      */
	"-P",  /* pause      */
	"-L",  /* loop       */
	"-T",  /* transition */
	};
#define MAX_SWITCHES 3

#define IS_SWITCH(x)     (((x) >= SW_OFFSET) && ((x) <= SW_OFFSET+MAX_SWITCHES))
#define IS_RESV_WORD(x)  (((x) >= RESV_OFFSET) && ((x) <= RESV_OFFSET+MAX_RESV_WORD))
#define IS_RESV_LINE_WORD(x) \
  (((x) >= RESV_LINE_OFFSET) && ((x) <= RESV_LINE_OFFSET+MAX_RESV_LINE_WORD))

/*
#define DEFAULT_SPEED 0
#define DEFAULT_PAUSE 0
#define DEFAULT_LOOP  1
#define DEFAULT_IN_TRANS   R_CUT
#define DEFAULT_OUT_TRANS  R_CUT
*/


my_isspace(c)
char c;
{
if (c=='\n' || c== '\r') 
	{
	if (!in_loop_cycle) linecount++;
	return(1);
	}
return( c==' ' ||  c=='\f' || c=='\t' || c=='\v' || c==','); /* note comma ignored */
}



int my_getc()
/* filters out comments */
{
int c = fgetc(bat_file);

if (c==COMMENT_CHAR) 
	{
	while ( (c=fgetc(bat_file))  &&
		 c!=EOF && c!=CTRL_Z && c !='\n' && c!='\r' ) ;
	}
return(c);
}





getsym()
{
int c;
symchars[0]='\0';
sym_ptr=0;
g_num=0;

if (bat_closed) return(sym=EOF_SYM);
if (theres_more)
	{
        strcpy(symchars,leftover);
	sym_ptr=strlen(symchars);
	theres_more=0;
	goto PT1;
	}

if (was_eof) return(sym=EOF_SYM);

while ( my_isspace(c=my_getc()) );  /* eat spaces */
ungetc(c,bat_file);  
symline = -1;
while ((c!=EOF) && !my_isspace(c=my_getc(bat_file)) && (sym_ptr < MAX_SYM_CHAR))
	{
	if (symline < 0) symline=linecount;
	if (c!=EOF) 
		symchars[sym_ptr++]=toupper(c);	/* collect characters */
	}

if (c==EOF || c==CTRL_Z) /* cause it to wait out one cycle */
	was_eof=1;

symchars[sym_ptr]='\0';			/* end string */

if ( sym=is_switch()) 		return(sym);
PT1:;
if ( symchars[0]=='\0')	        return(sym=UNDEFINED);
if ( sym=is_resv_word()) 	return(sym);
if ( sym=is_resv_line_word()) 	return(sym);
if ( sym=is_number(symchars)) 	return(sym); /* allow for neg nums ? */
/*else*/			return(sym=NAME_SYM);
}




is_resv_word() /* test whether its in tablek of reserved words */
{
int i=0;
while ((i<= MAX_RESV_WORD) && strcmp(symchars,resv_words[i])) i++;
if (i > MAX_RESV_WORD) return(0);
else return(i+RESV_OFFSET);
}


is_resv_line_word() /* test whether its in tablek of reserved words */
{
int i=0;
while ((i<= MAX_RESV_LINE_WORD) && strcmp(symchars,resv_line_words[i])) i++;
if (i > MAX_RESV_LINE_WORD) return(0);
else return(i+RESV_LINE_OFFSET);
}



is_switch()  /* test whether its in table of reserved switches */
{
int i=0;
int l1;
int count;
int pos,k;

if (symchars[0]!='-') return(0);
while ((i<= MAX_SWITCHES) && (strstr(symchars,switches[i])==NULL)) i++;
if (i > MAX_SWITCHES) return(0);
else 
	{
	if (sym_ptr - strlen(switches[i]))   /* sym_ptr is len of symchars */
		{
		pos= strlen(switches[i]);
		k=0;
		theres_more=1;		
		while (leftover[k++]=symchars[pos++]);
		}
	else theres_more=0;
	return(i+SW_OFFSET);
	}
}



is_number(s)
char s[];
{
int ival=0;
int sign=1;
int i=0;

if (s[i]=='-') 
	{
	sign=-1;
	i++;
	}
else if(s[i]=='+')
	i++;

while (s[i])
	{
	if (isdigit(s[i]))
		ival=ival*10+s[i++]-'0';
	else 	return(0);
	}
g_num=sign*ival;
return(INT_NUM);
}


do_trans_parm(in_trans, out_trans)
int *in_trans,  *out_trans;
{
int trans_in, trans_out;
int itran_set, otran_set;

/**in_trans = *out_trans = R_CUT; */
trans_in  = trans_out  = R_CUT;
itran_set = otran_set  = 0;

LOOP17:
getsym();
if (sym==R_FADEIN)
	{
	if (!itran_set) 
		{
		itran_set=1;
		trans_in=R_FADEIN;
		goto LOOP17;
		}
	else bat_err(vbat_112 /* "Can't repeat transition parameter" */,symline);
	}
else if (sym==R_FADEOUT)
	{
	if (!otran_set) 
		{
		otran_set=1;
		trans_out=R_FADEOUT;
		goto LOOP17;
		}
	else bat_err(vbat_112 /* "Can't repeat transition parameter" */,symline);
	}
else if (sym==R_CUT)
	{
	goto LOOP17;
	}
/* else bat_err("Unknown parameter to transition option",symline); */
*in_trans  = trans_in;
*out_trans = trans_out;
}





do_param_line()
{
char buf[MAX_FILE_CHAR];
char picfile[MAX_FILE_CHAR];
int speed1,pause1,loop1;
int trans_in, trans_out;
int olinecount;

speed1 =	DEFAULT_SPEED;
pause1 =	DEFAULT_PAUSE;
loop1  =	DEFAULT_LOOP;
trans_in  =	DEFAULT_IN_TRANS;
trans_out =	DEFAULT_OUT_TRANS;

strcpy(picfile,symchars);

olinecount=linecount; /* remember the line the filename was on */
getsym();
while ( sym!=EOF_SYM  && !stop_bat && IS_SWITCH(sym))
	switch(sym)
	{
	case SW_SPEED:  
		getsym();
		if (sym==INT_NUM)
			{
			speed1=g_num;
			if (speed1<0 || speed1>SL_MAX_SPEED)
				speed1=DEFAULT_SPEED;
			getsym();			
			}
		else bat_err(vbat_113 /* "Expecting int parameter for speed" */,symline);
		break;
	case SW_PAUSE:  
		getsym();
		if (sym==INT_NUM)
			{
			pause1=g_num;
			if (pause1<0 || pause1>MAX_PAUSE)
				pause1=DEFAULT_PAUSE;
			getsym();			
			}
		else bat_err(vbat_114 /* "Expecting int parameter for pause" */,symline);
		break;
	case SW_LOOP:   
		getsym();
		if (sym==INT_NUM)
			{
			loop1=g_num;
			if (loop1<0 || loop1>INFINITE_LOOP)
				loop1=DEFAULT_LOOP;
			getsym();			
			}
		else bat_err(vbat_130 /* "Expecting int parameter for loop" */,symline);
		break;
	case SW_TRANS:  
		do_trans_parm(&trans_in, &trans_out);
		break;
	default: break;
	}

if (stop_bat) return(0);
if (access(picfile,0)==0) /* file exists */
	{
	if (!just_checkin_bat) 
	  play_pic(picfile,speed1,pause1,loop1,trans_in,trans_out,0);
	return(1);
	}
else
	{
	sprintf(buf,"%s %s",vbat_127 /* "Trouble opening " */,picfile);
	bat_err(buf,olinecount);
	stop_bat=1;
	return(0);
	}
}



do_loop()
{
if (stop_bat || !in_loop_cycle || just_checkin_bat) return(0);
if ( (file_loop!=INFINITE_LOOP) && (++file_cur_loop >= file_loop) )
	return(in_loop_cycle=0);
else 
	{
	was_eof=0;  
	fseek(bat_file,0L,SEEK_SET); /* goto beginning of file */
	return(1);
	}
}




bat_err(err_descr,linenum)
/* pass linenum <0 for just putting out one line */
char *err_descr;
int linenum;
{
char *bufs[7];
char buf0[MAX_CONTINU_CHAR+7];
char buf3[MAX_CONTINU_CHAR+7];
char buf4[MAX_CONTINU_CHAR+7];
char buf5[MAX_CONTINU_CHAR+7];
char *bufp;
char *bat_line();
int err1;

if (strlen(err_descr) >= MAX_CONTINU_CHAR )
	err_descr[MAX_CONTINU_CHAR] = '\0';
if (linenum+1 > 0)
	{
	sprintf(buf0,vbat_118 /* "Error near line %d in %s" */,linenum+1,bat_file_name);
	bufs[0] = buf0;
	bufs[1] = err_descr;
	bufs[2] = "  ";

	if (linenum == 0)		
		{
		sprintf(buf3,"[%d] %s",linenum+1,bat_line(linenum, &err1) );
		bufs[3] = buf3;
		sprintf(buf4,"[%d] %s",linenum+2,bat_line(linenum+1, &err1) );
		bufs[4] = buf4;
		bufs[5] = NULL; 
		}
	else
		{
		sprintf(buf3,"[%d] %s",linenum,bat_line(linenum-1, &err1) );
		bufs[3] = buf3;
		sprintf(buf4,"[%d] %s",linenum+1,bat_line(linenum, &err1) );
		bufs[4] = buf4;
		bufp=bat_line(linenum+1, &err1) ;
		if (err1)  /* in case error is on last line of file */
			bufs[5]=NULL;
		else
			{
			sprintf(buf5,"[%d] %s",linenum+2,bufp );
			bufs[5] = buf5;
			bufs[6] = NULL; 
			}
		}
	continu_box(bufs);
	}
else
	continu_line(err_descr);
bat_fclose();
stop_bat=1;
}



char *bat_line(line_num, err1)
int line_num;
int *err1;
{
int k;
int l;

fseek(bat_file,0L,SEEK_SET); /* goto beginning of file */
for (k=0; k < line_num+1; k++)
	if (fgets(public_buffer,120,bat_file)==NULL) goto LABEL1;

l= strlen(public_buffer);

if (l >= MAX_CONTINU_CHAR)
	{
	public_buffer[MAX_CONTINU_CHAR] = '\0';
	public_buffer[MAX_CONTINU_CHAR-1] = ' ';
	}
else public_buffer[l-1]=' ';

*err1=0;
return(public_buffer);

LABEL1:;
*err1=1;
public_buffer[0]=' ';
public_buffer[1]='\0';
return(public_buffer);
}




bat_fopen(filename)
char *filename;
{
char buf[MAX_FILE_CHAR];
strcpy(bat_file_name,filename);

bat_init();
file_loop=1;
linecount=0;
if ( (bat_file=fopen(bat_file_name,"r"))==NULL)  /* open bat file */
	{
	sprintf(buf,"%s %s",vbat_127 /* "Trouble opening " */,bat_file_name);
	bat_err(buf,-1);
	bat_closed=1;
	}
}




bat_fclose()
{
if (!bat_closed)
	{
	bat_closed=1;
	was_eof=0;
	fclose(bat_file);
	}
}





batch_process(fname)
char *fname;
{
bat_fopen(fname);

REPLAY:;
if (stop_bat) return(0);

getsym();
while (sym!=EOF_SYM && !stop_bat)
	{
	if (IS_RESV_WORD(sym)) 
		{
		switch(sym)
			{
#ifdef OLD_FEATURE
			case R_NOTICE_KEYS: /* keys ON */
				getsym();
				notice_keys=1;
				break;
			case R_DONT_NOTICE_KEYS: /* keys OFF */
				getsym();
				notice_keys=0;
				break;
#endif OLD_FEATURE
			case R_NOEXIT:
				getsym();
				exit_word=0;
				break;
			case R_EXIT:
				getsym();
				exit_word=1;
				break;
			case R_LINK:
				getsym();
				if (sym==NAME_SYM || sym==INT_NUM) /* dos allows numbers as filenames */
					{
					if (access(symchars,0)==0) 
						{
						strcpy(fname,symchars);	
						bat_fclose();
						return(1);	
						}
					else bat_err(vbat_128 /* "Couldn't open linked-to file" */,symline);
					}
				else bat_err(vbat_129 /* "Expected filename here" */,symline);
				break;
			case R_LOOP:
				if (do_loop()) goto REPLAY;
		
				getsym();
				if (file_loop_found) 
					{
					linecount--; /* for showing error cond. */
					getsym(); /* get in param */
					break;
					}
				if (sym==INT_NUM)
					{
					file_loop=g_num;
					if (g_num<0 || g_num>INFINITE_LOOP)
					 	file_loop=DEFAULT_LOOP;
					file_cur_loop=0;
					file_loop_found=1;
					in_loop_cycle=1;
					if (do_loop()) goto REPLAY;

					getsym();			
					}
				else bat_err(vbat_130 /* "Expecting int parameter for loop" */,symline);
				break;
			default:  break;
			}
		}
	else if (sym==NAME_SYM)  /* it's a line with a filename and parameters*/
		do_param_line();
	else if (sym==UNDEFINED) getsym();
	else bat_err(vbat_131 /* "Error in processing batch file" */,linecount);  /* error ? */
	}

bat_fclose();
return(0);
}



bat_init()
{
theres_more=
file_cur_loop=
file_loop_found=
/* in_loop_cycle= */
stop_bat=
bat_closed=
was_eof = 0;
}




process_bat(fname)
char *fname;
{
int save_exit=exit_word;
char big_buffer[120];

strcpy(big_buffer, fname);

just_checkin_bat = 1;
while (batch_process(big_buffer));
if (!stop_bat)  /* if there were no errs then go on */
	{
	exit_word= save_exit;
	just_checkin_bat = 0;
	while (batch_process(fname));
	}
else exit_word= save_exit;
return(0);
}



/**********************************************

Filename | Type | Speed | Pause | Loop | Transition

Example script:

d:\vpaint\anim1.fli,4,2,0,0,0,0  <<<<<<<<< GIVEN
c:\pics\bug.gif,12,1,6,2,10
rudy.fli,10,2,2,7,1,15
loop 3
link h:next.fli

d:\vpaint\anim1.fli -s 4 -l 2
c:\pics\bug.gif,-sp 12 -pause 1  from_black to_white -color 10 -loop 3
rudy.fli -s10 -p2 -l2  -t1 , 15
loop 3
link h:next.txt

In the above script, the ANIM1.FLI will play at a speed of 4
jiffies, twice through and there will be no transitions in or
out.  Next will be the BUG.GIF picture which will fade in from
black in six seconds, display for 12 seconds, then fade to white
in 10 seconds.  Next, the RUDY.FLI will fade in from white in 7
seconds, loop 2 times at a speed of 10 jiffies, then fade to
black in 15 seconds. This entire script will cycle through 3
times, and then the NEXT.TXT script will be loaded and begun.

 **********************************************/
/* NOTE: different ASCII text editors treat eof differently
   some use ctrl Z & others don't.. So the best policy is to append 
   an extra blank line to the end of the batch file
*/

