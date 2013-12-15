
	include vram.i

_text	segment	para public use32 'code'
	extrn _vram_put_hseg:near
	extrn _vram_double_hline:near
	assume cs:CGROUP,ds:DGROUP

	public ss2d

pubr struc
	pulc_savebp	dd ?
	pulc_ret	dd ?
	pulc_v		dd ?
	pulc_cbuf	dd ?
	pulc_ytable dd ?
	pulc_xoff	dd ?
	pulc_yoff	dd ?
	pulc_width  dd ?
pubr ends

set_last_dot proc near
;ebx has x position of dot
;al has dot
	ret
set_last_dot endp
setbank macro
	local inbank
;input: cl has bank #
;trashes: ax,dx
	cmp cl,vrambank
	jz inbank
	call _setbank
inbank:
	endm

;vram_unss2 decodes a FLI_LS chunk of a flic frame.  
	public vram_unss2
; vram_unss2(v, cbuf, ytable, xoff, yoff)
; UBYTE *cbuf;  /* points to a FLI_LC chunk after chunk header. */
; Ytable *ytable;		  
vram_unss2 proc near
	;save the world and set the basepage
	push ebp
	mov ebp,esp
	push es
	push esi
	push edi
	push ebx


	mov eax,PHAR_REAL_SEG
	mov es,ax		;set destination segment
	mov esi,[ebp].pulc_cbuf		;source is compressed stream
	lodsw	;get the count of lines packets
	mov edx,eax
	xor ecx,ecx	;clear out hi bits of count
	jmp lpack

skiplines:			;come here to skip some lines
	neg ax			
	add [ebp].pulc_yoff,eax
lpack:
ss2d:
	lodsw				;fetch line op
	test ax,ax


	jns do_opcount		; if positive it's an ss2 opcount
	cmp ah,0C0h		; if bit 0x40 of ah is on (unsigned >= C0h)
	jae skiplines  		; we skip lines 

					; if not put immediate dot at end of line
	mov ebx,[ebp].pulc_width   		; width to ebx
	dec ebx
	add ebx,[ebp].pulc_xoff
	call set_last_dot
	;;;mov byte ptr es:[edi+ebx-1],al 	; put dot at screen + width - 1
	lodsw			; get following opcount for compressed data 
	test ax,ax
	jz test_lline

do_opcount:
	push ax				;save number of ops in this line
	mov ebx,[ebp].pulc_yoff
	shl ebx,3
	add ebx,[ebp].pulc_ytable
	cmp [ebx].yta_split,0
	jnz split_line
	mov cx,[ebx].yta_bank
	push edx
	setbank
	pop edx
	mov edi,[ebx].yta_address
	add edi,[ebp].pulc_xoff
	pop bx			; Get # of ops in this line
	public ppack
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
	inc [ebp].pulc_yoff
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
test_lline:
	inc [ebp].pulc_yoff
	dec edx
	jnz lpack

pend:
	mov eax,esi	;return position in compression stream

	pop ebx
	pop edi
	pop esi
	pop es
	pop	ebp
	ret	


hsegp struc
	vph_v	dd ?
	vph_pixbuf	dd ?
	vph_x	dd ?
	vph_y	dd ?
	vph_width dd ?
hsegp ends

hlinep struc
	vdh_v	dd ?
	vdh_color	dd ?
	vdh_x	dd ?
	vdh_y	dd ?
	vdh_width dd ?
hlinep ends



	public split_line	;DEBUG
split_line:			;come here if this line is split into 2 banks...

;save scratch regs
	pop bx			;get line op-count
	push eax
	push ecx
	push edx
	sub esp,5*4		;space for parameters to hline and hseg
	mov eax,[ebp].pulc_v
	mov [esp].vdh_v,eax
	mov eax,[ebp].pulc_yoff
	mov [esp].vdh_y,eax
	mov eax,[ebp].pulc_xoff
	mov [esp].vdh_x,eax
split_loop:
	lodsw			;5	fetch skip count and data count
	movzx ecx,al	;3
	add [esp].vdh_x,ecx
	test ah,ah		;2	if data count negative it's a run
	js split_prun	;?
	movzx edi,ah
	add edi,edi
	mov [esp].vph_width,edi
	mov [esp].vph_pixbuf,esi
	call _vram_put_hseg
	add esi,edi
	add [esp].vdh_x,edi
	dec bx
	jnz split_loop
	jmp end_split

split_prun:
	neg ah
	movzx edi,ah
	add edi,edi	;convert word count to byte count
	mov [esp].vdh_width,edi
	lodsw		;fetch repeat word
	mov [esp].vdh_color,eax
	call _vram_double_hline
	add [esp].vdh_x,edi
	dec bx
	jnz split_loop

end_split:
	add esp,5*4		;clear hline/hseg parameter space
;restore scratch regs
	pop edx
	pop ecx
	pop eax

;decrement counters and jump to next line.
	inc [ebp].pulc_yoff
	dec edx
	jnz lpack
	jmp pend

vram_unss2 endp

_text	ends
	end
