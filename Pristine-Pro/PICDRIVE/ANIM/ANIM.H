#ifndef ANIM_H
#define ANIM_H

typedef struct {
   UBYTE operation; /* =5 for RIFF style */
   UBYTE mask;
   USHORT w,h;
   SHORT  x,y;
   ULONG abstime; /* for timing w.r.t. start of anim file in jiffies (notused)*/
   ULONG reltime; /* for timing w.r.t. last frame in jiffies */
   UBYTE interleave; /* =0 for XORing to two frames back, =1 for last frame
                             (not used yet)  */
   UBYTE pad0;
   ULONG bits;
   UBYTE pad[16];
   } AnimationHeader;

#endif
