	include mathhost.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

MATHLIB_2JUMP sqrt _r_ml_sqrt math_sqrt

code	ends
	end
