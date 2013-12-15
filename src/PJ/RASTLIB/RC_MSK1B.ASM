;generated via makeasms.bat
	include raster.i
	include rastlib.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

RASTMASK_JUMP pj__mask1blit RL_MASK1BLIT 

code	ends
	end
