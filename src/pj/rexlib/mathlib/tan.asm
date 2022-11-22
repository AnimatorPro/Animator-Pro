	include mathhost.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

MATHLIB_2JUMP tan _r_ml_tan math_tan

code	ends
	end
