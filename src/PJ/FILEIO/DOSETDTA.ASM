CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

include errcodes.i

	public pj_dset_dta
;	void pj_dset_dta(Fndata *dta);
pj_dset_dta proc near
jsep	struc
	jse_xxx	dd ?
	jse_ret	dd ?
	jse_dta dd ?
jsep	ends
	push edx
	mov edx,[esp].jse_dta
	mov ah,1ah
	int 21h
	pop edx
	ret
pj_dset_dta endp

code	ends
	end
