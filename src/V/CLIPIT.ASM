

;clipit.asm - clipping routine for blits.

;:ts=10

	TITLE   clipit
_TEXT	SEGMENT  BYTE PUBLIC 'CODE'
_TEXT	ENDS
_DATA	SEGMENT  WORD PUBLIC 'DATA'
_DATA	ENDS
CONST	SEGMENT  WORD PUBLIC 'CONST'
CONST	ENDS
_BSS	SEGMENT  WORD PUBLIC 'BSS'
_BSS	ENDS
DGROUP	GROUP	CONST,	_BSS,	_DATA
	ASSUME  CS: _TEXT, DS: DGROUP, SS: DGROUP, ES: DGROUP
_TEXT      SEGMENT



	;clipblit - raster blit oriented clip.  Expects bp to point to
	;a parameter frame as suggested below.  Returns with carry set
	;if clipped out, otherwise adjusts wid,height,sx1,sy1,dx1,dy1
	;to clipped values.  Trashes ax.
	PUBLIC	clipblit
wid	equ 4+2
height	equ 6+2
sx1	equ 8+2
sy1	equ 10+2
spt_o	equ 12+2
spt_s	equ 14+2
snext	equ 16+2
dx1	equ 18+2
dy1	equ 20+2
dpt_o	equ 22+2
dpt_s	equ 24+2
dnext	equ 26+2
clipblit proc far
	;1st  clip to the right
	mov	bx,[bp+wid]
	or	bx,bx	;set flags
	jle	bclipped
	mov	ax,[bp+dx1]	;get starting dest x
	sub	ax,320
	jge	bclipped
	add	ax,bx
	jl	norclip
	sub	[bp+wid],ax
norclip:	;clip to the left
	mov	ax,[bp+dx1]	;get starting dest x
	and	ax,ax		;set flags
	jns	nolclip
	add	[bp+wid],ax
	jle	bclipped
	sub	[bp+sx1],ax
	mov	word ptr[bp+dx1],0
nolclip:	;clip off the bottom
	mov	bx,[bp+height]
	or	bx,bx	;set flags
	jle	bclipped
	mov	ax,[bp+dy1]	;get starting dest y
	sub	ax,200
	jge	bclipped
	add	ax,bx
	jl	nobclip
	sub	[bp+height],ax
nobclip:	;clip off the top
	mov	ax,[bp+dy1]	;get starting dest y
	and	ax,ax		;set flags
	jns	nouclip
	add	[bp+height],ax
	jle	bclipped
	sub	[bp+sy1],ax
	mov	word ptr[bp+dy1],0
nouclip:
	clc
	ret
bclipped:
	stc
	ret
clipblit endp
_TEXT	ENDS
END
