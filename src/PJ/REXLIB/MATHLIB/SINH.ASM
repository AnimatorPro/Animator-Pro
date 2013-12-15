	include mathhost.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

MATHLIB_2JUMP sinh _r_ml_sinh math_sinh

code	ends
	end
