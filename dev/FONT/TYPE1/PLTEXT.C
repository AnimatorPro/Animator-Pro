 
/* 2D text module */

#include <math.h>
#include "jimk.h"
#include "menu.h"
#include "3ds.h"
#include "shape.h"
#include "rtndefs.h"
#include "scrn.h"
#include "object.h"
#include "data.h"
#include "gfx.h"
#include "trig.h"
#include "nimbus.h"

/* This module's functions */

extern void pltx_setup(void);
extern void do_pltx(int forced);
extern void select_font(void);
extern void rdy_txdia(void);
extern void tx_ok(Flicmenu *m);
extern void tx_can(Flicmenu *m);
extern void enter_text(void);
extern int lookup_string(char *string, int use);
extern int get_fntchar(int ch, int useoff, int dx, int dy, int sx, int sy,
	int use);
extern void tx_to_world(int xin, int yin, int useoff, int dx, int dy,
	int sx, int sy, float *outx, float *outy);
extern void bez_fix(int pt, int infix, int outfix);
extern short *at1_get_char(int index);
extern short *nq_get_char(int index);
extern short *be_get_char(int index);
extern float text_aspect;
extern int text_constrain;

/* End of this module's functions */

extern void fq_stringq(),see_menu_back();

extern Flicmenu *txaddr;
extern int TXROOT;

extern char gp_buffer[],t1_fullname[];
extern int t1_chars,t1_width,t1_minx,t1_miny,t1_maxy;

struct be_hdr
{
short length;
short namelen;	/* Should be 55 */
short URWnum;
char name[14];
short f1[46];
short fontlen;	/* Should be 12 */
short fontind;
short chars;
} beh;

FILE *fstream;
int tx_ptct,tx_x1,tx_y1,tx_y2,tx_w,tx_flag,thisx,pbase,spc_size=200;
int tx_minx,tx_miny;
int txstat;
int error_in_font=0;
short fontct;
long fidptr,fixptr,fdptr;
float tx1,tx2,ty1,ty2,txscale,tyscale;
NQ_hdr nqh;

/* Text editables */

struct stringq tx_stringq =
 {
 2, 2, 31, 31-1, 0, P.tx_string, NULL, 0, 0,NULL,NULL,ANY_CHAR
 };

/* Text details */

struct feel_tab tx_feel[]=
 {
 TEXTOK,tx_ok,
 TEXTCAN,tx_can,
 EMPTY,NULLFUNC
 };

struct edit_tab tx_edit[]=
 {
 TEXTDATA,fq_stringq,(char *)&tx_stringq,
 EMPTY,NULLFUNC,NOTEXT
 };

/* Translation table for ASCII-URW character ID */

static short asc_urw[256]=
{
1001,1002,1003,1004,1005,1006,1016,1018,1019,1020,
1011,1012,1013,1014,1015,1028,1029,1041,1097,1099,
1098,1021,1042,1039,1040,1036,1037,1115,1038,1030,
1031,-1,614,636,638,512,698,630,609,626,
627,634,640,607,623,601,622,510,501,502,
503,504,505,506,507,508,509,602,608,1111,
644,1112,616,637,101,102,103,104,105,106,
107,108,109,110,111,112,113,114,115,116,
117,118,119,120,121,122,123,124,125,126,
628,700,629,1151,1154,755,301,302,303,304,
305,306,307,308,309,310,311,312,313,314,
315,316,317,318,319,320,321,322,323,324,
325,326,655,1152,656,1108,1336,210,217,639,
418,401,407,404,417,-1,416,427,419,228,
435,429,208,210,236,329,129,239,440,252,
449,452,268,251,402,511,631,2312,-1,619,
403,428,240,451,436,237,658,327,615,-1,
-1,-1,-1,514,618,203,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,754,-1,-1,-1,-1,
657,-1,-1,-1,-1,-1,-1,659,-1,-1,
-1,516,-1,-1,-1,-1,-1,-1,513,651,
-1,-1,-1,-1,-1,-1
};

