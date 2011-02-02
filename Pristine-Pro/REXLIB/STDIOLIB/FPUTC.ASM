	include stdiolib.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

STDIOLIB_JUMP fputc siol_fputc

code	ends
	end
