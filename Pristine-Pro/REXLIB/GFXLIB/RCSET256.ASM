;generated via makeasms.bat
	include raster.i
	include rastlib.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

RASTLIB_JUMP pj_uncc256 RL_UNCC256 

code	ends
	end