void
pltx_setup(void)
{
new_snap_rtn(X_PLTX,X_NO_RTN,C_XORHAIRS,progstr(STR1115),0,1);
}

void
do_pltx(int forced)
{
Window *w;
float fdum;
int textconst,res;
int ix,ge_tx,ge_ty,opoly,npoly;

if(P.fontsel==0)
 {
 continu_line(progstr(STR1116));
 return;
 }
if(strlen(P.tx_string)==0)
 {
 continu_line(progstr(STR1117));
 return;
 } 

shp_usave();	/* Save settings for undo */

if(forced)
 {
 if(get_override(2,progstr(STR1118),UNIV_CHAR,&tx1,"Y:",UNIV_CHAR,&ty1,
	NULL,0,NULL)==0)
  goto reset;
 if(get_override(2,progstr(STR1120),UNIV_CHAR,&tx2,"Y:",UNIV_CHAR,&ty2,
	NULL,0,NULL)==0)
  goto reset;
 goto do_it;
 }

w=(Window *)&work_wind[cur_window];

if(mouse_button & 0x2)
 return;

textconst=(get_kstate() & 4) ? 1:0;	/* Constrain to default aspect? */

opoly=poly_count();
if(lookup_string(P.tx_string,0)) {
 if(tx_ptct==0)	/* No valid chars entered */
  {
  continu_line(progstr(STR1123));
  goto reset;
  }
 if((ptct+tx_ptct)>SHAPE_MAX)  {
  sprintf(gp_buffer,progstr(STR1124),tx_ptct,ptct,SHAPE_MAX);
  do_alert(0,gp_buffer);
  goto reset;
  }
 }	
else
  goto reset;

ge_tx=snapx;
ge_ty=snapy;

show_prompt(progstr(STR1122));

/* Let user define a rubber box */

clip_active(w);

if (textconst) {
	text_constrain = 1;
	text_aspect = (float)tx_w/(float)(tx_y2-tx_y1);
#if 0
	printf(" tx_w = %d, tx_y1 = %d tx_y2 = %d , text_aspect = %.4f\n",
		tx_w,tx_y1,tx_y2,text_aspect);
#endif
	}
res = wind_rbox(w,1,textconst,snapx,snapy,&ge_tx,&ge_ty,
	     &tx1,&ty1,&fdum,&tx2,&ty2,&fdum,1);
text_constrain = 0;

if (res==0){
	can_prompt();
	goto reset;
	}

do_it: 
if(tx1==tx2 || ty1==ty2)
 {
 bad_region(0);
 goto reset;
 }

/* Swap corners if necessary */

if(tx1>tx2)
 dataswap((char *)&tx1,(char *)&tx2,sizeof(float));
if(ty1>ty2)
 dataswap((char *)&ty1,(char *)&ty2,sizeof(float));

/* Find text dimensions and create, if possible */

tyscale=(ty2-ty1)/(float)(tx_y2-tx_y1);
if(textconst)
 txscale=tyscale;
else
 txscale=(tx2-tx1)/(float)tx_w;
tx_minx=tx_x1;
tx_miny=tx_y1;
lookup_string(P.tx_string,1);
fix_1st_flags();	/* Flag 1st verts on all polys */
npoly=poly_count();
for(ix=opoly; ix<npoly; ++ix)
 poly_allwind(ix,WHITE,G_REPLACE);
P.shaper_ok=0;

reset:
restart_rtn();
}

