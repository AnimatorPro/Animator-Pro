

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

	public ss2d

;unss2 decodes a FLI_LS chunk of a flic frame.  
	public unss2
; unss2(cbuf, screen, bpr, screen_seg)
; UBYTE *cbuf;  /* points to a FLI_LC chunk after chunk header. */
; UBYTE *screen;		  
; int screen_seg
unss2 proc near
pubr struc
	pulc_savebp	dd ?
	pulc_ret	dd ?
	pulc_cbuf	dd ?
	pulc_screen dd ?
	pulc_bpr 	dd ?
	pulc_seg	dd ?
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


	mov eax,[ebp].pulc_seg
	mov es,ax		;set destination segment
	mov esi,[ebp].pulc_cbuf		;source is compressed stream
	mov edi,[ebp].pulc_screen	;destination is screen in ebx
	mov ebp,[ebp].pulc_bpr
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
	js skiplines		;if negative means skip some lines
	mov bx,ax			; else it's the number of ops in this line
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
	add edi,ebp			;Go to next line of destination.
	dec edx
	jnz lpack
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

unss2 endp


code	ends
	end
