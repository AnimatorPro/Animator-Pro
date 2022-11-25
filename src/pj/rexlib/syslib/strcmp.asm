	include aasyslib.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

SYSLIB_JUMP strcmp sysl_strcmp

code	ends
	end
