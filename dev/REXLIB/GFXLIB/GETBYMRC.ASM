;generated automatically via makeasms.bat

	include gfxlib.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

GFXLIB_JUMP pj_rcel_bytemap_alloc gfxl_pj_rcel_bytemap_alloc

code	ends
	end
