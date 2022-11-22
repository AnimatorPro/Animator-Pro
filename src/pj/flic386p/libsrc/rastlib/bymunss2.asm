

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

;pj_unss2 decodes a FLI_LS chunk of a flic frame.  
	public pj_unss2
; pj_unss2(cbuf, screen, bpr, screen_seg)
; UBYTE *cbuf;  /* points to a FLI_LC chunk after chunk header. */
; UBYTE *screen;		  
; int screen_seg
pj_unss2 proc near
pubr struc
	pulc_edx	dd ?
	pulc_ecx	dd ?
	pulc_ebx	dd ?
	pulc_edi	dd ?
	pulc_esi	dd ?
	pulc_es		dd ?
	pulc_ebp	dd ?
	pulc_ret	dd ?
	pulc_cbuf	dd ?
	pulc_screen dd ?
	pulc_bpr 	dd ?
	pulc_seg	dd ?
	pulc_width	dd ?
pubr ends
	;save the world and set the basepage
	push ebp
	mov ebp,esp
	push es
	push esi
	push edi
	push ebx
	push ecx
	push edx


	mov eax,[esp].pulc_seg
	mov es,ax		;set destination segment
	mov esi,[esp].pulc_cbuf		;source is compressed stream
	mov edi,[esp].pulc_screen	;destination is screen in edi 
	mov ebp,[esp].pulc_bpr
	lodsw	;get the count of lines packets
	mov edx,eax
	xor ecx,ecx	;clear out hi bits of count
	jmp lpack

skiplines:			;come here to skip some lines
	push edx
	neg ax			
	mul ebp
	pop edx
	add edi,eax
	xor eax,eax		;clear hi bits of eax for later...
lpack:
ss2d:
	lodsw				;fetch line op
	test ax,ax
	jns do_opcount		; if positive it's an ss2 opcount
	cmp ah,0C0h		; if bit 0x40 of ah is on (unsigned >= C0h)
	jae skiplines  		; we skip lines 

					; if not put immediate dot at end of line
	mov ebx,[esp].pulc_width   		; width to ebx
	mov byte ptr es:[edi+ebx-1],al 	; put dot at screen + width - 1
	lodsw			; get following opcount for compressed data 
	test ax,ax
	jz line_is_done ; if no ops line is done
do_opcount:
	mov bx,ax		; number of ops in this line to bx
	push edi
ppack:
	lodsw			;5
	mov cl,al		;2
	add edi,ecx		;2
	test ah,ah		;2
	js prun			;?
	mov cl,ah		;2
	rep movsw
	dec bx
	jnz ppack
	pop edi
line_is_done:
	add edi,ebp			;Go to next line of destination.
	dec edx             ; --lineops
	jnz lpack           ; not done lpack
	jmp pend
prun:
	neg ah
	mov cl,ah
	lodsw
	rep stosw
	dec bx
	jnz ppack
	pop edi
	add edi,ebp			;Go to next line of destination.
	dec edx
	jnz lpack

pend:
	mov eax,esi	;return position in compression stream

	pop edx
	pop ecx
	pop ebx
	pop edi
	pop esi
	pop es
	pop	ebp
	ret	

pj_unss2 endp


code	ends
	end
