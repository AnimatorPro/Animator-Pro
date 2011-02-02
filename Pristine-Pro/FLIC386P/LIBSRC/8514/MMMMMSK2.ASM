	include a8514.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

;void mmmm_mask2blit(UBYTE *mbytes, LONG mbpr, 
;	Coor sx, Coor sy, Ucoor width, Ucoor height,
;	Vscreen *v, Coor dx, Coor dy,
;	Pixel oncolor, Pixel offcolor);
_pj_8514_mask2blit proc near
	public _pj_8514_mask2blit
mk2p struc
	mk2_ebp	dd ?
	mk2_ret dd ?
	mk2_mbytes dd ?
	mk2_bpr	dd ?
	mk2_sx dd ?
	mk2_sy dd ?
	mk2_width dd ?
	mk2_height dd ?
	mk2_v dd ?
	mk2_dx dd ?
	mk2_dy dd ?
	mk2_oncolor dd ?
	mk2_offcolor dd ?
mk2p ends
	push ebp
	mov ebp,esp
	push ebx
	push ecx
	push edx
	push esi
	push edi

;make sure have GP is free
           CLRCMD
           WAITQ     8                 ; v1.00

	pop edi
	pop esi
	pop edx
	pop ecx
	pop ebx
	pop ebp
	ret
_pj_8514_mask2blit endp

code	ends
	end
