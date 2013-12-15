	include stdiolib.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

STDIOLIB_JUMP fflush siol_fflush

code	ends
	end
