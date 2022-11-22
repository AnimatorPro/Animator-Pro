	include mathhost.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

MATHLIB_2JUMP floor _r_ml_floor math_floor

code	ends
	end
