;*****************************************************************************
;* hgsdcomp.asm - Decompression-related driver routines.
;*****************************************************************************

	include hgsc.inc
	include macro.inc
	include ..\inc\rastlib.i

_CODE	segment para public use32 'CODE'

	public	hgs_unss2


;-----------------------------------------------------------------------------
; void unss2_rect(Hrast *r, void *psource, long pixsize, Coor x,y,w,h);
; Note:  A major cheapness exists here...if either the X coordinate or the
;	 width value is odd, we punt off to the generic library unss2 routine.
;	 The HGSC just doesn't do odd-aligned transfers fast enough to make
;	 it worth trying to write performance code for such rare situations.
;-----------------------------------------------------------------------------

hgs_unss2 proc	near

therast equ	dword ptr [ebp+8]
psource equ	dword ptr [ebp+12]
pixsize equ	dword ptr [ebp+16]
xcoor	equ	dword ptr [ebp+20]
ycoor	equ	dword ptr [ebp+24]
wcoor	equ	dword ptr [ebp+28]
hcoor	equ	dword ptr [ebp+32]

linecnt equ	dword ptr [esp] 	;

LCLSIZE equ	4

	mov	eax,[esp+16]		; get X coordinate,
	or	eax,[esp+24]		; OR in the width,
	test	al,1			; if both values were even, we can
	jz	short we_can_do_it	; do it in high-performance code below.
	mov	eax,[esp+4]		; else get the raster pointer parm,
	mov	eax,[eax].rast_grclib	; get the generic lib pointer from the
	jmp	dword ptr RL_UNSS2RECT[eax]  ; raster struct, punt to generic.

we_can_do_it:

	push	ebp
	mov	ebp,esp
	Save	ebx,esi,edi,es
	sub	esp,LCLSIZE

	mov	ax,SS_DOSMEM
	mov	es,ax

	mov	wpes:[HCTRL],0C800h	; halt 34010, no incr, incw

	mov	esi,psource		; load pointer to compressed data.
	movzx	eax,wpds:[esi]		; load line count from data stream,
	add	esi,2			; adjust input pointer.
	mov	linecnt,eax		; save it in our local variable.

	mov	edx,ycoor		; load starting Y coordinate,
	shl	edx,YSHFTMUL		; scale it to hgs memory.
	mov	eax,xcoor		; load starting X coordinate,
	lea	edx,[eax*8+edx] 	; make pointer into hgs memory.

	jmp	short lineloop		; jump to start of processing

nextline:
	mov	ebp,-1		   ;2
	add	linecnt,ebp	   ;2
	jz	short done	   ;3
skiplines:			   ;
	shl	ebp,YSHFTMUL	   ;3	; scale skiplines value to hgs memory.
	sub	edx,ebp 	   ;2	; skip value is negative; this is an add.
lineloop:			   ;
	movsx	ebp,word ptr [esi] ;6	; get next opword value.
	add	esi,2		   ;2	; increment input pointer.
	test	ebp,ebp 	   ;2	; test two high bits for special cases,
	js	short skiplines    ;10	; if either bit on, go do special stuff.
dopackets:			   ;
	mov	ebx,edx 	   ;2	; point to 1st word on line in hgs mem.
	mov	edi,HDATA16	   ;2	; point to hgs fast memory.
packetloop:			   ;
	sub	ebp,1		   ;2	; decrement packet count, if done with
	jb	short nextline	   ;9	; packets, go get next line data.
				   ;
	lodsw			   ;5
	movsx	ecx,ah		   ;3
	and	eax,0FFh	   ;2
	lea	ebx,[eax*8+ebx]    ;2+1 ; add skip count to X offset.
				   ;
;	movzx	eax,byte ptr [esi] ;3	; load byte skip count,
;	inc	esi		   ;2	; increment input pointer.
;	lea	ebx,[eax*8+ebx]    ;2+1 ; add skip count to X offset.
;	movsx	ecx,byte ptr[esi]  ;3	; get op byte (repeat or literal count).
;	inc	esi		   ;2	; increment input pointer. assume we
				   ;
	neg	ecx		   ;2	; have a run, neg the op to a runlength.
	jz	short packetloop   ;10	; if it was just a skip count, loop now.
	js	short doliteral    ;9	; if count now negative, we have literal.
	mov	eax,ebx 	   ;	; set up hgs transfer address...
	mov	wpes:[HADDL],ax    ;	; store low order of address
	shr	eax,16		   ;	; move high order to low part of eax
	mov	wpes:[HADDH],ax    ;	; store high order of address
	lea	eax,[ecx*8]	   ;	; scale length from word to hgs byte
	lea	ebx,[eax*2+ebx]    ;	; count and and add for next X offset.
	lodsw			   ;	; load the word to be repeated.
	rep stosw		   ;	; store the run.
	jmp	short packetloop   ;	; go do next packet.
doliteral:			   ;
	neg	ecx		   ;2	; make literal count positive again,
	mov	eax,ebx 	   ;	; set up hgs transfer address...
	mov	wpes:[HADDL],ax    ;	; store low order of address
	shr	eax,16		   ;	; move high order to low part of eax
	mov	wpes:[HADDH],ax    ;	; store high order of address
	lea	eax,[ecx*8]	   ;	; scale length from word to hgs byte
	lea	ebx,[eax*2+ebx]    ;	; count and and add for next X offset.
	rep movsw		   ;	; move the literal to hgs memory.
	jmp	short packetloop   ;	; go do the next packet.
done:
	add	esp,LCLSIZE
	Restore ebx,esi,edi,es
	pop	ebp
	ret

hgs_unss2 endp

_CODE	ends
	end
