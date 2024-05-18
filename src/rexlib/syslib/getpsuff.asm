	include aasyslib.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

SYSLIB_JUMP pj_get_path_suffix sysl_pj_get_path_suffix

code	ends
	end
