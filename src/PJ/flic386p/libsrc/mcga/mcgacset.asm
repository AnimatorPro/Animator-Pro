CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

;void pj_mcga_set_colors(void *v, int startix, int count, Color *cmap);
	public pj_mcga_set_colors
pj_mcga_set_colors proc near
jscp	struc	;pj_mcga_set_colors parameter structure
	jsc_esi dd ?
	jsc_edx dd ?
	jsc_ecx dd ?
	jsc_ebx dd ?
	jsc_ret	dd ?
	jsc_v	dd ?
	jsc_startix dd ?
	jsc_count dd ?
	jsc_cmap dd ?
jscp	ends

	push ebx
	push ecx
	push edx
	push esi

	mov ebx,[esp].jsc_startix
	mov ecx,[esp].jsc_count
	mov esi,[esp].jsc_cmap

st1:
	mov	dx,3c8h
	mov al,bl
	out dx,al
	inc bl
	inc dx
	jmp s1
s1:
	lodsb
	shr al,2
	out dx,al
	jmp s2
s2:
	lodsb
	shr al,2
	out dx,al
	jmp s3
s3:
	lodsb
	shr al,2
	out dx,al
	loop st1

	pop esi
	pop edx
	pop ecx
	pop ebx
	ret

pj_mcga_set_colors endp


	;pj_mcga_uncc256(Rast_mcga *v, Cbuf *csource)
	;set the color palette hardware from a compressed source 
	;of format:
	;WORD # of runs, run1, run2, ...,runn
	;each run is of form:
	;BYTE colors to skip, BYTE colors to set, r1,g1,b1,r2,g2,b2,...,rn,gn,bn
	public pj_mcga_uncc256
pj_mcga_uncc256 proc near
	push ebp
	mov ebp,esp
	push esi
	push edi
	push ebx

	xor eax,eax ;and hi bits of accumulator

	mov esi,[ebp+12]	;load the source compressed color data
	mov edi,0		;clear dest color index 
	lodsw
	mov ebx,eax   	;get the count of color runs
	test ebx,ebx
	jmp endscu
scu:
	lodsb		;get the colors to skip
	add edi,eax	;add to color index
	lodsb		;get the count of colors to set
	mov ecx,eax	;use it as a loop counter
	or  ecx,ecx	;test for zero
	jnz	sset1c
	mov ecx,256
sset1c:
	mov	edx,3c8h	;point dx to vga color control port
	mov eax,edi
	out dx,al	;say which color index to start writing to
	inc edi		;bump color index
	inc edx		;point port to vga color data
	;jmp ss1	;stall as per IBM VGA tech spec to give hardware time to settle
ss1:
	lodsb		;get red component
	shr al,2
	out dx,al	;tell the video DAC where it is at
	;jmp ss2	;stall some more for poor slow hardware
ss2:
	lodsb		;same same with green component
	shr al,2
	out dx,al
	;jmp ss3
ss3:
	lodsb		;same with blue
	shr al,2
	out dx,al
	loop sset1c

	dec ebx
endscu:
	jnz scu

	pop ebx
	pop edi
	pop esi
	pop	ebp
	ret	
pj_mcga_uncc256 endp


code	ends
	end

