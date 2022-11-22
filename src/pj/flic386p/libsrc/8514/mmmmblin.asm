	include a8514.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP


;void pj_8514_blitrect(Vscreen *source, Coor src_x, Coor src_y,
;			 Vscreen *dest, Coor dest_x, Coor dest_y,
;			 Coor width, Coor height);
pj_8514_blitrect proc near
	public pj_8514_blitrect
mbip struc
	mbi_ebp	dd ?
	mbi_ret dd ?
	mbi_source dd ?
	mbi_sx dd ?
	mbi_sy dd ?
	mbi_dest dd ?
	mbi_dx dd ?
	mbi_dy dd ?
	mbi_width dd ?
	mbi_height dd ?
mbip ends
	push ebp
	mov ebp,esp
	push ecx




;make sure GP is free

           WAITQ     8                 ; v1.00

	;set up source x/y
	mov ecx,[ebp].mbi_source
	mov eax,[ebp].mbi_sx
	add eax,[ecx].vm_xcard
	mov dx,CUR_X_POS
	out dx,ax
	mov eax,[ebp].mbi_sy
	add eax,[ecx].vm_ycard
	mov dx,CUR_Y_POS
	out dx,ax

	;set up dest x/y
	mov ecx,[ebp].mbi_dest
	mov eax,[ebp].mbi_dx
	add eax,[ecx].vm_xcard
	mov dx,DESTX_DIASTP
	out dx,ax
	mov eax,[ebp].mbi_dy
	add eax,[ecx].vm_ycard
	mov dx,DESTY_AXSTP
	out dx,ax

	mov eax,[ebp].mbi_width
	dec eax
	mov dx,MAJ_AXIS_PCNT
	out dx,ax
	mov eax,[ebp].mbi_height
	dec eax
	mov dx,MLTFUNC_CNTL
	out dx,ax

	;
	mov ax,ALL_PLANE_CPY+MIX_SRC
	mov dx,FGRD_MIX
	out dx,ax

	;mov ax,0c0f3h
	mov ax,WRITCMD or PLANAR or DRAWCMD or YMAJAXIS or COPY_RECT
	mov edx,[ebp].mbi_dx
	cmp [ebp].mbi_sx,edx
	jb testy
	or ax,INCX
testy:
	mov edx,[ebp].mbi_dy
	cmp [ebp].mbi_sy,edx
	jb blitit
	or ax,INCY
blitit:
	mov dx,COMMAND
	out dx,ax

           WAITQ     1                 ; v1.00

	;set foreground mix back for faster put_dot
	mov ax,F_CLR_ACTIVE+MIX_SRC
	mov dx,FGRD_MIX
	out dx,ax

	pop ecx
	pop ebp
	ret
pj_8514_blitrect endp

code	ends
	end
