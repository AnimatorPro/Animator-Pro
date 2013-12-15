
	include a8514.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

	public _pj_8514_set_rect
	public pj_8514_set_rect
_pj_8514_set_rect proc near
pj_8514_set_rect:
msrp	struc
	msr_ebp	dd ?
	msr_red dd ?
	msr_v	dd ?
	msr_color dd ?
	msr_x	dd ?
	msr_y	dd ?
	msr_w	dd ?
	msr_h	dd ?
msrp ends
	push ebp
	mov ebp,esp
	mov ecx,[ebp].msr_v

	;wait for graphics processor to be free.
           WAITQ     8                 ; v1.00

	mov eax,[ebp].msr_x
	add eax,[ecx].vm_xcard
	mov dx,CUR_X_POS
	out dx,ax
	mov eax,[ebp].msr_y
	add eax,[ecx].vm_ycard
	mov dx,CUR_Y_POS
	out dx,ax
	mov eax,[ebp].msr_w
	dec eax
	mov dx,MAJ_AXIS_PCNT
	out dx,ax
	mov eax,[ebp].msr_h
	dec eax
	and ax,0FFFh	;hi bits reserved for select function.  0 - MIN_AXIS_PCNT
	mov dx,MLTFUNC_CNTL
	out dx,ax
	mov eax,[ebp].msr_color
	mov dx,FRGD_COLOR
	out dx,ax
;	mov ax,(WRITCMD+DRAWCMD+INCX+INCY+BIT16+FILL_X_RECT)
	mov ax,40F3h
	mov dx,COMMAND
	out dx,ax

	pop ebp
	ret
_pj_8514_set_rect endp


code	ends
	end
