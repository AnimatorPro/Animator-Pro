	include procblit.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP



	;pj_tbli_line(Pixel *s, Pixel *d, Ucoor w, Tcolxldat *tcxl)
	public pj_tbli_line
pj_tbli_line proc near
tbllp	struc	;pj_tbli8 parameter structure
	tbll_edi dd ?	;what's there 
	tbll_esi dd ?
	tbll_ecx dd ?
	tbll_ret dd ?	;return address for function
	tbll_s	dd ?	;1st parameter 
	tbll_d	dd ?
	tbll_w	dd ?
	tbll_tcxl dd ?
tbllp ends
	push ecx
	push esi
	push edi

	mov eax, dword ptr [esp].tbll_tcxl ; eax = tcx 
	mov ah, byte ptr TCX_TCOLOR[eax]   ; ah = tcx->tcolor
	mov ecx,[esp].tbll_w
	mov esi,[esp].tbll_s
	mov edi,[esp].tbll_d
tbllp_loop:
	lodsb
	cmp al,ah	;is it tcolor?
	jz tbllp_skippix
	stosb
	loop tbllp_loop
	jmp tbllp_endl
tbllp_skippix:
	inc edi
	loop tbllp_loop
tbllp_endl:

	pop edi
	pop esi
	pop ecx
	ret
pj_tbli_line endp

code	ends
	end
