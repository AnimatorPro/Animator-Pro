	include aasyslib.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

SYSLIB_JUMP free sysl_free

code	ends
	end
