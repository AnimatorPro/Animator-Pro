

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP


;pj_unlccomp decodes a FLI_LC chunk of a flic frame.  This is the main
;'delta' type chunk.  It contains information to transform one frame
;into the next frame where hopefully the next frame is mostly the same
;as the one frame.  This uses a combination of 'skip' compression and
;run length compression.  Each packet of this chunk
	public pj_unlccomp
; pj_unlccomp(cbuf, screen, bpr, screen_seg)
; UBYTE *cbuf;  /* points to a FLI_LC chunk after chunk header. */
; UBYTE *screen;		   /* 64000 byte screen */
; int screen_seg
pj_unlccomp proc near
pubr struc
	pulc_savebp	dd ?
	pulc_ret	dd ?
	pulc_cbuf	dd ?
	pulc_screen dd ?
	pulc_bpr 	dd ?
	pulc_seg	dd ?
pubr ends
linect equ dword ptr[ebp-4]
	;save the world and set the basepage
	push ebp
	mov ebp,esp
	sub esp,4
	push es
	push esi
	push edi
	push ebx
	push ecx
	push edx

	cld	;clear direction flag in case Aztec or someone mucks with it.
	xor eax,eax	;clear hi bits of accumulator

	mov eax,[ebp].pulc_seg
	mov es,ax		;set destination segment

	mov esi,[ebp].pulc_cbuf
	mov edi,[ebp].pulc_screen
	lodsw	;get the count of # of lines to skip
	mul [ebp].pulc_bpr
	add edi,eax
	lodsw		;get line count
	mov	linect,eax	;save it on stack
	mov	edx,edi	;keep pointer to start of line in dx
	xor	ah,ah	;clear hi bit of ah cause lots of unsigned uses to follow
linelp:
	mov	edi,edx
	lodsb		;get op count for this line
	mov bl,al  
	test bl,bl
	jmp endulcloop
ulcloop:
	lodsb	;load in the byte skip
	add edi,eax
	lodsb	; load op/count
	test al,al
	js ulcrun
	mov ecx,eax
	rep movsb
	dec bl
	jnz ulcloop
	jmp ulcout
ulcrun:
	neg al
	mov ecx,eax ;get signed count
	lodsb	  ;value to repeat in al
	rep stosb
	dec bl
endulcloop:
	jnz ulcloop
ulcout:
	add	edx,[ebp].pulc_bpr
	dec linect
	jnz	linelp
	mov eax,esi	;return position in compression stream

	pop edx
	pop ecx
	pop ebx
	pop edi
	pop esi
	pop es
	mov	esp,ebp
	pop	ebp
	ret	

pj_unlccomp endp


code	ends
	end