void
select_font(void)
{
char *fname,ext[5];

init_wildlist("FNT","BE","PFB",NULL);
if((fname=get_filename(progstr(STR1126),P.txdrawer,P.txfile))!=NULL)
 {
 split_fext(P.txfile,NULL,ext);
 strcpy(ext,&ext[1]);

 if(strcmp(ext,"PFB")==0)
  {
  if(at1_open(fname)==0)
   {
   at1_close();
   P.fontsel=0;
   return;
   }
  strcpy(gp_buffer,progstr(STR1127));
  t1_fullname[40]=0;
  strcat(gp_buffer,t1_fullname);
  sprintf(&gp_buffer[strlen(gp_buffer)],progstr(STR1128),t1_chars);
  show_prompt(gp_buffer);
  at1_close();
  P.fontsel=1;	/* Indicate .PFB file */
  }
 else
 if(strcmp(ext,"FNT")==0)
  {
  if(nq_open(fname)==0)
   {
   nq_close();
   P.fontsel=0;
   return;
   }
  strcpy(gp_buffer,progstr(STR1127));
  strcat(gp_buffer,nqh.fontname);
  sprintf(&gp_buffer[strlen(gp_buffer)],progstr(STR1128),nqh.num_chars);
  show_prompt(gp_buffer);
  nq_close();
  P.fontsel=2;	/* Indicate .FNT file */
  }
 else
 if(strcmp(ext,"BE")==0)
  {
  if(be_open(fname)==0)
   {
   be_close();
   P.fontsel=0;
   return;
   }
  strcpy(gp_buffer,progstr(STR1127));
  beh.name[13]=0;
  strcat(gp_buffer,beh.name);
  sprintf(&gp_buffer[strlen(gp_buffer)],progstr(STR1128),beh.chars);
  show_prompt(gp_buffer);
  be_close();
  P.fontsel=3;	/* Indicate .BE file */
  }
 else
  invalid_ext();
 }
}

void
rdy_txdia(void)
{
rsrc_plugin(TXROOT,tx_feel,tx_edit,NULL,NULL,NULL,NULL);
}

void
tx_ok(Flicmenu *m)
{
close_menu();
txstat=1;
}

void
tx_can(Flicmenu *m)
{
close_menu();
txstat=0;
}

void
enter_text(void)
{
int ix;

init_stq_string(&tx_stringq);
strcpy(gp_buffer,P.tx_string);

tx_loop:
fix_cursor(C_ARROW,0);
center_dialog(txaddr);
sdraw_menu(txaddr);
do_menu(txaddr,TXROOT+TEXTDATA);
unfix_cursor(0);

if(txstat)
 {
 if(strlen(P.tx_string)==0)
  {
  bad_string:
  continu_line(progstr(STR1129));
  goto tx_loop;
  }
 for(ix=0; ix<strlen(P.tx_string); ++ix)
  {
  if(P.tx_string[ix]!=' ')
   goto ok_string;
  }
 goto bad_string;

 ok_string:;
 }
else
 strcpy(P.tx_string,gp_buffer);
}

/* Go out to font file, grab chars in string */
/* Returns 0 on error			     */

int cont_warned;

int
lookup_string(char *string,int use)
{
int ix,status;

status=0;
cont_warned=0;

/* Open font file, skip header */

strcpy(gp_buffer,P.txdrawer);
strcat(gp_buffer,"\\");
strcat(gp_buffer,P.txfile);
switch(P.fontsel)
 {
 case 1:
  if(at1_open(gp_buffer)==0)
   {
   at1_close();
   return(0);
   }
  break;
 case 2:
  if(nq_open(gp_buffer)==0)
   {
   nq_close();
   return(0);
   }
  break;
 case 3:
  if(be_open(gp_buffer)==0)
   {
   be_close();
   return(0);
   }
  break;
 }

/* Reset string information, get i size for space */

tx_w=0;
switch(get_fntchar('i',0,0,0,1,1,0))
 {
 case -1:
  goto done;
 case 1:
  spc_size=tx_w;
  break;
 default:
  spc_size=200;
  break;
 }

/* Reset string information */

tx_ptct=tx_flag=tx_w=0;

for(ix=0; ix<strlen(string); ++ix)
 {
 if(get_fntchar((int)string[ix],0,0,0,1,1,use)<=0)
  goto done;
 } 
status=1;

done:

switch(P.fontsel)
 {
 case 1:
  at1_close();
  break;
 case 2:
  nq_close();
  break;
 case 3:
  be_close();
  break;
 }
return(status);
}

/* Lookup a character from the font file */
/* Returns 1 (OK) 0 (NOT IN FONT) -1 (ERROR) */

