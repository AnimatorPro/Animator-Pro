
CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

;	i86_swout(int port, void *data, int count)
	public i86_swout
i86_swout proc near
osw_p struc
	osw_savedi dd ?
	osw_savedx dd ?
	osw_savecx dd ?
	osw_ret	dd ?
	osw_port dd ?
	osw_data dd ?
	osw_count dd ?
osw_p ends
	push ecx
	push edx
	push esi
	mov edx,[esp].osw_port
	mov esi,[esp].osw_data
	mov ecx,[esp].osw_count
	rep outsw
	pop esi
	pop edx
	pop ecx
	ret
i86_swout endp


code	ends
	end
