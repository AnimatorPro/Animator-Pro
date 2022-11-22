
	include a8514.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

dot_v equ [ebp+8]
dot_color equ [ebp+12]
dot_x equ [ebp+16]
dot_y equ [ebp+20]

	public _pj_8514_put_dot
	public pj_8514_put_dot
_pj_8514_put_dot proc	near
pj_8514_put_dot:
	push ebp
	mov ebp,esp

	mov ecx,dot_v

           WAITQ     6                 ; v1.00

	mov eax,dot_x
	add eax,[ecx].vm_xcard
	mov dx,CUR_X_POS
	out dx,ax
	mov eax,dot_y
	add eax,[ecx].vm_ycard
	mov dx,CUR_Y_POS
	out dx,ax
	mov eax,dot_color
	mov dx,FRGD_COLOR
	out dx,ax
	xor eax,eax		;just one dot...
	mov dx,MAJ_AXIS_PCNT
	out dx,ax
	mov dx,MLTFUNC_CNTL
	out dx,ax
	mov ax,040F3h
	mov dx,COMMAND
	out dx,ax

	pop ebp
	ret	
_pj_8514_put_dot	endp

_pj_8514_get_dot proc near
pj_8514_get_dot:
	public _pj_8514_get_dot
	public pj_8514_get_dot
gdot_v equ [ebp+8]
gdot_x equ [ebp+12]
gdot_y equ [ebp+16]

	push ebp
	mov ebp,esp

	mov ecx,gdot_v

;make sure have enough queue entries
           CLRCMD                      ; v1.00

	xor eax,eax		;just one dot...
      inc AX ; Account for word read.
	mov dx,MAJ_AXIS_PCNT
	out dx,ax

;set up x/y position
	mov eax,gdot_x
	add eax,[ecx].vm_xcard
	mov dx,CUR_X_POS
	out dx,ax
	mov eax,gdot_y
	add eax,[ecx].vm_ycard
	mov dx,CUR_Y_POS
	out dx,ax
	mov dx,COMMAND
	mov ax,03318h
	out dx,ax
	mov dx,CMD_STATUS
wloop:
	in al,dx
	test al,DATA_AVAIL
	jz	wloop

	mov dx,PIX_TRANS
	in ax,dx
	and eax,0FFh

           push      AX
           mov       DX, SUBSYS_CNTL   ; v1.00
           mov       AX, 4             ; Clear the "Data register read" flag.
           out       DX, AX
           pop       AX

	pop ebp
	ret

_pj_8514_get_dot endp


code	ends
	end
