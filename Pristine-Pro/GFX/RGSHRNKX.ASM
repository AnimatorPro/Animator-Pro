
CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

	public shrinkx
;void JREGS shrinkx(int width,int height,Pixel *spt,int sbpr,
;	Pixel *dpt, int dbpr, int shrinkby)
;Ram to Ram shrink in x dimension...
shrinkx proc near
shXp	struc
	shX_edi 	dd ?	;what's there from pushad
	shX_esi 	dd ?
	shX_ebp 	dd ?
	shX_esp 	dd ?
	shX_ebx 	dd ?
	shX_edx 	dd ?
	shX_ecx 	dd ?
	shX_eax 	dd ?
	shX_ret 	dd ?	;return address for function
	shX_width	dd ?	;1st parameter 
	shX_height	dd ?
	shX_spt		dd ?
	shX_sbpr	dd ?
	shX_dpt		dd ?
	shX_dbpr	dd ?
	shX_shrinkby	dd ?
shXp	ends
	pushad
	mov ebp,esp

	;convert bytes-per-row to mods...
	mov ebx,[ebp].shX_shrinkby
	mov eax,ebx
	mov ecx,[ebp].shX_width
	mul ecx
	sub [ebp].shX_sbpr,eax
	sub [ebp].shX_dbpr,ecx

	mov esi,[ebp].shX_spt
	mov edi,[ebp].shX_dpt
	mov eax,[ebp].shX_height	;outer loop counter in eax
	mov edx,[ebp].shX_width
	dec ebx						;ebx = shrinkby-1
shXouter:
	mov ecx,edx
shXinner:
	movsb
	add esi,ebx
	loop shXinner
	add esi,[ebp].shX_sbpr
	add edi,[ebp].shX_dbpr
	dec	eax
	jnz shXouter

	popad
	ret
shrinkx	endp


code	ends
	end
