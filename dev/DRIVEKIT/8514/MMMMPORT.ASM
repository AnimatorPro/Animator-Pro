
CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP


;	pj_8514_i86_inw(int port)
	public pj_8514_i86_inw
pj_8514_i86_inw proc near
inw_p struc
	inw_ret	dd ?
	inw_port dd ?
inw_p ends
	mov edx,[esp].inw_port
	xor eax,eax
	in ax,dx
	ret
pj_8514_i86_inw endp

;	pj_8514_i86_inb(int port)
	public pj_8514_i86_inb
pj_8514_i86_inb proc near
inb_p struc
	inb_ret	dd ?
	inb_port dd ?
inb_p ends
	mov edx,[esp].inb_port
	xor eax,eax
	in al,dx
	ret
pj_8514_i86_inb endp


;	pj_8514_i86_outb(int port, int value)
	public pj_8514_i86_outb
pj_8514_i86_outb proc near
otb_p struc
	otb_ret	dd ?
	otb_port dd ?
	otb_value dd ?
otb_p ends
	mov edx,[esp].otb_port
	mov eax,[esp].otb_value
	out dx,al
	ret
pj_8514_i86_outb endp


;	pj_8514_i86_outw(int port, int value)
	public pj_8514_i86_outw
pj_8514_i86_outw proc near
otw_p struc
	otw_ret	dd ?
	otw_port dd ?
	otw_value dd ?
otw_p ends
	mov edx,[esp].otw_port
	mov eax,[esp].otw_value
	out dx,ax
	ret
pj_8514_i86_outw endp

;	pj_8514_i86_swout(int port, void *data, int count)
	public pj_8514_i86_swout
pj_8514_i86_swout proc near
osw_p struc
	osw_savedi dd ?
	osw_ret	dd ?
	osw_port dd ?
	osw_data dd ?
	osw_count dd ?
osw_p ends
	push esi
	mov edx,[esp].osw_port
	mov esi,[esp].osw_data
	mov ecx,[esp].osw_count
	rep outsw
	pop esi
	ret
pj_8514_i86_swout endp


code	ends
	end