int
get_fntchar(int ch,int useoff,int dx,int dy,int sx,int sy,int use)
{
int setsec,consec,imgsec,digs,start;
float wx,wy,basex,basey;
short data[8],type,*cdata,initx,inity,lastx,lasty,work,setw,n,m;
short xmin,xmax,ymin,ymax,leftset;
struct sh
{
signed char l;
signed char h;
};
union
{
short s;
struct sh b;
} comb;

/* If space, skip it and advance to next position */

if(ch==' ')
 {
 tx_w+=spc_size;
 return(1);
 }

/* See if character ID is in file */

error_in_font=0;
switch(P.fontsel)
 {
 case 1:
  cdata=at1_get_char(ch);
  break;
 case 2:
  cdata=nq_get_char(ch);
  break;
 case 3:
  cdata=be_get_char(ch);
  break;
 }

if(error_in_font)
 return(-1);
if(cdata==NULL)
 return(2); /* If we get here, there was no match -- return 2 */

/* Got character's id match -- calc file offset for actual data */

got_id:

switch(P.fontsel)
 {
 case 1:	/* Adobe Type1 (.PFB) font */
  tx_x1=t1_minx;
  tx_y1=t1_miny;
  tx_y2=t1_maxy;

  start=1;
  thisx=tx_w;
  tx_w+=t1_width;
  at1_loop:
  switch(*cdata++)
   {
   case 0:	/* End of description */
    break;
   case 1:	/* Add a spline segment */
    data[0]= *cdata++;
    data[1]= *cdata++;
    data[2]= *cdata++;
    data[3]= *cdata++;
    data[4]= *cdata++;
    data[5]= *cdata++;
    data[6]= *cdata++;
    data[7]= *cdata++;
    
    if(start)
     {
     start=0;
     pbase=ptct;
     tx_to_world(data[0],data[1],useoff,dx,dy,sx,sy,&basex,&basey);
	     
     if(use)
      {
      Shpwork[ptct].x=basex;
      Shpwork[ptct].y=basey;
      Shpwork[ptct].z=
       Shpwork[ptct].inx=Shpwork[ptct].iny=Shpwork[ptct].inz=
       Shpwork[ptct].outx=Shpwork[ptct].outy=Shpwork[ptct].outz=0.0;
      Shpwork[ptct].flags=0;
      }
     }
    else
     {
     if(use)
      {
      tx_to_world(data[0],data[1],useoff,dx,dy,sx,sy,&wx,&wy);
      if(wx!=Shpwork[ptct].x || wy!=Shpwork[ptct].y)
       {
       if(cont_warned==0)
        {
        continu_line("Continuity mismatch in font");
        cont_warned=1;
        }
       }
      }

     /* Check Closure */

     tx_to_world(data[6],data[7],useoff,dx,dy,sx,sy,&wx,&wy);
     if(wx==basex && wy==basey)
      {
      tx_ptct++;
      if(use)
       {
       tx_to_world(data[2],data[3],useoff,dx,dy,sx,sy, 
	 &Shpwork[ptct].outx,&Shpwork[ptct].outy);
       bez_fix(ptct,0,1);
       tx_to_world(data[4],data[5],useoff,dx,dy,sx,sy,
 	 &Shpwork[pbase].inx,&Shpwork[pbase].iny);
       bez_fix(pbase,1,0);
       start=1;
       Shpwork[ptct++].flags=POLYEND | POLYCLOSED;
       }
      goto at1_loop;
      }
     }
    tx_ptct++;
    if(use)
     {
     tx_to_world(data[2],data[3],useoff,dx,dy,sx,sy,
	&Shpwork[ptct].outx,&Shpwork[ptct].outy);
     bez_fix(ptct,0,1);
     ptct++;
     tx_to_world(data[4],data[5],useoff,dx,dy,sx,sy, 
	&Shpwork[ptct].inx,&Shpwork[ptct].iny);
     tx_to_world(data[6],data[7],useoff,dx,dy,sx,sy,
	&Shpwork[ptct].x,&Shpwork[ptct].y);
     bez_fix(ptct,1,0);
     Shpwork[ptct].z=Shpwork[ptct].inz=
     Shpwork[ptct].outx=Shpwork[ptct].outy=Shpwork[ptct].outz=0.0;
     Shpwork[ptct].flags=0;
     }
    goto at1_loop;
   case 2:	/* End of polygon */
    if(start==0)
     {
     tx_ptct++;
     if(use)
      Shpwork[ptct++].flags=POLYEND | POLYCLOSED;
     start=1;
     }
    goto at1_loop;
   }
  break;
 case 2:	/* Nimbus Q (.FNT) font */
  setw=data[0]= *cdata++;
  if(useoff==0)
   {
   thisx=tx_w;
   tx_w+=data[0];
   }
  data[0]= *cdata++;
  data[1]= *cdata++;
  data[2]= *cdata++;
  data[3]= *cdata++;
  if(useoff==0)
   {
   if(tx_flag==0)
    {
    tx_flag=1;
    tx_x1=data[0];
    tx_y1=data[1];
    tx_y2=data[3];
    }
   else
    {
    if(data[0]<tx_x1)
     tx_x1=data[0];
    if(data[1]<tx_y1)
     tx_y1=data[1];
    if(data[3]>tx_y2)
     tx_y2=data[3];
    }
   }
  n= *cdata++;
  cdata= &cdata[3*n];	/* Bypass N data */
  m= *cdata++;
  cdata= &cdata[3*m];	/* Bypass M data */
  nq_loop:
  type= *cdata++;
  switch(type)
   {
   case 0:
    data[0]= *cdata++;
    data[1]= *cdata++;
    initx=lastx=data[0];
    inity=lasty=data[1];
    tx_ptct++;
    if(use)
     {
     pbase=ptct;
     goto normpt2;
     }
    break;
   case 1:
    lastx=data[0]= *cdata++;
    lasty=data[1]= *cdata++;
    nq_line:
    if(data[0]!=initx || data[1]!=inity)
     {
     tx_ptct++;
     if(use)
      {
      normpt2:
      tx_to_world(data[0],data[1],useoff,dx,dy,sx,sy,
  	&Shpwork[ptct].x,&Shpwork[ptct].y);

      Shpwork[ptct].z=
      Shpwork[ptct].inx=Shpwork[ptct].iny=Shpwork[ptct].inz=
      Shpwork[ptct].outx=Shpwork[ptct].outy=Shpwork[ptct].outz=0.0;

      Shpwork[ptct].flags=0;
      ptct+=1;
      }
     }
    else
     {
     if(ptct)
      Shpwork[ptct-1].flags=POLYEND | POLYCLOSED;
     }
    break;
   case 2:
    data[0]= *cdata++;
    data[1]= *cdata++;
    data[2]= *cdata++;
    data[3]= *cdata++;
    lastx=data[4]= *cdata++;
    lasty=data[5]= *cdata++;
    nq_bez:
    if(data[4]!=initx || data[5]!=inity)
     {
     tx_ptct++;
     if(use)
      {
      tx_to_world(data[0],data[1],useoff,dx,dy,sx,sy,
	&Shpwork[ptct-1].outx,&Shpwork[ptct-1].outy);
      bez_fix(ptct-1,0,1);
      tx_to_world(data[2],data[3],useoff,dx,dy,sx,sy,
	&Shpwork[ptct].inx,&Shpwork[ptct].iny);
      tx_to_world(data[4],data[5],useoff,dx,dy,sx,sy,
	&Shpwork[ptct].x,&Shpwork[ptct].y);
      Shpwork[ptct].z=Shpwork[ptct].inz=
      Shpwork[ptct].outx=Shpwork[ptct].outy=Shpwork[ptct].outz=0.0;
      bez_fix(ptct,1,0);

      Shpwork[ptct].flags=0;
      ptct+=1;
      }
     }
    else
     {
     if(use)
      {
      /* Load up bezier control pts only */

      tx_to_world(data[0],data[1],useoff,dx,dy,sx,sy,
  	&Shpwork[ptct-1].outx,&Shpwork[ptct-1].outy);
      bez_fix(ptct-1,0,1);
      tx_to_world(data[2],data[3],useoff,dx,dy,sx,sy,
	&Shpwork[pbase].inx,&Shpwork[pbase].iny);
      bez_fix(pbase,1,0);
      Shpwork[ptct-1].flags=POLYEND | POLYCLOSED;
      }
     }
    break;
   case 3:
    return(1);
   case 5:
    data[0]= *cdata++;
    if(get_fntchar(data[0],1,0,0,sx,sy,use)<=0)
     return(0);
    data[1]= *cdata++;
    data[2]= *cdata++;
    data[3]= *cdata++;
    if(get_fntchar(data[3],1,data[1],data[2],sx,sy,use)<=0)
     return(0);
    break;
   case 6:
    data[0]=lastx;
    lasty=data[1]=lasty + *cdata++;
    goto nq_line;
   case 7:
    lastx=data[0]=lastx + *cdata++;
    data[1]=lasty;
    goto nq_line;
   case 8:
    comb.s= *cdata++;
    lastx=data[0]=lastx+comb.b.h;
    lasty=data[1]=lasty+comb.b.l;
    goto nq_line;
   case 9:
    comb.s= *cdata++;
    data[0]=lastx+comb.b.h;
    data[1]=lasty+comb.b.l;
    comb.s= *cdata++;
    data[2]=data[0]+comb.b.h;
    data[3]=data[1]+comb.b.l;
    comb.s= *cdata++;
    lastx=data[4]=data[2]+comb.b.h;
    lasty=data[5]=data[3]+comb.b.l;
    goto nq_bez;
   default:
    break;
   }
  goto nq_loop;
 case 3:	/* .BE fonts */
  setsec=cdata[0];
  consec=cdata[setsec]+setsec;
  imgsec=consec+cdata[consec]*2048+cdata[consec+1];

  digs=cdata[setsec+2];
  if(useoff==0)
   {
   thisx=tx_w;
   tx_w+=cdata[setsec+3];
   }
  leftset=cdata[setsec+4];
  xmin=data[0]=cdata[setsec+7]+leftset;
  ymin=data[1]=cdata[setsec+9];
  xmax=data[2]=cdata[setsec+8]+leftset;
  ymax=data[3]=cdata[setsec+10];
  if(useoff==0)
   {
   if(tx_flag==0)
    {
    tx_flag=1;
    tx_x1=data[0];
    tx_y1=data[1];
    tx_y2=data[3];
    }
   else
    {
    if(data[0]<tx_x1)
     tx_x1=data[0];
    if(data[1]<tx_y1)
     tx_y1=data[1];
    if(data[3]>tx_y2)
     tx_y2=data[3];
    }
   }

  cdata= &cdata[imgsec+2];

  while(digs)
   {
   if(cdata[0]<0 && cdata[1]>=0)
    type=2;	/* Moveto */
   else
   if(cdata[0]<0 && cdata[1]<0)
    type=3;	/* Lineto */
   else
   if(cdata[0]>=0 && cdata[1]>=0)
    type=4;	/* Curveto */

   switch(type)
    {
    case 2:
     digs--;
     data[0]=abs(*cdata++)-1+xmin;
     data[1]=abs(*cdata++)-1+ymin;
     initx=data[0];
     inity=data[1];
     tx_ptct++;
     if(use)
      {
      pbase=ptct;
      goto normpt3;
      }
     break;
    case 3:
     digs--;
     data[0]=abs(*cdata++)-1+xmin;
     data[1]=abs(*cdata++)-1+ymin;
     if(data[0]!=initx || data[1]!=inity)
      {
      tx_ptct++;
      if(use)
       {
       normpt3:
       tx_to_world(data[0],data[1],useoff,dx,dy,sx,sy,
   	&Shpwork[ptct].x,&Shpwork[ptct].y);

       Shpwork[ptct].z=
       Shpwork[ptct].inx=Shpwork[ptct].iny=Shpwork[ptct].inz=
       Shpwork[ptct].outx=Shpwork[ptct].outy=Shpwork[ptct].outz=0.0;
       Shpwork[ptct].flags=0;
       ptct+=1;
       }
      }
     else
      {
      if(ptct)
       Shpwork[ptct-1].flags=POLYEND | POLYCLOSED;
      }
     break;
    case 4:
     digs-=3;
     data[0]=abs(*cdata++)-1+xmin;
     data[1]=abs(*cdata++)-1+ymin;
     data[2]=abs(*cdata++)-1+xmin;
     data[3]=abs(*cdata++)-1+ymin;
     data[4]=abs(*cdata++)-1+xmin;
     data[5]=abs(*cdata++)-1+ymin;
     if(data[4]!=initx || data[5]!=inity)
      {
      tx_ptct++;
      if(use)
       {
       tx_to_world(data[0],data[1],useoff,dx,dy,sx,sy,
	&Shpwork[ptct-1].outx,&Shpwork[ptct-1].outy);
       bez_fix(ptct-1,0,1);
       tx_to_world(data[2],data[3],useoff,dx,dy,sx,sy,
	&Shpwork[ptct].inx,&Shpwork[ptct].iny);
       tx_to_world(data[4],data[5],useoff,dx,dy,sx,sy,
	&Shpwork[ptct].x,&Shpwork[ptct].y);
       Shpwork[ptct].z=Shpwork[ptct].inz=
       Shpwork[ptct].outx=Shpwork[ptct].outy=Shpwork[ptct].outz=0.0;
       bez_fix(ptct,1,0);

       Shpwork[ptct].flags=0;
       ptct+=1;
       }
      }
     else
      {
      if(use)
       {
       /* Load up bezier control pts only */

       tx_to_world(data[0],data[1],useoff,dx,dy,sx,sy,
  	&Shpwork[ptct-1].outx,&Shpwork[ptct-1].outy);
       bez_fix(ptct-1,0,1);
       tx_to_world(data[2],data[3],useoff,dx,dy,sx,sy,
	&Shpwork[pbase].inx,&Shpwork[pbase].iny);
       bez_fix(pbase,1,0);
       Shpwork[ptct-1].flags=POLYEND | POLYCLOSED;
       }
      }
     break;
    }
   }
  return(1);
 }
}

