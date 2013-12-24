;*****************************************************************************
;* GS_POE.ASM - Do-nothing set_gs routine for POE fliclib ussage.
;*
;*****************************************************************************

code	segment public dword use32 'CODE'
	assume cs:code

	public pj_set_gs
	public pj_get_gs
	public pj_get_ds

;*****************************************************************************
;* short pj_get_gs(void) - Return the value currently in the GS segreg.
;*****************************************************************************

pj_set_gs proc near			; do-nothing set is really a get
pj_get_gs proc near
	mov	ax,gs			; return value from GS segreg
	movzx	eax,ax
	ret
pj_get_gs endp
pj_set_gs endp

;*****************************************************************************
;* short pj_get_ds(void) - Return the value currently in the DS segreg.
;*****************************************************************************

pj_get_ds proc near
	mov	ax,ds			; return value from DS segreg
	movzx	eax,ax
	ret
pj_get_ds endp

code	ends
	end
