;generated via makeasms.bat
	include raster.i
	include rastlib.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

RASTLIB_JUMP pj__get_vseg RL_GET_VSEG 

code	ends
	end