/* Calc world coords from text coords */

void
tx_to_world(int xin,int yin,int useoff,int dx,int dy,int sx,int sy,
	float *outx,float *outy)
{
float workx,worky;

/*
*outx=(float)xin;
*outy=(float)yin;
return;
*/

workx=(float)(xin-tx_minx);
worky=(float)(yin-tx_miny);

if(useoff)
 {
 workx=(float)dx+((float)sx/256.0)*workx;
 worky=(float)dy+((float)sy/256.0)*worky;
 }

workx+=(float)thisx;

*outx=workx*txscale+tx1;
*outy=worky*tyscale+ty1;
}

/* Fix bezier control points so they're relative to verts */

void
bez_fix(int pt,int infix,int outfix)
{
if(infix)
 {
 if(Shpwork[pt].inx!=0.0)
  Shpwork[pt].inx-=Shpwork[pt].x;
 if(Shpwork[pt].iny!=0.0)
  Shpwork[pt].iny-=Shpwork[pt].y;
 }
if(outfix)
 {
 if(Shpwork[pt].outx!=0.0)
  Shpwork[pt].outx-=Shpwork[pt].x;
 if(Shpwork[pt].outy!=0.0)
  Shpwork[pt].outy-=Shpwork[pt].y;
 }
}

int
nq_open(char *fname)
{
if((fstream=fopen(fname,"rb"))==NULL)
 {
 cant_open_font();
 return(0);
 }
if(fread(&nqh,sizeof(nqh),1,fstream)!=1)
 {
 continu_line(progstr(STR0353));
 nq_close();
 return(0);
 }

return(1);
}

