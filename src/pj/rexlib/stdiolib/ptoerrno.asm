	include stdiolib.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

STDIOLIB_JUMP pj__get_pto_errno siol_pj__get_pto_errno

code	ends
	end
