
CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP


;	i86_outb(int port, int value)
	public i86_outb
i86_outb proc near
otb_p struc
	otb_savedx dd ?
	otb_ret	dd ?
	otb_port dd ?
	otb_value dd ?
otb_p ends
	push edx
	mov edx,[esp].otb_port
	mov eax,[esp].otb_value
	out dx,al
	pop edx
	ret
i86_outb endp


code	ends
	end
