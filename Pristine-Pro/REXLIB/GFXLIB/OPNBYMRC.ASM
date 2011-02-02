;generated automatically via makeasms.bat

	include gfxlib.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

GFXLIB_JUMP pj_rcel_bytemap_open gfxl_pj_rcel_bytemap_open

code	ends
	end
