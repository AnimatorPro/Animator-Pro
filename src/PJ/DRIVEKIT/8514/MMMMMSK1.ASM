
	include a8514.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

;void _pj_8514_mask1blit(UBYTE *mbytes, Coor mbpr, Coor sx, Coor sy, 
;	Rast8514 *v, Coor dx, Coor dy,
;	Ucoor width, Ucoor height,
;	RCOLOR oncolor);
_pj_8514_mask1blit proc near
	public _pj_8514_mask1blit
mb1p	struc	;mb1 parameter structure
	mb1_local_spt dd ?
	mb1_local_dpt dd ?
	mb1_local_imask db 4 dup(?)
	mb1_ebp dd ?
	mb1_ret dd ?	;return address for function
	mb1_sp	dd ?
	mb1_sbpr dd ?
	mb1_sx	dd ?
	mb1_sy	dd ?
	mb1_dv	dd ?
	mb1_dx	dd ?
	mb1_dy	dd ?
	mb1_w	dd ?
	mb1_h	dd ?
	mb1_color dd ?
mb1p ends
spt	equ	[ebp].mb1_local_spt
dpt equ [ebp].mb1_local_dpt
imask equ [ebp].mb1_local_imask
	push ebp
	sub esp,12
	mov ebp,esp
	push ebx
	push esi
	push edi

	;add card x/y offsets to destination x/y
	mov edx,[ebp].mb1_dv
	mov eax,[edx].vm_xcard
	add [ebp].mb1_dx,eax
	mov eax,[edx].vm_ycard
	add [ebp].mb1_dy,eax

	;get starting source address in spt
	mov	eax,[ebp].mb1_sy
	mul	[ebp].mb1_sbpr	;y line offset in ax
	mov	ebx,[ebp].mb1_sx
	shr	ebx,3	; += (sx1>>3)
	add	eax,ebx	;start source offset in ax
	add eax,[ebp].mb1_sp
	mov spt,eax


	;calculate start mask for line into imask
	mov ecx,[ebp].mb1_sx
	and ecx,7
	mov al,80h
	shr	al,cl	
	mov imask,al

	;wait for graphics processor to be free.
           CLRCMD                      ; v1.00
           WAITQ     8

	;set up color register (same for all of blit)
	mov	eax,[ebp].mb1_color
	mov dx,FRGD_COLOR
	out dx,ax
	;tell GP we're just doing one dot at a time
	xor eax,eax		
	mov dx,MAJ_AXIS_PCNT	;one dot wide
	out dx,ax
	mov dx,MLTFUNC_CNTL		;one dot high
	out dx,ax

abline:			;This is top of line loop
	mov	ecx,[ebp].mb1_w	;dot count in ecx
	mov	bh,imask		;get mask into bh
	mov	esi,spt			;source start in esi
	mov	edi,[ebp].mb1_dx
	mov	bl,[esi]		;fetch 1st byte of source into bl
	inc	esi

	;Store y position in GP reg and increment
           WAITQ     1                 ; v1.00
	mov eax,[ebp].mb1_dy
	mov dx,CUR_Y_POS
	out dx,ax
	inc eax
	mov [ebp].mb1_dy,eax

abpix:
	test	bl,bh
	jnz	abset
	inc	edi	;increment x
	shr	bh,1
	jz	newsrc
	loop	abpix
	jmp	zline
abset:	
           WAITQ     2                 ; v1.00

	mov eax,edi
	inc edi
	mov dx,CUR_X_POS
	out dx,ax
	mov ax,040F3h
	mov dx,COMMAND
	out dx,ax
	shr	bh,1
	jz	newsrc
	loop	abpix
zline:	mov	eax,[ebp].mb1_sbpr
	add	spt,eax
	dec	[ebp].mb1_h
	jnz	abline
	jmp	za1

newsrc:	;get next byte of source
	mov	bl,[esi]		;fetch byte of source into bl
	inc	esi
	mov	bh,80h		;mask to 1st pixel in byte
	loop	abpix
	jmp	zline

za1:

	pop edi
	pop esi
	pop ebx
	add esp,12
	pop ebp
	ret
_pj_8514_mask1blit endp
code	ends
	end