int nq_close(void)
{
if(fstream!=NULL)
 {
 fclose(fstream);
 fstream=NULL;
 }
}

/* Get the nth character in the Nimbus-Q font file */

short *
nq_get_char(int index)
{
int ix,jx;
short *buf,urw;
long offset;
static int dumped=0;

/* Get URW code for this character */

urw=asc_urw[index-1];

buf=(short *)holebuf;
fseek(fstream,(long)sizeof(NQ_hdr),SEEK_SET);
if(fread(buf,2*nqh.num_chars,1,fstream)!=1)
 {
 badfont:
 show_prompt(progstr(STR2027));
 return(NULL);
 }
/*#define DEBUGGING*/
#ifdef DEBUGGING
if(dumped==0)
 {
 dumped=1;
 printf("NQ font contains:\n");
 for(ix=0; ix<nqh.num_chars; ++ix)
  {
  printf("Code:%d\n",buf[ix]);
  next_ix:;
  }
 getch();
 }
#endif

for(ix=0; ix<nqh.num_chars; ++ix)
 {
 if(buf[ix]==urw)
  goto got_char;
 }

/* Char not in font! */

return(NULL);

got_char:
fseek(fstream,(long)sizeof(NQ_hdr)+nqh.num_chars*2L+ix*4L,SEEK_SET);
if(fread(&offset,4,1,fstream)!=1)
 goto badfont;

fseek(fstream,offset,SEEK_SET);
fread(buf,4096,1,fstream);
return(buf);
}

