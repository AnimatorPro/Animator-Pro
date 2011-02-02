 Here's some misc updates.

 The poco stuff fixes the same poco bug that you fixed recently.
(Crashes on a "can't find file" error.)  I send mine because I know I
put comments in and updated the versions doc and all, I don't know if
you did all that on your end.

 In pdraccess, there's a minor bugfix (to support "variable rez" PDRs, a
concept that exists only in theory right now, I think.  Also a wee
integrity check just in case a misbehavin PDR ever again reports 0 width
or height like the BMP bug was doing.

 In BMP.C, there's just the wee change I told you about a couple weeks ago.

 And in TIFF.C, it now scales the palettes into the 0-65535 range when it
writes a palette-mapped file.

