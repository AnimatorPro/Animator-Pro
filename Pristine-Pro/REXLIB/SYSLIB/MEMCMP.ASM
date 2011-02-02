	include aasyslib.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

SYSLIB_JUMP memcmp sysl_memcmp

code	ends
	end
