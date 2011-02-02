	include aasyslib.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

SYSLIB_JUMP memset sysl_memset

code	ends
	end
