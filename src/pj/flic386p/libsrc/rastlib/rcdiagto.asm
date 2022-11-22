	include raster.i
	include rastlib.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

RASTLIB_JUMP pj_diag_to_ptable RL_DTO_PTAB 

code	ends
	end
