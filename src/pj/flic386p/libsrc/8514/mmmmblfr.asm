;mmmmblfr.asm - move rectangular 8 bit image from RamRast to 8514.
;
	include a8514.i
	include raster.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP


;void pj_8514_f_blitrect(RamRast *source, Coor sx, Coor sy,
;			 Vscreen *dest, Coor dx, Coor dy,
;			 Coor width, Coor height);
pj_8514_f_blitrect proc near
	public pj_8514_f_blitrect
mbfp struc
	mbf_ebp	dd ?
	mbf_ret dd ?
	mbf_source dd ?
	mbf_sx dd ?
	mbf_sy dd ?
	mbf_dest dd ?
	mbf_dx dd ?
	mbf_dy dd ?
	mbf_width dd ?
	mbf_height dd ?
mbfp ends
	push ebp
	mov ebp,esp
	push ebx
	push ecx
	push edx
	push esi
	push edi

	;Grab the parameters we need from bytemap structure
	mov edi,[ebp].mbf_source
	mov ebx,[edi].bym_bpr
	mov edi,[edi].bym_p
	;and calculate starting address for source in edi.
	mov eax,[ebp].mbf_sy
	mul ebx
	add eax,[ebp].mbf_sx
	add edi,eax
	;Adjust the x,y dest position based on xcard/ycard in dest rast8514
	mov esi,[ebp].mbf_dest
	mov eax,[esi].vm_xcard
	add [ebp].mbf_dx,eax
	mov eax,[esi].vm_ycard
	add [ebp].mbf_dy,eax

           CLRCMD                      ; v1.00

;Transfer one line at a time using fill rectangle X command of the GP
;with height of one.  Width pixels at a time are moved with an outs
;through the Pixel Transfer port and onto the 8514 screen.

;Set up loop invariant registers in 8514
;width
	mov eax,[ebp].mbf_width
	mov	ecx,eax
	dec eax			
	mov dx,MAJ_AXIS_PCNT
	out dx,ax
	inc ecx 	;Calculate 16 bit word width and save
	shr ecx,1
	mov [ebp].mbf_width,ecx 
;line height
	xor eax,eax
	mov dx,MLTFUNC_CNTL
	out dx,ax
;set up mix to copy from pixel data port
	mov ax,PTRANS_ACTIVE+MIX_SRC
	mov dx,FGRD_MIX
	out dx,ax

;use command to transfer data 1 line at a time.
mbp_line:

	mov eax,[ebp].mbf_dx
	mov dx,CUR_X_POS
	out dx,ax
	mov eax,[ebp].mbf_dy
	mov dx,CUR_Y_POS
	out dx,ax
	inc eax
	mov [ebp].mbf_dy,eax
	mov ax,WRITCMD+DRAWCMD+INCX+INCY+PC_TRANS+BIT16+BYTE_SWAP+FILL_X_RECT
	mov dx,COMMAND
	out dx,ax
	mov dx,PIX_TRANS
	mov esi,edi
	mov ecx,[ebp].mbf_width
	rep outsw
	add edi,ebx	;go to next line of source

	dec [ebp].mbf_height
	jnz mbp_line

           mov       DX, SUBSYS_CNTL   ; v1.00
           mov       AX, 4             ; Clear the "Data register read" flag.
           out       DX, AX

	WAITQ 2
	;set foreground mix back for faster put_dot
	mov ax,F_CLR_ACTIVE+MIX_SRC
	mov dx,FGRD_MIX
	out dx,ax

	pop edi
	pop esi
	pop edx
	pop ecx
	pop ebx
	pop ebp
	ret
pj_8514_f_blitrect endp

code	ends
	end
