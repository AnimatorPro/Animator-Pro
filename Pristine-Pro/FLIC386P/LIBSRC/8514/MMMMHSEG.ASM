
	include a8514.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP


_pj_8514_get_hseg proc near
	public _pj_8514_get_hseg
ghsp	struc
	ghs_ebp	dd ?
	ghs_ret	dd ?
	ghs_v		dd ?
	ghs_pixbuf dd ?
	ghs_x	dd ?
	ghs_y	dd ?
	ghs_width dd ?
ghsp	ends

	push ebp
	mov ebp,esp
	push ebx
	push edi

	mov edi,[ebp].ghs_v

;make sure have enough queue entries
           CLRCMD                      ; v1.00


;set up x/y position
	mov eax,[ebp].ghs_x
	add eax,[edi].vm_xcard
	mov dx,CUR_X_POS
	out dx,ax
	mov eax,[ebp].ghs_y
	add eax,[edi].vm_ycard
	mov dx,CUR_Y_POS
	out dx,ax
	mov ecx,[ebp].ghs_width
	mov eax,ecx
	dec eax
	mov dx,MAJ_AXIS_PCNT
	out dx,ax
	mov ax,03318h
	mov dx,COMMAND
	out dx,ax
	mov dx,CMD_STATUS
ghs_wloop:
	in al,dx
	test al,DATA_AVAIL
	jz	ghs_wloop

	mov edi,[ebp].ghs_pixbuf
	mov dx,PIX_TRANS
	mov ebx,ecx
	shr ecx,1	;doing most of it word at a time
	jecxz ghs_lastpixl
	rep insw
ghs_lastpixl:
	test bl,1
	jz ghs_nolast
	in ax,dx
	stosb
ghs_nolast:
           mov       DX, SUBSYS_CNTL   ; v1.00
           mov       AX, 4             ; Clear the "Data register read" flag.
           out       DX, AX


	pop edi
	pop ebx
	pop ebp
	ret

_pj_8514_get_hseg endp

_pj_8514_put_hseg proc near
	public _pj_8514_put_hseg
phsp	struc
	phs_ebp	dd ?
	phs_ret	dd ?
	phs_v		dd ?
	phs_pixbuf dd ?
	phs_x	dd ?
	phs_y	dd ?
	phs_width dd ?
phsp	ends

	push ebp
	mov ebp,esp
	push ebx
	push esi

	mov ecx,[ebp].phs_v

;make sure have GP is free
           CLRCMD                      ; v1.00

;set up x/y position
	mov eax,[ebp].phs_x
	add eax,[ecx].vm_xcard
	mov dx,CUR_X_POS
	out dx,ax
	mov eax,[ebp].phs_y
	add eax,[ecx].vm_ycard
	mov dx,CUR_Y_POS
	out dx,ax
	mov ecx,[ebp].phs_width
	mov eax,ecx
	dec eax
	mov dx,MAJ_AXIS_PCNT
	out dx,ax
	mov ax,PTRANS_ACTIVE+MIX_SRC
	mov dx,FGRD_MIX
	out dx,ax
	mov ax,03319h
	mov dx,COMMAND
	out dx,ax
	mov esi,[ebp].phs_pixbuf
	mov dx,PIX_TRANS
	inc ecx		;make sure get last pixel if odd.
	shr ecx,1
	rep outsw

           mov       DX, SUBSYS_CNTL   ; v1.00
           mov       AX, 4             ; Clear the "Data register read" flag.
           out       DX, AX

	WAITQ 2
	;set foreground mix back for faster put_dot
	mov ax,F_CLR_ACTIVE+MIX_SRC
	mov dx,FGRD_MIX
	out dx,ax


	pop esi
	pop edx
	pop ebp
	ret

_pj_8514_put_hseg endp

code	ends
	end
