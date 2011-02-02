;un5brun.asm - low level routines to uncompress first frame of fli into
;1/5 size screen (by uncompressing 1 out of 5 lines, and skipping  past
;the other 4).


	include raster.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

locals equ (1*4)

;void hscale_unbrun(Raster *dv, void *csource, int scale, int linect)
	public hscale_unbrun
hscale_unbrun proc near
hscp	struc	;hscale_unbrun parameters
	hsc_dbpr dd ?
	hsc_ebp	dd ?
	hsc_ret dd ?
	hsc_dv dd ?
	hsc_compbuf dd ?
	hsc_scale dd ?
	hsc_linect dd ?
hscp ends
	;save the world and set the basepage
	push ebp
	sub esp,locals
	mov ebp,esp
	push ebx
	push ecx
	push edx
	push esi
	push edi
	push ds
	push es

	mov edi,[ebp].hsc_dv	;fetch destination screen structure
	mov eax,[edi].bym_bpr	
	mov [ebp].hsc_dbpr,eax	;save dest bytes-per-row
	mov ax,[edi].bym_pseg	;move dest pixel pointer to es:edi
	mov es,ax
	mov edi,[edi].bym_p
	mov esi,[ebp].hsc_compbuf ;get source pointer into ds:esi

	dec [ebp].hsc_scale;	;uncompress 1, skip scale-1...
	mov	edx,edi	;keep pointer to start of line in dx
	xor	eax,eax	;clear hi bit of ah cause lots of unsigned uses to follow
linelp:
	mov	edi,edx
	add	edx,[ebp].hsc_dbpr		;dest 1 line forwards
	inc esi		;skip over op count
ulcloop:
	lodsb	; load op/count
	test al,al
	js ucopy
	mov ecx,eax ;move count to ecx
	lodsb	  ;value to repeat in al
	rep stosb ;store same value again and again...
	cmp edi,edx
	jnz ulcloop
	jmp ulcout
ucopy:
	neg al
	mov ecx,eax ;get sign  corrected copy count
	rep movsb 
	cmp edi,edx
	jnz ulcloop
ulcout:			;advance to next line...
	mov ebx,[ebp].hsc_scale		;decompressed 1 line, now skip scale-1
skipline:
	inc esi		;skip line op-count
	mov ecx,[ebp].hsc_dbpr
skloop:
	lodsb		;get op/count byte
	test al,al
	js	scopy	;if signed its variable length
	inc esi		;else just followed by one byte
	sub ecx,eax
	jnz skloop
	jmp endskloop
scopy:
	neg al
	add esi,eax
	sub ecx,eax
	jnz skloop
endskloop:
	dec ebx
	jnz skipline

	dec [ebp].hsc_linect
	jnz	linelp


	pop es
	pop ds
	pop edi
	pop esi
	pop edx
	pop ecx
	pop ebx
	add esp,locals
	pop ebp
	ret
hscale_unbrun endp

code	ends
	end
