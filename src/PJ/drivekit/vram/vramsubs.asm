
	include vram.i


_text	segment	para public use32 'code'
	assume cs:CGROUP,ds:DGROUP

	public isit_v7
isit_v7 proc near
	push ebx
;1st see if it's a Video 7 board of any kind
	mov ax,6f00h
	int 10h
	cmp bx,'V7'
	jnz isv_nope
;now check to make sure it have VRAM extension registers and so on
	mov ax,6f07h
	int 10h
	test ah,80h			;hi bit of ah set if VRAM
	jz isv_nope
	and bl,0f0h
	cmp bl,070h			;chip revisions 70h to 7Fh have our extensions.
	jnz isv_nope
	mov eax,1
isv7_done:
	pop ebx
	ret
isv_nope:			;return 0, not a VRAM-VGA
	xor eax,eax
	jmp isv7_done
isit_v7 endp

	public get_xmode	;get extended vga mode
get_xmode proc near
	push ebx
	mov ax,6f04h
	int 10h
	and eax,0ffh
	pop ebx
	ret
get_xmode endp


	public set_xmode	;set extended vga mode
set_xmode proc near
	push ebp
	mov ebp,esp
	push ebx
	mov ebx,8[ebp]
	mov ax,6f05h
	int 10h
	pop ebx
	pop ebp
	ret
set_xmode endp

EXTREG equ 3c4h

	public enable_ext		;enable VRAM specific extension regs.
enable_ext proc near
	mov dx,EXTREG
	mov ax,0ea06h
	out dx,ax
	ret
enable_ext endp


	public disable_ext		;disable VRAM extension regs
disable_ext proc near
	mov dx,EXTREG
	mov ax,0ae06h
	out dx,ax
	ret
disable_ext endp

	; busy-wait for start of vblank (Works with any VGA)
	public vram_wait_vsync
vram_wait_vsync proc near
	push edx
	mov	dx,3dah	;video status port
wnvb: 	; busy wait for top of screen or out of vertical blank
	in	al,dx
	test al,8
	jnz wnvb
wvb: 	; busy wait for bottom of screen or start of vertical blank
	in	al,dx
	test al,8
	jz wvb
	pop edx
	ret
vram_wait_vsync endp


_text	ends
	end
