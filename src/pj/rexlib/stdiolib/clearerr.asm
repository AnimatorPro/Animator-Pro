	include stdiolib.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

STDIOLIB_JUMP clearerr siol_clearerr

code	ends
	end
