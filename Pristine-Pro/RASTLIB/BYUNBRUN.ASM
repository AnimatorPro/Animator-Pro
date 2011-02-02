	include raster.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP


public pj_bym_unbrun_rect

;void grc_unbrun_rect(const Raster *v, void *ucbuf, const LONG pixsize,
;					  Coor xorg, Coor yorg, Coor width, Coor height)
;
;   Uncompress data into a rectangular area inside raster using
;   byte-run-length compression scheme used in Autodesk Animator 1.0
;   for the first frame of a FLI. Note width and height must be the same
;	as compressed with.
;

pj_bym_unbrun_rect proc near

bubr struc
	bubr_savebp		dd ?
	bubr_ret		dd ?
	bubr_r			dd ?
	bubr_cbuf		dd ?
	bubr_pixsize	dd ?
	bubr_xorg		dd ?
	bubr_yorg		dd ?
	bubr_width		dd ?
	bubr_height		dd ?
bubr ends

	;save the world and set the basepage
	push ebp
	mov ebp,esp
	push es
	push esi
	push edi
	push ebx
	push ecx
	push edx

; get segment and pixel pointer from raster and origin coordinates

	mov ebx,[ebp].bubr_r	; fetch raster (bytemap) pointer
	mov es,[ebx].bym_pseg	; put pixles segment it in extra segment
	mov edx,[ebx].bym_bpr	; bytes per row to edx;

	mov eax,[ebp].bubr_yorg ; move y origin to eax
	imul eax,edx			; multiply by bytes per row
	add eax,[ebp].bubr_xorg ; add x origin to pixel pointer
	add eax,[ebx].bym_p	; add pixels pointer to edi giving first pixel

	mov ebx,eax				  ; pointer to line start - bpr in eax
	add ebx,[ebp].bubr_width  ; add width to get a max test pointer

	mov esi,[ebp].bubr_cbuf ;compressed buffer pointer to esi

	xor ecx,ecx		;clear hi bits of count register
	xor eax,eax		; clear eax unsigned uses follow
	cld				; clear direction flag in case someone mucked with it.

	sub ebx,edx	 ; decrement since we increment right off

line_loop:

	add ebx,edx				 ; advance tester by bpr to next line (plus width)
	mov edi,ebx
	sub edi,[ebp].bubr_width ; get pointer to next line start (tester - width)
							 ; put in edi

	inc esi ; skip over obsolete opcount byte

do_op_loop:

	lodsb	; load op/count
	test al,al
	jns stuffbytes

	neg al
	mov cx,ax ; get negated copy count
	rep movsb
	cmp edi,ebx  ; done with line yet?
	jb do_op_loop	; if below line max do next op

	dec [ebp].bubr_height ; done ?
	jnz	line_loop
	jmp short done

stuffbytes:

	mov cx,ax		;move count to cx
	lodsb			;value to repeat in al
	rep stosb		;store same value again and again...
	cmp edi,ebx	; done with line yet?
	jb do_op_loop	; if below line max do next op

	dec [ebp].bubr_height ; done ?
	jnz	line_loop

done:

	mov eax,esi	 ;return position in compressed stream
	pop edx
	pop ecx
	pop ebx
	pop edi
	pop esi
	pop es
    leave	; pop ebp move ebp to esp
	ret

pj_bym_unbrun_rect endp

code	ends
	end

