	include mathhost.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

MATHLIB_2JUMP asin _r_ml_asin math_asin

code	ends
	end
