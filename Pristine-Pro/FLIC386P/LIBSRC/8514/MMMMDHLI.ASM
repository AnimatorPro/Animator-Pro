	include a8514.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

_pj_8514_d_hline proc near
pj_8514_d_hline:
	public _pj_8514_d_hline
	public pj_8514_d_hline
dhlp	struc
	dhl_ebp	dd ?
	dhl_ret	dd ?
	dhl_v		dd ?
	dhl_color dd ?
	dhl_x	dd ?
	dhl_y	dd ?
	dhl_width dd ?
dhlp	ends

	push ebp
	mov ebp,esp

	mov ecx,[ebp].dhl_v

;make sure have enough queue entries

           WAITQ     6                 ; v1.00

;set up x/y position
	mov eax,[ebp].dhl_x
	add eax,[ecx].vm_xcard
	mov dx,CUR_X_POS
	out dx,ax
	mov eax,[ebp].dhl_y
	add eax,[ecx].vm_ycard
	mov dx,CUR_Y_POS
	out dx,ax
	mov eax,[ebp].dhl_color
	mov dx,FRGD_COLOR
	out dx,ax
	mov ecx,[ebp].dhl_width
	mov eax,ecx
	dec eax
	mov dx,MAJ_AXIS_PCNT
	out dx,ax
	mov ax,(WRITCMD+STROKE_ALG+DRAWCMD+LINE_DRAW)
	mov dx,COMMAND
	out dx,ax


	pop ebp
	ret

_pj_8514_d_hline endp

code	ends
	end
