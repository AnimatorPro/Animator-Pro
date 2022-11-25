	include aasyslib.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

SYSLIB_JUMP pj_rexlib_init sysl_pj_rexlib_init

code	ends
	end
