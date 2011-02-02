	include aasyslib.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

SYSLIB_JUMP pj_rexlib_load sysl_pj_rexlib_load

code	ends
	end
