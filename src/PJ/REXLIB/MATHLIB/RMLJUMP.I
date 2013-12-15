	include mathhost.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

MATHLIB_2JUMP XXXX _r_ml_XXXX math_XXXX

code	ends
	end
