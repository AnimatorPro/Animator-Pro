;generated via makeasms.bat
	include raster.i
	include rastlib.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

RASTLIB_JUMP pj_unss2_rect RL_UNSS2RECT 

code	ends
	end
