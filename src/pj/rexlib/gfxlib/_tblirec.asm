;generated automatically via makeasms.bat

	include gfxlib.i

CGROUP	group	code

code	segment dword 'CODE'
	assume cs:CGROUP,ds:CGROUP

GFXLIB_JUMP pj__tblitrect gfxl__tblitrect

code	ends
	end
