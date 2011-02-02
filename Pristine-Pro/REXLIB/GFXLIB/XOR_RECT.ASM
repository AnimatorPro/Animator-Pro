;generated automatically via makeasms.bat

	include gfxlib.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

GFXLIB_JUMP pj_xor_rect gfxl_xor_rect

code	ends
	end
