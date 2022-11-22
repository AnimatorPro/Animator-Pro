;include errcodes.i

;-----------------------------------------------------------------------------
;
;-----------------------------------------------------------------------------
data	segment dword public 'DATA'

	public pj_crit_errval

pj_crit_errval dw 0040H ; if not set by handler will be "critical" error
is_installed   db 0	; flag: are we currently installed?

data	ends

;-----------------------------------------------------------------------------
;
;-----------------------------------------------------------------------------
bss	segment dword public 'BSS'

old_real_vec dd ?			; dword, old real-mode vector.
old_prot_off dd ?			; old protected-mode vector offset.
old_prot_seg dw ?			; old protected-mode vector segment.

bss	ends

;-----------------------------------------------------------------------------
;
;-----------------------------------------------------------------------------
code	segment dword 'CODE'

CGROUP	group code
DGROUP	group data,bss

	assume cs:CGROUP,ds:DGROUP

	public pj_doserr_install_handler
	public pj_doserr_remove_handler
	public pj_dget_err

;*****************************************************************************
; replacement handler, turns off abort/retry/fail...
;*****************************************************************************

de_handler proc near
	mov	pj_crit_errval,di	; save error status for later.
	mov	al,3			; tell DOS to "Fail" w/o prompting.
	iretd
de_handler endp

;*****************************************************************************
;* routine to install our handler...
;*****************************************************************************

pj_doserr_install_handler proc near

	test	is_installed,0FFH	; have we been installed already?
	jnz	short #already_installed; yep, just punt.

	push	ebx
	push	es

	mov	cl,024H 		; vector number to get.
	mov	ax,2502H		; phar lap get vector function code.
	int	21H			; get old protected-mode vector.
	mov	old_prot_off,ebx	; save old vector offset value.
	mov	old_prot_seg,es 	; save old vector segment value.

	mov	cl,024H 		; vector number to get.
	mov	ax,2503H		; phar lap get vector function code.
	int	21H			; get old read-mode vector.
	mov	old_real_vec,ebx	; save old vector value.

	push	ds
	mov	cl,024H 		; vector number to set.
	lea	edx,de_handler		; address of handler in ds:edx.
	mov	ax,cs			; get code segment,
	mov	ds,ax			; store it into DS.
	assume	ds:nothing		; DS is now unusable for addressing.
	mov	ax,02506H		; phar lap set vector function code.
	int	21H			; do it.
	pop	ds
	assume	ds:DGROUP		; DS is usable again.

	pop	es
	pop	ebx

	mov	is_installed,0FFh	; indicate we're now installed.

#already_installed:

	ret

pj_doserr_install_handler endp

;*****************************************************************************
;* routine to de-install our handler...
;*****************************************************************************

pj_doserr_remove_handler proc near

	test	is_installed,0FFH	; first see if we're even installed,
	jz	short #not_installed	; if not, just punt.

	push	ebx
	push	ds

	mov	cl,024H 		; vector number to set.
	mov	ebx,old_real_vec	; old real mode vector.
	mov	edx,old_prot_off	; old protected-mode offset.
	mov	ds,old_prot_seg 	; old protected-mode segment.
	assume	ds:nothing		; DS is now unusable for addressing.
	mov	ax,2507H		; phar lap function code to set both
	int	21H			; real and protected mode vectors.

	pop	ds
	assume	ds:DGROUP		; DS is usable again.
	pop	ebx

	mov	is_installed,0		; indicate we're not installed anymore.

#not_installed:

	ret

pj_doserr_remove_handler endp

;*****************************************************************************
;* routine to return error status of last critical error...
;*****************************************************************************

pj_dget_err proc near

	push	ebx
	push	esi
	push	edi
	push	ds
	push	es

	xor	ebx,ebx 		;version zero...
	mov	ah,59H
	int	21H
	and	eax,0FFFFH		;mask any hi bits
	cmp	eax,0053H		;is error to be found in critical error?
	jne	#return_eax
	movsx	eax,word ptr pj_crit_errval ; use critical error value
	add	eax, 0013H

#return_eax:

	pop	es
	pop	ds
	pop	edi
	pop	esi
	pop	ebx
	ret

pj_dget_err endp

code	ends

	end
