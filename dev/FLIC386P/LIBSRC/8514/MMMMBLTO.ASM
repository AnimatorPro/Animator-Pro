;mmmmblto.asm - move rectangular 8 bit image to RamRast from 8514.
;
	include a8514.i
	include raster.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP


;void pj_8514_t_blitrect(Vscreen *source, Coor sx, Coor sy,
;			 RamRast *dest, Coor dx, Coor dy,
;			 Coor width, Coor height);
pj_8514_t_blitrect proc near
	public pj_8514_t_blitrect
mbtp struc
	mbt_ebp	dd ?
	mbt_ret dd ?
	mbt_source dd ?
	mbt_sx dd ?
	mbt_sy dd ?
	mbt_dest dd ?
	mbt_dx dd ?
	mbt_dy dd ?
	mbt_width dd ?
	mbt_height dd ?
mbtp ends
	push ebp
	mov ebp,esp
	push ebx
	push ecx
	push edx
	push esi
	push edi

	;Grab the parameters we need from bytemap structure
	mov esi,[ebp].mbt_dest
	mov ebx,[esi].bym_bpr
	mov esi,[esi].bym_p
	;and calculate starting address for bytemap dest in esi.
	mov eax,[ebp].mbt_dy
	mul ebx
	add eax,[ebp].mbt_dx
	add esi,eax
	;add x/y offsets of this particular screen in card to source position
	mov edi,[ebp].mbt_source
	mov eax,[edi].vm_xcard
	add [ebp].mbt_sx,eax
	mov eax,[edi].vm_ycard
	add [ebp].mbt_sy,eax


;make sure GP is free

           CLRCMD                      ; v1.00

;Transfer lines using stroke line horizontal command of the GP.
;Most of transfer is done 2 bytes at a time with an ins on the Pixel Transfer 
;port. If width is odd do last pixel separately.
;

;Set up loop invariant registers in 8514
;width
	mov eax,[ebp].mbt_width
	dec eax			
	mov dx,MAJ_AXIS_PCNT
	out dx,ax

;use stroke line command to transfer data 1 line at a time.
mbt_line:
	mov eax,[ebp].mbt_sx
	mov dx,CUR_X_POS
	out dx,ax
	mov eax,[ebp].mbt_sy
	mov dx,CUR_Y_POS
	out dx,ax
	inc eax
	mov [ebp].mbt_sy,eax
	mov ax,DRAWCMD+PC_TRANS+BIT16+BYTE_SWAP+STROKE_ALG+LINE_DRAW
	mov dx,COMMAND
	out dx,ax
	mov dx,PIX_TRANS
	mov edi,esi
	mov eax,[ebp].mbt_width
	mov ecx,eax
	shr ecx,1
	rep insw
	test eax,1	;check for odd length
	jz mbt_nolast
	in ax,dx	;fetch last word
	stosb		;store last byte
mbt_nolast:
	add esi,ebx	;go to next line of dest
	dec [ebp].mbt_height
	jnz mbt_line

           mov       DX, SUBSYS_CNTL   ; v1.00
           mov       AX, 4             ; Clear the "Data register read" flag.
           out       DX, AX

	pop edi
	pop esi
	pop edx
	pop ecx
	pop ebx
	pop ebp
	ret
pj_8514_t_blitrect endp

code	ends
	end