/* Open and check out .BE file */

int
be_open(char *fname)
{
if((fstream=fopen(fname,"rb"))==NULL)
 {
 cant_open_font();
 return(0);
 }
if(fread(&beh,sizeof(beh),1,fstream)!=1)
 {
 bad_be:
 continu_line(progstr(STR0353));
 be_close();
 return(0);
 }
if(beh.namelen!=55 || beh.fontlen!=12)
 goto bad_be;

return(1);
}

int be_close(void)
{
if(fstream!=NULL)
 {
 fclose(fstream);
 fstream=NULL;
 }
}

/* Get the nth character in the .BE font file */

short *
be_get_char(int index)
{
int ix,chcount;
short temp;
long seekpoint;
struct x
{
short ch;
short rec;
short ptr;
} chptr;
struct x2
{
short recs;
short words;
} chsize;
struct x *chlist;

index--;

fseek(fstream,68L*2L,SEEK_SET);	/* Seek to hierarchy */
if(fread(&temp,2,1,fstream)!=1)
 {
 bad_be:
 show_prompt(progstr(STR2028));
 return(NULL);
 }

/* Seek past hierarchy and into character index section */

temp--;
fseek(fstream,2L*temp,SEEK_CUR);

/* Report information on characters */

if(fread(&chptr,6,1,fstream)!=1)
 goto bad_be;

/* Read in the character index information */

chptr.ch-=3;	/* Subtract out header words */
chcount=chptr.ch/3;

chlist=(struct x *)holebuf;
if(fread(chlist,chcount*6,1,fstream)!=1)
 goto bad_be;

for(ix=0; ix<chcount; ++ix)
 {
 if(chlist[ix].ch==asc_urw[index])
  goto gotch;
 }
return(NULL);

/* If we reach here, we have char pointer.  Seek to it & read it */

gotch:
/*
printf("Char %d (%c) urw:%d rec:%d ptr:%d\n",index+1,index+1,chlist[ix].ch,
	chlist[ix].rec,chlist[ix].ptr-1);
*/
seekpoint=(chlist[ix].rec-1)*4096L+(long)(chlist[ix].ptr-1)*2;
fseek(fstream,seekpoint,SEEK_SET);
if(fread(&chsize,4,1,fstream)!=1)
 goto bad_be;
temp=chsize.recs*4096+chsize.words*2;
if(fread(holebuf,temp,1,fstream)!=1)
 goto bad_be;
return((short *)holebuf);
}
