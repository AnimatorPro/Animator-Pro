

	include raster.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

;extern JREGS int pj_bym_get_dot(Vscreen *v, int x, int y);
;	memory screen get dot routine.
	public pj_bym_get_dot
pj_bym_get_dot proc near
bgdp	struc	;cdot parameter structure
	bgd_edi dd ?
	bgd_edx dd ?
	bgd_ret dd ?
	bgd_v	dd ?
	bgd_x	dd ?
	bgd_y	dd ?
bgdp ends
	push edx
	push edi

	;put screen address into eax
	mov edi,[esp].bgd_v
	mov eax,[esp].bgd_y
	mul [edi].bym_bpr	
	add eax,[esp].bgd_x
	add eax,[edi].bym_p

	;and bgd pixel
	mov al,[eax]
	and eax,0ffh	;clear hi bits of result

	pop edi
	pop edx
	ret
pj_bym_get_dot endp


;void JREGS pj_bym_put_dot(Bytemap *v, RCOLOR color, LONG x, LONG y);
;	Unclipped put dot for memory structure.
	public pj_bym_put_dot
pj_bym_put_dot proc near
_bymp	struc	;pj_bym_put_dot parameter structure
	_bym_edi dd ?
	_bym_edx dd ?
	_bym_ret dd ?
	_bym_v	dd ?
	_bym_color dd ?
	_bym_x	dd ?
	_bym_y	dd ?
_bymp ends

	push edx
	push edi

	;get pointer to screen structure
	mov edi,[esp]._bym_v

	;put line address into edx
	mov eax,[esp]._bym_y
	mul [edi].bym_bpr	
	add eax,[esp]._bym_x
	mov edi,[edi].bym_p
	add edi,eax

	;and set pixel to color
	mov eax,[esp]._bym_color
	mov [edi],al

	pop edi
	pop edx
	ret
pj_bym_put_dot endp

;void JREGS pj_bym_cput_dot(Bytemap *v, RCOLOR color, LONG x, LONG y);
;	Clipped put dot for memory structure.
	public pj_bym_cput_dot
pj_bym_cput_dot proc near
cdop	struc	;pj_bym_put_dot parameter structure
	cdo_edi dd ?
	cdo_edx dd ?
	cdo_ret dd ?
	cdo_v	dd ?
	cdo_color dd ?
	cdo_x	dd ?
	cdo_y	dd ?
cdop ends

	push edx
	push edi

	mov edi,[esp].cdo_v

	;load y coordinate and check it's within screen bounds
	mov eax,[esp].cdo_y
	or eax,eax	;test for y coordinate less than zero
	js	cdotz
	cmp	ax,[edi].bym_h
	jge	cdotz

	;put line address into eax
	mul [edi].bym_bpr	

	;load x coordinate and check it's within screen bounds
	mov edx,[esp].cdo_x
	or edx,edx	;test for x coordinate less than zero
	js	cdotz
	cmp	dx,[edi].bym_w
	jge	cdotz

	;finish computing screen address into ds:eax
	add eax,edx
	add eax,[edi].bym_p

	;and set pixel to color
	mov edx,[esp].cdo_color
	mov [eax],dl

cdotz:
	pop edi
	pop edx
	ret
pj_bym_cput_dot endp


code	ends
	end
