	include mathhost.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

MATHLIB_2JUMP ldexp _r_ml_ldexp math_ldexp

code	ends
	end
