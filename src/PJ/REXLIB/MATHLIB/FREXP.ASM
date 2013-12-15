	include mathhost.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

MATHLIB_2JUMP frexp _r_ml_frexp math_frexp

code	ends
	end
