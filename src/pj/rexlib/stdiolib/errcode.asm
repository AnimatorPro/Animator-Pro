	include stdiolib.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

STDIOLIB_JUMP pj_errno_errcode siol_pj_errno_errcode

code	ends
	end
