// common strings that appear all over the place

NameString common_strings
{
aalrmenu "=Ram6by6.fnt"  // font for low resolution menus
aamenu "aamenu.fnt"      // font for medium resolution menus
//aamenu "clean.a8"
aahrmenu "aahrmenu.fnt"  // font for high resolution menus
//aahrmenu "clean.a11"
continue "Continue"
yes "Yes"
no "No"
ok "OK"
cancel "Cancel"
load "Load"
save "Save"
any_key "Any key to continue >"
enter "Enter choice >"
unnamed "unnamed"       // name for unnamed files
pls_wait "Please Wait:"
}


// help strings and names for all the inks. xxx_n == name xxx_help == help
NameString ink_texts
{
xor_n "Xor",
xor_help "Exclusive or the current color register with image."
vgrad_n "V Grad",
vgrad_help "Vertical Gradient: apply cluster colors from top to bottom."
unzag_n "Unzag",
unzag_help "Antialias (soften) staircase edges.  Doesn't affect vertical "
	"or horizontal edges."
tile_n "Tile",
tile_help "Use the cel as a tile pattern."
sweep_n "Sweep",
sweep_help "Remove isolated pixels."
split_n "Split",
split_help "Slide every other scan line in opposite direction. Ink strength "
	"is pixel offset."
spark_n "Spark",
spark_help "Sparkle: change pixel to the sum of its four neighbors modulo "
	"256."
soften_n "Soften",
soften_help "Blur the image.  Repeat for increased effect."
smear_n "Smear",
smear_help "Drag colors gently in direction of mouse motion.  "
	"(See pull for a strong smear.)"
scrape_n "Scrape",
scrape_help "Scrape through to reveal Swap Screen."
rgrad_n "R Grad",
rgrad_help "Radial gradient: apply cluster colors in a concentric "
	"circular pattern."
pull_n "Pull",
pull_help "Drag colors strongly in direction of mouse motion.  "
	"(See smear for a gentle smear.)"
opaque_n "Opaque",
opaque_help "Use current color with no see-through."
lgrad_n "L Grad",
lgrad_help "Line-contour gradient:  Apply cluster colors horizontally "
	"contoured to edge."
jumble_n "Jumble",
jumble_help "Mix pixels randomly. Best results at low ink strength."
hollow_n "Hollow",
hollow_help "Reduce solid shapes to outlines.  Reverse of fill."
hgrad_n "H Grad",
hgrad_help "Horizontal gradient: apply cluster colors from left to right."
gray_n "Gray",
gray_help "Desaturate. Remove chroma. Push towards gray."
glow_n "Glow",
glow_help "Glow cluster: shift cluster colors one step."
glaze_n "Glaze",
glaze_help "Make translucent layers of color while mouse button is down. "
	"Works well with spray."
glass_n "Glass",
glass_help "Make a single transparent layer. Release mouse button or start "
	"a new shape for a new layer."
emboss_n "Emboss",
emboss_help "Bas-relief look.  Highlight upper-left set of edges.  Shadow the opposite set."
dark_n "Dark",
dark_help "Make colors darker.  Push towards black."
close_n "Close",
close_help "Close single pixel gaps in boundary lines of current color.  "
		   "Designed as an aid in creating fillable boundary shapes."
bright_n "Bright",
bright_help "Brighten:  increase apparent lighting."
add_n "Add",
add_help "Add register number of current color to image modulo 256."
and_help "Logical and the current color register with image."
merge_n "Merge"
merge_help "Transparently merge with swap screen."
minus_n "Minus",
minus_help "Subtract the current color register from image.",
or_n "Or",
or_help "Logical or the current color register with image.",
slice_n "Slice",
slice_help "Slide every other vertical line in opposite direction."
		   "Ink strength is pixel offset.",
smooth_n "Smooth",
smooth_help "Mild defocusing effect.  Like soften but more subtle.   "
			"Shifts image to the left half a pixel as a side effect.",
unbuzz_n "UnBuzz",
unbuzz_help "Blend pixels above and below to reduce flicker in interlace "
			"images.",
no_help "No help text."
}

// help strings and names for all the ptools xxx_n = name xxx_help = help
NameString tool_texts
{
text_n "Text"
text_help "Edit text in a boxed area of the screen.\n"
	"or load ascii files or fonts.",
streak_n "Streak"
streak_help "Freehand, brush-size line.\n"
	"May be broken depending on mouse speed.",
star_n "Star"
star_help	"Create a star shape.\n"
	"Define center first, then size and angle.",
spray_n "Spray"
spray_help	"Apply brush-size ink in random, circular pattern.\n"
	"Set speed and width below.",
spline_n "Spline"
spline_help "Curved spline.\n"
	"Set curvature with Tens, Bias and Cont sliders\n"
	"Load and save from poly tool.",
spiral_n "Spiral"
spiral_help "Spiral-shaped line.\n"
	"Set center, then angle, then turns.",
shape_n "Shape"
shape_help	"Drag mouse to apply freehand boundary.\n"
	"Then release to close shape.",
sep_n "Sep."
sep_help "Separate colors.\n"
	"Replace selected color(s) with current ink.",
rpoly_n "Rpoly"
rpoly_help "Regular polygon.\nAll sides same length.\n"
	"Set points below.",
poly_n "Poly"
poly_help "Irregular polygon.\nDefine one point at a time.",
petal_n "Petal"
petal_help "A flower-like shape.\nSet radius and points with sliders.",
oval_n "Oval"
oval_help "Define minor axis, then angle and major axis.",
move_n "Move"
move_help "Move a boxed area of the screen.\n"
	"Fills area left behind with key color.\n"
	"See also Copy.",
line_n "Line"
line_help "Apply ink in a straight line using current brush.",
gel_n "Gel"
gel_help "Freehand line with soft tapered edge.\n"
	"Effect varies with brush size.",
fillto_n "Fill To"
fillto_help "Click on boundary color.\n"
	"Then click anywhere within boundary to fill.",
fill_n "Fill"
fill_help "Apply ink to all pixels until stopped by a different color.",
edge_n "Edge"
edge_help "Click on a color.  Edges of that color will be "
	"fringed with current ink.",
driz_n "Driz."
driz_help "Drizzle:  Line gets thinner with faster motion."
	"  Best with a medium to large brush.",
draw_n "Draw"
draw_help "Hold down left button to apply unbroken line using current brush.",
circle_n "Circle"
circle_help "Make a circle.  Uses current brush if not filled.",
copy_n "Copy"
copy_help "Copy a boxed area of the screen.\nSee also Move"
box_n "Box"
box_help "Draw a rectangle.  Uses Current brush if not filled.",
no_help "No help text."
}

NameString vdriver_texts
{
V7VGA_0  "Video 7 640x400.  Works with most"
	" V7 VGA cards other than the VEGA VGA.",
V7VGA_1  "Video 7 640x480.  Works with most V7 VGA cards other"
	" than the VEGA VGA if they have 512K memory.",
V7VGA_2  "Video 7 720x540.  Requires card to have 512K memory",
V7VGA_3  "Video 7 800x600.  Works with VRAM card with 512K.  You'll need a"
	" 'Super VGA' monitor to display this."
}

NameString picdrive_texts
{
// flilo.pdr texts
flilo_info "Will create files readable by low resolution Animator."

// targa.pdr texts
targa_info "Reads most TARGA image formats.  "
   "Writes 24-bit RGB TARGA format.  "
   "\n\n"
   "To read truecolor TARGA files, or write files "
   "with different color or compression schemes, "
   "use the ANICONV program.  "
targa_fmt "Targa output format:\n"
	"Color Mapped\n"
	"Truecolor (RGB)\n"
	"Cancel\n"
targa_comp "Targa output compression:\n"
	"None\n"
	"Run Length\n"
	"Cancel\n"

// tiff.pdr texts
tiff_cmapped_info "Reads monochrome and color mapped formats.  "
   "Writes TIFF image files in color mapped format "
   "using LZW compression."
   "\n\n"
   "To read RGB TIFF files, or write TIFF files "
   "using different color options or compression "
   "schemes, use the ANICONV program."
tiff_grey_info "Reads monochrome and color mapped formats.  "
   "Writes TIFF image files in greys only "
   "using packbits compression."
   "\n\n"
   "To read RGB TIFF files, or write TIFF files "
   "using different color options or compression "
   "schemes, use the ANICONV program."
tiff_rgb_info "Reads monochrome and color mapped formats.  "
   "Writes TIFF image files in RGB format "
   "using packbits compression."
   "\n\n"
   "To read RGB TIFF files, or write TIFF files "
   "using different color options or compression "
   "schemes, use the ANICONV program."
tiff_fmt "TIFF output format:\n"
	"Grayscale\n"
	"Color Mapped\n"
	"Truecolor (RGB)\n"
	"Cancel\n"
tiff_comp "TIFF output compression:\n"
	"None\n"
	"Packbits\n"
	"LZW\n"
	"Cancel\n"

}

NameString idriver_texts
{
summa_opt0 "Summa input type\n"
	"Stylus\n"
	"Puck\n"
	"Cancel"

mmouse_opt0 "Select mouse speed\n"
	"Slow\n"
	"Medium\n"
	"Fast\n"
	"Cancel",

wacom_opt0 "Select device type\n"
	"Pressure sensitive stylus\n"
	"Side button stylus\n"
	"Puck\n"
	"Cancel",
}

// Definition of the main pull-downs
Pull home
{
"Ani",100
	{
	"About Ani",101
	"----",102
	"Browse",103
	"Frames",104
	"Optics",105,'o','O'        //two key equiv followed by one key equiv
	"Palette   ",106,'p','@'
	"Tools",107
	"Inks",108
	"Titling",109,'l'
	"Tween",110,'w'
	"----",111
	"Quit",112,'q','Q'
	}
"Flic",200
	{
	"New",201,'n','N'
	"Reset",202
	"Total Frames",203
	"Composite...",204
	"Join...",205
	"Effects...",206
	"Backwards...",207
	"Save Segment",208
	"Files...",209
	}
"Pic",300
	{
	"Clear",301,'c','X'
	"Restore",302
	"Apply Ink",303
	"Separate",304
	"View",305
	"Files...",306
	}
"Cel",400
	{
	"Anim Cel...",408
	"Clip",401,'C',0x9
	"Get",402,'G','G'
	"Lasso",403
	"Move",404,'M','M'
	"Paste",405,'P','`'
	"Stretch",406
	"Turn",407
	"Release",410
	"Files...",411,
	}
"Trace",500
	{
	"Blue Frame",501
	"Unblue Frame",502
	"Next Blue",503
	"Insert Tween",504
	"Erase Guides",505
	"Clip Changes",506
	"Repeat Changes",507
	"Loop Segment",508
	"Segment Flip     ",509,'S',0xa
	"Flip Five",510,'F','5'
	}
"Swap",600
	{
	"Clip",601
	"Trade",602
	"Paste",603
	"View",604
	"Release",605
	}
"Poco",700,';'
	{
	"Program",701
	"Use",702,'U','U'
	"----",703
	}
"Extra",800
	{
	"Mask...",801
	"Grid...",802
	"Record...",803
	"Settings...",804
	"Configure...",805
	"Screen Size",807,'z'
	"Info",806,'I','?'
	}
}

NameString widge_retry
{
ask "Hardware lock failure.\n"
	"Check cables and connections."
r "Retry"
c "Cancel"
}

NameString exit_pj
{
ask "Exit Autodesk Animator Pro?\n![1]",
y "Yes",
x "Abandon",
n "No",
}

NameString reset_flic
{
ask "Reset to ![1] X ![2] flic with\n"
	"default settings?\n![3]\n",
y	"Yes",
size "Set Size",
n	"No"
}

NameString new_flic
{
ask "New ![1] X ![2] flic?\n![3]\n",
y	"Yes",
size "Set Size",
n	"No"
}

Qchoice comm_port
{
"Select serial port for tablet"
"COM 1"
"COM 2"
"COM 3"
"COM 4"
"CANCEL"
}

Qchoice composit_mu
{
"Composite Functions"
"Overlay ![1]"
"Underlay ![1]"
"Cross-fade"
"Cancel"
}

Text save_changes_first { "Save changes to current file first?" }
Text save_changes_to_first { "Save changes to ![1] first?" }


Qchoice poco_program
{
"Programming"
"Edit"
"Run"
"Load"
"Save ![1]"
"Save As..."
"New"
"pocolib.txt"
"Exit",
}


Qchoice frame_del_mu
{
"Frame Delete Menu"
"Segment, Frames (![1] to ![2])"
"All but segment"
"Frames from Current"
"Exit menu"
}

NameString pdr_info
{
read_yes "Reads up to ![1] frames."
read_no  "Does not read image files."
write_yes "Writes up to ![1] frames."
write_no  "Does not write image files."

// final formatter for info box
// [1] is module name
// [2] is title test fom module
// [3] is read_yes or no
// [4] is write_yes or no
// [5] is info text from module

box_text "Image file module: ![1]\n"
		  "![2]\n\n"
		  "![3]\n"
		  "![4]\n\n"
		  "![5]"
}


Text max_frames { "The animation \"![1]\" contains ![2] frames.\n"
				   "Autodesk Animator Pro can only load ![3] frames.\n\n"
				   "Truncate file to ![3] frames and continue?" }
Text load_first {  "\"![1]\" is a ![2] frame animation\n"
				   "Load first frame as a picture?" }
Text squeeze_to { "Squeeze to how many colors?" }
Text nofont_char { "Character ![1] isn't in font" }
Text olay_abort { "Abort composite on\n" "Frame ![1] of ![2]" }
Text olay_start { "Composite flic starting this frame?" }
Text celseq_del { "Deleting Cel sequence." }
Text celseq_todisk { "Moving Cel sequence to disk." }

// time menu qnumbers
Text set_frame0 { "Set this frame to first frame?" }
Text move_ahead { "Move forward how many frames?" }
Text move_back { "Move back how many frames?" }
Text jump_frame { "Jump to frame number?" }
Text set_frames  { "How many frames do you want in this flic?" }
Text insert_after { "How many frames to insert after current frame?" }
Text del_from { "How many frames (starting from current frame) to delete?" }

Text gif_snap { "Save snapshot of screen to .GIF file:" }
Text comp_frames { "Enter transition frames." }

Text temp_path
{
"Set semicolon separated path of directories for temp files.\n"
}

Text vdriver_info
{
"![1]\n\n"
"File ![2] Mode ![3]\n"
"Pixel depth ![4]\n"
"Min width ![5] Max width ![6]\n"
"Min height ![7] Max height ![8]\n"
"Storage bytes available ![9]"
}

Text browse_info
{
"![1]\n"
"![2] frames in ![3] bytes\n"
"Average frame size ![4]\n"
"Speed ![5] jiffies/frame\n"
"Resolution ![6] x ![7]",
}

Text browse_cel_info
{
"![1]\n"
"(Animator 1.0 uncompressed cel)\n"
"1 frame in ![2] bytes\n"
"Resolution ![3] x ![4]",
}

Text about
{
"Autodesk Animator Pro\n"
"v. ![1]a    ![2]\n"
"Copyright 1992,1993 by Jim Kent.\n"
"![3] ![4], ![5]. ![6]\n"
"Produced exclusively for\n"
"Autodesk, Inc.\n"
"by Yost Group, Inc.\n"
" \n"
".PFB PostScript font files\n"
"Copyright 1993 by ANIM8, Inc.",
}

Text one_per_scroll
{
"To get one frame per scroll\n"
"you'd need ![1] frames.",
}


Text font_spacing {"Set extra space between characters."}
Text font_leading {"Set extra space between lines."}
Text set_font_height {"Set height of font."}
Text font_scale_only {"Only Postscript Type1 fonts (.PFB) are scaleable."}
Text sel_any_font {"Select any font"}
Text sel_str {"Select"}

Qchoice comp_cmap
{
"What about the color maps?"
"Combine color maps"
"Keep current colors"
"Use incoming colors"
"No fitting"
"Cancel"
}

Qchoice comp_past
{
"Composite past end?"
"Wrap at end"
"Chop at end"
}

Qchoice effects
{
"Special Effects"
"Shrink x2"
"Expand x2"
"Crop"
"Trails"
"Pixelate"
"Engrave"
"Lace"
"Grays Only"
"Blue Numbers"
"Cancel"
}

Qchoice trails
{
"Type of Trail Effect",
"Transparent",
"Zero Clear",
"Key Color Clear",
"Cancel",
}

Qchoice mask
{
"Mask Menu",
"Use",
"Create",
"Clip",
"Invert",
"View",
"Paste",
"Release",
"Files...",
"Exit Menu",
}

Qchoice grid
{
"Grid Menu"
"Use"
"Create"
"Paste"
"View"
"Angle Snap"
"Exit Menu"
}

Qchoice configuration
{
"Adjust Animator Configuration",
"Path for Temporary Files",
"Save Default Settings",
"Input Device (Mouse/Tablet)",
"Display Coordinates",
"Cancel",
}

Qchoice input_device
{
"Select graphics input device",
"Microsoft Compatible Mouse",
"Summagraphics MM 1201 12x12 Tablet",
"Custom input device",
"Select serial port for tablet",
"Cancel",
}

Qchoice justify
{
"Justify",
"Left",
"Right",
"Center",
"Fill Line",
"Cancel",
}

Qchoice record
{
"Macro Recording"
"Start Record"
"End Record"
"Use Macro"
"Repeat Macro"
"Realtime Record"
"Files..."
"Cancel"
}


Text info
{
"Autodesk Animator Pro Info:\n\n"
"![1] bytes free ![2] largest\n"
"this frame update ![3]\n"
"next frame update ![4]\n"
"total updates ![5] average ![6]\n"
"resolution: ![7] by ![8]"
}

// panel data

// common parts loaded at startup
NameString pj_muparts
{
kcol "K"
fillp "F"
otime "T"
pan "Pan"
ccycle "C"
zoom "Zoom" "\nz"

// f|s|a buttons in time slider
tseg_frame "F"
tseg_seg "S"
tseg_all "A"
pbrush "\nb" // key only for brush button

// texts, first char is 'T'
Trub_line "(![1] ![2]) wid ![3] hgt ![4] (![5] ![6]) deg ![7] rad ![8]",
Tbox_coor "(![1] ![2] ) (![3] ![4] )"
Trub_rect "![1] ![2]  (![3] ![4])  ![5] ![6]"
Trub_circle "R = ![1] D = ![2]"

}

NameString home_panel
{
title "Home"
clusid "AB"  // first char is clus #0 2nd #1
grid "Grid"
mask "Mask"
files "Files..."
redo "Redo" "\nr"
undo "Undo" "\n\b"
}

NameString home_keys
{
// items common to menus with standard header

ztogl "z"
status "?"
qpal "@"
color "\0\x3c"  // f2 key
prevf "\0\x4b"  // left arrow
nextf "\0\x4d"  // right arrow
play "\0\x50"   // down arrow
frame1 "\0\x48" // up arrow

// items in home menu only
help "\0\x3b"   // f1 key
togm " "
togb "b"
clrp "x"
undo "\b"
redo "r"
flp5 "5"
flpr "\r"
celcut "g"
celclp "\t"
celpas "`"
celmov "m"
// qsave "s"
qload "l"
qnew "n"
quit "q"
over_under "v"
dither "d"
key_color "k"
one_color "1"
two_color "2"
quit2 "\x1b" // escape
pouse "u"
optic "o"
qinsf "\0\x52"  // insert key
qkillf "\0\x53" // delete key

// testing items
debug "!"
test "/"
eatk "'"
trdp "+"
trdm "-"
rdir "="
}

Strings home_help
{
	"Animator Pro Keyboard Commands:"
	"Down Arrow  - Play flic"
	"Up Arrow    - Go to first frame"
	"Left Arrow  - Back one frame"
	"Right Arrow - Forward one frame"
	"Backspace   - Undo"
	"Escape      - Cancel/Quit"
	"Space       - Cancel/Toggle menus"
	"Return      - Flip through segment"
	"Tab         - Cel Clip"
	"Insert      - Insert a frame"
	"Delete      - Delete a frame"
	"F1 - Help"
	"F2 - Get color"
	"F3 - Set point"
	"F4 - Reuse point"
	"F5 - Save snapshot GIF"
	"a - Animator menu"
	"b - Toggle brush"
	"c - Cel menu"
	"d - Toggle dither"
	"e - Extras menu"
	"f - Flic menu"
	"g - Cut out cel"
	"k - Toggle key color clear"
	"l - Load flic"
	"m - Move cel"
	"n - New flic"
	"o - Optics panel"
	"p - Pic menu"
	"q - Quit"
	"r - Redo"
	"s - Swap menu"
	"t - Trace menu"
	"u - Use poco program"
	"v - Toggle over/under"     //new
	"x - Clear screen"
	"z - Toggle zoom"
	"1 - Toggle cel in 1 color"
	"2 - Toggle 2 color polygons"
	"5 - Flip through last 5 frames"
	"` - Paste cel"
	"? - Info"
	"@ - Palette Panel"
	"; - Poco menu"
}

Text editor_help
{
"You are in a simple text editor with word wrap.  "
"To exit the editor press <escape> or the right mouse button.  "
"Other editor controls are:\n"
"  arrow keys  - move cursor\n"
"  <page up>   - up one screen\n"
"  <page down> - down one screen\n"
"  <home>      - to start of line\n"
"  <end>       - to end of line\n"
"  <insert>    - toggle insert/exchange\n"
"  <delete>    - delete character\n"
"  <shift><delete> - delete line\n"
}

Text editor_move_cursor
{
"You can also position the cursor with the left mouse button."
}

Text editor_move_window
{
"You can move around the text editing window with the left mouse button.  "
"Click outside the text window to redefine the shape and size of the window.  "
"Click inside the text window to simply move it."
}


NameString zoom_panel
{
title "Zoom Settings" "\nq"
src "Source"
dest "Dest"
fullsz "Fullsize"
scale "SCALE"
// texts, first char is 'T'
Tzwin_title "![1] times ZOOM"
}

NameString browse_panel
{
cel_title "Browse Cels" "\nq"
fli_title "Browse Flics" "\nq"
load "Load" "\nl"
dir "Dir"
drive "Drive"
ok "Ok" "\n\r"
cancel "Cancel"
del "Delete" "\nd"
info "Info" "\ni"
play "Play" "\np"

// Texts Note first char is 'T'
Tcant_read "Can't Read!"
}

NameString cel_panel
{
title "Anim Cel" "\nq"
bluelast "Blue Last"
stream "Stream"
moveto "To cursor"
grid "Grid"
mask "Mask"
cel_frames "Cel Frames:"
undo "Undo" "\n\b"

// Texts Note first char is 'T'
Tscale "Stretch"
Tturn "Turn"
Tmove "Move",
Tpaste "Paste"
Tsprite "Sprite"
Tsetkey "Set Key"
}

NameString sprite_panel
{
title "Sprite"
rend "Render Sprite"
kill "Cancel Sprite"
}

NameString join_panel
{
title "Join" "\nq"
name1 "1: ![1]"
name2 "2: ![1]"
boxilate "Boxilate"
dissolve "Dissolve"
custom "Custom"
cut "Cut"
color "Colors"
blend "Blend"
col2 "2:"
col1 "1:"
rev "Reverse"
match "Match Size"
swapend "Swap Ends"
still "Still"
frames "Frames:"
preview "Preview"
rend "Render"
custinfo "Custom: ![1]  ![2] Frames, (![3] X ![4])"
}

NameString flisize_panel
{
title "Flic format menu"
ok "OK\n\r"
fullsize "Full screen (![1] X ![2])\nf"
cancel "Cancel"
defaultsz "![1] X ![2]"
hgt "Height"
wid "Width"
}

NameString driver_panel
{
title "\nq" // key only, text from outside
ok "Ok" "\n\r"
cancel "Cancel"
info "Info" "\ni"
}

NameString saveseg_panel
{
title "Segment Save" "\nq"
save "Save Segment"
cancel "Cancel"
}

NameString frames_panel
{
title "Frames" "\nq"
mul5 "*5"
mul3 "*3"
mul2 "*2"
jmp_d "D"
jmp_c "C"
jmp_b "B"
jmp_a "A"
marks "MARKS"
use_d "D"
use_c "C"
use_b "B"
use_a "A"
seg "Segment"
otime "Time Select"
speed "Play speed"
del "Delete" "\n\0\x53"
insert "Insert" "\n\0\x52"
}

NameString optics_panel
{
title "Optics" "\nq"
prop "Proportional"
szby_mouse "Mouse control"
by_mouse "Mouse control"
as_spin "Same as Spin"
szc_dflt "Default"
as_size "Same as Size"
spc_dflt "Default"
enlarge "Enlarge"
reduce "Reduce"
sz_both "Both"
sz_cent "Center"
sp_turns "Turns"
sp_axis "Axis"
sp_cent "Center"
sp_closed "Closed"
pa_open "Open"
pa_save "Save"
pa_load "Load"
pa_view "View"
pa_edit "Edit"
pa_clock "Clocked"
pa_samp "Sampled"
pa_poly "Polygon"
pa_spline "Spline"
path "Path"
move "Move"
size "Size"
spin "Spin"
clear_tk "Clear Track"
clear_mv "Clear Move"
continue "Continue"
use "Use" "\nr"
loop "Loop" "\nl"
view "View" "\nv"
kcol "K"
fillp "F"
t_all "A"
t_seg "S"
t_frm "F"
pen  "\nb" // key equiv only
}

NameString titles_panel
{
title "Titling" "\nq"
fill "Fill Line"
center "Center"
right "Right"
left "Left"
fcount "Frame Count"
by_char "By Character"
By_pix "By Pixel"
still "Still"
type_on "Type On"
across "Scroll Across"
scroll_up "Scroll Up"
ld_font "Load Font"
movewin "Place Window"
ld_text "Text Files..."
edit "Edit Text"
new "New Text"
render "Do Titling"
justify "Justify"
scroll "Scrolling"
movement "Movement"
}

NameString multi_panel
{
title "Time select" "\nq"
nextcel "Next Cel"
complete "Complete"
out_slow "Out slow"
in_slow "In slow"
reverse "Reverse"
ppong "Ping-Pong"
still "Still"
render "Render" "\nr"
preview "Preview" "\np"
cancel "Cancel"
key "K"
msk "M"
fill "F"
otime "T"
pen "\nb" // key only

// two sets with 't' (to) and 'f' (from)
tall "To All" "\na"
fall "From All" "\na"

tseg "To Segment" "\ns"
fseg "From Segment" "\ns"

tframe "To Frame" "\nf"
fframe "From Frame" "\nf"
}

NameString files_panel
{
title "Files" "\nq"
record "Record"
mask "Mask"
settings "Settings"
optic "Optic"
path "Path"
poly "Polygon"
font "Font"
text "Text"
tween "Tween"
pal "Palette"
cel "Cel"
pic "Picture"
flic "Flic"
del "Delete" "\nd"
save "Save" "\ns"
load "Load" "\nl"
exit "Exit" "\ne"
}

NameString fscroll_panel
{
ok "\n\r" // text is input
cancel "Cancel"
plus "+" "\n+"
wild "Wildcard:"
dir "Dir:"
file "File:"
}

NameString palette_panel
{
title "Palette" "\nq",
hls "HLS",
rgb "RGB",
b "B",
a "A",
clus "Cluster",
all "All",
t "T",
fit "Fit",
undo "Undo" "\n\b",
pen "\nb"
}


NameString option_panel
{
title "\nq"
help "Help"
clusid "AB"  // first char is clus #0 2nd #1
redo "Redo" "\nr"
undo "Undo" "\n\b"
}

NameString tool_panels
{
tfont "Font"
tsave "Save"
tload "Load"
tedit "Edit"
tplace "Reuse"
tjust "Justify"
copy "Copy"
one_color "One Color" "\n1"
under "Under" "\nv"
sep_boxed "Boxed"
sep_close "Near"
sep_range "Cluster"
sep_single "Single"
os_psens "Pres Sens"
biastag "Bias"
conttag "Cont"
tenstag "Tens"
c2curve "2 Color" "\n2"
fcurve "Filled"
ccurve "Closed"
ecurve "Reuse"
color2 "2 Color" "\n2"
filledp "Filled"
bcolor2 "2 Color" "\n2"
filledb "Filled"
fpg_files "Files"
fpg_o1mpg_editp "Reuse"
fpg_closed "Closed"
fpg_color2 "2 Color" "\n2"
fpg_filledp "Filled"
ompg_color2 "2 Color" "\n2"
ompg_filledp "Filled"
o1mpg_color2 "2 Color" "\n2"
o1mpg_filledp "Filled"
// texts, first char is 'T'
Tnthresh "Near Threshold"
Tswid "Spray Width"
Tsair "Air Speed"
Tbevel "Bevel"
Tomppts "POINTS"
Tosrpts "POINTS"
Tinrad "INNER RADIUS RATIO"
}

Text copy_has_moved
{
"The Move tool has been divided into two tools: Copy and Move."
}

NameString brush_panels
{
title "Brush menu" "\nq"
angle "Angle"
bsize "Brush Size"
// texts, first char is 'T'
Tgel_size "Gel Brush Size"
}

NameString ink_panels
{
dither "Dither"
rgcent "Center"
// texts, first char is 'T'
Tstrength "Ink Strength"
}

NameString tween_panel
{
title "Tween" "\nq"
grid "Grid"
mask "Mask"
tool "Tools"
// texts start with 'T'
Ttoolname "Tween"
}

NameString font_panel
{
title "Fonts" "\nq"
ok	"Load\n\r"
cancel "Cancel"
dir "Font Directory\nd"
space "Spacing\ns"
lead "Leading\nl"
height "Height\nh"
unzag "Unzag\nu"
}

// pull down data

Pull optics
{
"Presets",100
	{
	"Clear All",101
	"Pull Back",102
	"Spin",103
	"Twirl",104
	"Whirl",105
	"Spin Small",106,'m'
	"Squash",107,'u'
	"Files...",108
	"----",109
	"Quit Optics",110
	}
"Movement",200
	{
	" In Slow",201
	" Out Slow",202
	" Still",203
	" Ping-Pong",204
	" Reverse",205
	" Complete",206
	}
"Element",300
	{
	" Flic",301
	" Cel",302
	" Polygon",303
	" Spline",304
	" Tween",305
	"----",306
	" Set Tween",307,'n'
	" Set Cel",308,'l'
	" Outline",309
	}
}

Text optics_str {"Optics"}


Pull palette
{
"Palette",100
	{
	" Restore",101
	" Cycle Draw",102
	" One Palette",103
	" Menu Colors",104
	" Files...",105
	}
"Cluster",200
	{
	"Get Cluster",201
	"Pick Cluster",202
	"Unused Colors",203
	"Line Cluster",204
	"Find Ramp",205
	"Near Colors",206
	"Invert",207
	"Ping-Pong",208,'o'
	"Reverse",209
	}
"Arrange",300
	{
	"Luma Sort",301
	"Spectrums",302
	"Gradients",303
	"Cycle",304
	"Trade Clusters",305
	}
"Value",400
	{
	"Squeeze",401
	"Ramp",402
	"Tint",403
	"Negative",404
	"Use Cel",405
	"Default",406
	"Cut",407
	"Paste",408
	"Blend",409
	}
}


Text color_fit {"Color fit screen?"}
Text use_mc {"Use this color set for menus?"}
Text bad_colors {"Sorry, menus wouldn't be visible...."}
Text one_palette {
	"Make all frames use one color map?\n"
	"Takes a while....\n"
	}
Text no_palette {"\"[1]\" has no palette chunk!"}

Qchoice menu_colors
{
"Set Menu Colors"
"Default Grays"
"Blues"
"Classic"
"Burgundy"
"Spring"
"Inferno"
"Last 5"
"Cancel"
}

Text many_colors { "Too many colors, try fewer frames." }

Pull tween
{
"Tween",100
	{
	"Undo",101,'u','\b'
	"View Once",102
	"View Loop",103,'l'
	"Render",104
	"Trail",105
	"End to Start",106
	"Swap Ends",107
	"Clear Tween",108
	"Files...",109
	"----",111
	"Quit Tween",110
	}
"Shape",200
	{
	" Oval",206
	" Petal",204
	" Polygon",201,'n'
	" Rpoly",205
	" Shape",202,'h'
	" Star",203,'t'
	"----",207
	"Inside Out",208
	"Use Last",209
	"Load",210
	"Save",211
	}
"Move",300
	{
	" Move Point",301
	" Magnet",302,'g'
	" Blow",303
	" Move Shape",304,'o'
	" Move Tween",305,'v'
	" Size Shape",306,'i'
	" Size Tween",307,'z'
	" Link Points",308
	" Clear Links",309
	}
"Options",400
	{
	" Closed",401,'d'
	" Spline",402,'n'
	" Two Color",403,'w'
	"----",404
	" In Slow",405
	" Out Slow",406
	" Still",407
	" Ping-Pong",408
	" Reverse",409
	" Complete",410
	}
"Active",500
	{
	" Start",501
	" End",502
	" Both",503
	}
}

NameString tween_end
{
save "Save which end?",
load "Load which end?",
st "Start",
end "End",
esc "Cancel",
}

Text twe_trsteps { "How many steps in tween trail?" }

NameString twe_trail
{
ask "Keep trails (no undo)?",
kp "Keep",
vu "View",
esc "Cancel",
}

Pull cel
{
"Cel  ",100
	{
	"Clip",101
	"Get",102
	"Lasso",103,'O'
	"Browse",109,
	"Load",104
	"Save",105
	"Save Cursor",106,'U'
	"----",108
	"Quit Anim Cel",107
	}
"Position",200
	{
	"To Center",201,'C'
	"Stretch",202
	"Rotate",203
	"Mirror",204
	"Restore",205,'E'
	}
"Options",300
	{
	"*Clear Key Color",301
	"*Fit Colors",302
	"*Render Under",303,'R','V'
	"*One Color",304,'O','1'
	"*Next Cel",305
	}
}

Qchoice cel_stretch
{
"Stretch Cel"
"To Fill Flic"
"To Fill Box"
"Unstretch"
"Cancel"
}

Qchoice cel_turn
{
"Rotate Cel"
"90 Degrees Clockwise"
"Counter Clockwise 90"
"Unrotate"
"Cancel"
}

Qchoice cel_mirror
{
"Mirror Cel"
"Horizontal Axis"
"Vertical Axis"
"Other Axis"
"Cancel"
}

// driver menu title strings
Text pdrpic_hdr {"Picture File Format Menu"}
Text pdrfli_hdr {"Flic File Format Menu"}
Text idr_scroll {"Custom input (mouse/tablet) driver"}

//Titles to file requestor menu strings
Text load_block {"Load Block"}
Text save_block {"Save Block"}
Text load_cel {"Load Cel?"}
Text save_cel {"Save Cel?"}
Text load_comp_over {"Composite overlay"}
Text load_comp_under {"Composite underlay"}
Text load_comp_cross {"Composite cross fade"}
Text load_fli {"Load Flic?"}
Text load_fli_mask {"Load Flic to use as animated matte"}
Text save_fli {"Save flic to ![3]"}
Text save_fli_back {"Save backwards to ![3]"}
Text save_fli_seg {"Save frames (![1] to ![2]) to ![3]"}
Text load_mac {"Load Macro Recording File?"}
Text save_mac {"Save Macro Recording File?"}
Text load_msk {"Load Mask?"}
Text save_msk {"Save Mask?"}
Text load_opt {"Load Optics Move?"}
Text save_opt {"Save Optics Move?"}
Text load_pal {"Load Palette?"}
Text save_pal {"Save Palette?"}
Text load_pic {"Load Picture?"}
Text save_pic {"Save Picture to: "}
Text load_path {"Load Path File?"}
Text save_path {"Save Path File?"}
Text load_poly {"Load Polygon File?"}
Text save_poly {"Save Polygon File?"}
Text load_set {"Load Settings File?"}
Text save_set {"Save Settings File?"}
Text load_shape {"Load Shape file?"}
Text save_shape {"Save Shape file?"}
Text load_text {"Load Text?"}
Text save_text {"Save Text?"}
Text load_tween {"Load Tween File?"}
Text save_tween {"Save Tween File?"}
Text save_cursor {"Save Cursor?"}
Text load_poco {"Load Poco Program?"}
Text poco_qfile {"Select File:"}
Text save_poco {"Save Poco Program?"}
Text use_poco {"Use Poco Program?"}
Text use_str {"Use"}
Text del_file {"Delete file?"}
Text del_str {"Delete"}

// LOTS and LOTS of error messages

Text pj_help
{
"There are a number of command line options for Animator Pro.\n"
"The simplest are:\n"
"   ani                       Simply runs Animator Pro\n"
"   ani someflic.flc          Pre-loads someflic.flc.\n"
"   ani someprog.poc          Uses Poco program someprog.poc.\n"
"You may also use the switches below singly or in combination:\n"
"   -poc poco_path.any_suffix\n"
"   -cfg config-file_path\n"
"   -drv video-driver mode#\n"
"   -flic flic-file-name\n"
"   -mem|-m #of-bytes-of-memory-to-use\n"
"For example:\n"
"   ani -drv mcga.drv 0 -flic mrnumo.flc -mem 2000000\n"
"would run Animator Pro using mode 0 of the mcga driver (320x200 screen)\n"
"and load up mrnumo.flc while restricting the program to using two million\n"
"bytes of dynamic memory.\n"
}


// misc stack strings

Text screen_menu {"Screen Format Menu"}
Text dirty_file {"(You have ![1] unsaved changes.)"}
Text load_flic2 {"Load flic 2 to merge with ![1]"}

// continu box texts

Text pick_entry {"Please select entry or \"![1]\"."}
Text no_err_file { "Can't find Poco error file, oops..." }
Text cant_find {
	"Sorry Autodesk Animator Pro can't find:\n"
	"![1]\n",
}
Text all_colused {"No unused colors."}
Text text_notfnd {"![1] not found"}
Text no_troom {"Not enough room on temporary path."}
Text no_comm {"Comm port #![1] not found."}
Text truncated { "![1]\nFile truncated!" }
Text no_macro {"No macro recording defined."}
Text rot_cursor {"Can't save rotated or stretched cursors."}
Text newconfig {
	"Program isn't configured.\n"
	"Assuming defaults."
}

// qreq number texts

Text rot_grid {"Rotational constraint in degrees"}
Text max_blend {"Max blend percent?"}
Text max_tint {"Max tint percent?"}
Text near_sep {"Cluster near threshold?"}
Text clus_unused {"Number of unused colors for cluster?"}
Text macro_repeat {"Repeat macro how many times?"}
Text boxil_ht { "Set \"boxil\" height" }
Text boxil_wid { "Set \"boxil\" width" }
Text slatsize { "Set mask \"slat\" spacing" }
Text quant_wid {"Width of quantization unit?"}
Text quant_hgt {"Height of quantization unit?"}

// yes no box texts

Text load_new {"Load in new flic?\n![1]"}
Text one_scroll {"Make one frame per scroll (![1] frames)?"}
Text text_find {
	"No match after cursor.\n"
	"Find matches from start of file?"
}
Text animsv_exact {
	"Can't save ![1] X ![2],\n"
	"![3] color ![4] frame animation\n"
	"exactly:\n\n"
	"Can only save ![5] X ![6],\n"
	"with ![7] colors and ![8] frames.\n\n"
	"Save it anyway?"
}
Text save_conv {
	"Convert and save ![1] frame animation\n"
	"to ![2]?\n\n"
	"This takes a while."
}
Text picsv_exact {
	"Can't save ![1] X ![2], ![3] color\n"
	"picture exactly:\n"
	"Can only save ![4] X ![5], ![6] colors\n"
	"(the middle part of picture).\n\n"
	"Save it anyway?"
}

Text full_flic {"Start with new full screen flic?"}
Text noconfig { "Configuration file \"![1]\" not found.\n\n"
				  "Create default file?" }
Text badconfig { "\"![1]\" is not a configuration file\n\n"
				  "Overwrite with default file?" }
Text first_only {
	"\"![1]\" is a ![2] frame animation.\n"
	"It can be Loaded as a flic to convert frames.\n\n"
	"Load and use first frame only?"
}
Text fliload_slow  {
	"Load and convert ![1] frame animation\n\"![2]\"\n"
	"from non-flic file?\n\n"
	"This takes a while."
}

Text frame_del {"Delete this frame?"}
Text mask_del {"Release (delete) Mask?"}
Text alt_delete {"Release (delete) Swap screen?"}
Text cel_del {"Release (delete) Cel?"}

Text save_abort {"Abort saving of\n\"![1]\"?"}
Text macro_abort {"Abort macro playback?"}
Text clip_abort {"Abort multi-frame CEL clip?"}
Text join_pabort {"Abort Join Preview on\nFrame ![1] of ![2]?"}
Text join_abort {"Abort Join on\nFrame ![1] of ![2]"}
Text auto_abort {"Abort multiple-frame rendering\non frame ![1] of ![2]?"}
Text load_abort { "Abort load of \"![1]\"\non frame ![2] of ![3]?" }
Text poco_abort {"Abort poco program?"}

Text chop_end_ok {"This will chop off ![1] frames at the end ok?"}
Text really_del {"![1]\nReally delete file permanently?"}
Text over_old {"Overwrite old\n![1]?"}
Text setting_cmap {
	"There is no palette chunk present.\n"
	"Load default colors?"
}

// top window texts

Text top_celpos {"XY ![1],![2] (![3],![4]) W% ![5] H% ![6]"}
Text top_startc {"Start ![1]"}
Text top_secondc {"Start ![1] Colors ![2] Stop ![3]"}
Text top_lastc {"Start ![1] count ![2] this ![3]"}
Text start_rgb {"Start #![1] (R ![2] G ![3] B ![4])"}
Text stop_rgb {"Stop #![1] (R ![2] G ![3] B ![4])"}
Text tinting_rgb {"Tinting source #![1] (R ![2] G ![3] B ![4])"}
Text nearc_rgb {"Near Color #![1] (R ![2] G ![3] B ![4])"}
Text top_source {"Source ![1]"}
Text top_color {"Color #![1] (R ![2] G ![3] B ![4])"}
Text top_refit {"Refitting frame ![1]: ![2] to go..."}
Text top_cpack {"![1] total colors used, packing..."}
Text top_deg  {"![1] degrees"}
Text top_curs {"Select cursor \"hot spot\"."}

// wait box texts

Text tfile_move {"moving ![1]\n to ![2]..."}
Text tfile_toram {"moving ![1] into ram...\nsize ![2] rfree ![3]"}
Text wait_quit {"saving program state..."}
Text wait_reorder {"re-ordering flic frames..."}
Text wait_ringing {"couldn't load entire flic.\nCalculating ring frame..."}
Text olay_cleanup {"cleaning up fli overlays ..."}
Text wait_save {"saving ![1]"}
Text wait_fdread {"Reading floppy ![1]:"}


Qchoice over_inc_old
{
"File \"![1]\" exists",
"Increment to \"![2]\"",
"Overwrite old \"![1]\"",
"Cancel"
}

NameString errlines
{
// PJ error context messages
alloc_zero "Zero memory request!"
alt_alloc "Can't allocate Swap screen."
auto_apply "Auto apply failure."
auto_tflx "Error reading temporary file \"![1]\"."
auto_tflx2 "Error in temporary file \"![1]\"."
bro_info "Can't get info for flic \"![1]\"."
bro_menu "Error in browse menu."
bro_stamp "Can't make postage stamp for flic:\n   ![1]"
bro_view "Can't view flic \"![1]\"."
cant_cpack "Can't Squeeze colors"
cant_create "Can't create \"![1]\"."
cant_delete "Can't delete \"![1]\"."
cant_expand "Can't expand file"
cant_find "Can't find \"![1]\"."
cant_load "Can't load \"![1]\"."
cant_load2 "Couldn't load ![1] ![2]"
cant_save "Can't save \"![1]\"."
cel_change "changetocel() only takes bytemaps"
cel_clip "Can't clip CEL"
cel_mfclip "Can't clip multi-frame CEL"
cel_display "Error displaying CEL"
cel_install "Error installing cel menu."
cel_menu "Error in cel menu."
cel_play "Can't play cel"
cel_preview "Can't preview"
cel_rotate "Cel Rotate failure."
cel_stretch "Cel stretch failure."
comp_load "Can't load \"![1]\" for compositing."
cust_rast "Invalid call in custom raster"
dos_command "Can't load DOS command processor"
driver_info "Can't get info for ![1] mode ![2]"
driver_open "Can't open screen with driver\n\"![1]\"\n![2] x ![3] mode ![4]\n"
driver_query "Can't query module \"![1]\"\nEntry ignored:"
edit_window "Can't open editor window,"
fatal_create "Fatal error: Can't create \"![1]\"."
fatal_exit "Fatal error: exiting"
fcel_load "Can't load CEL flic \"![1]\""
fcel_reopen "Can't reopen file for \"![1]\""
fcel_temp "Can't load cel tempfile \"![1]\""
first_ok "Error reading: first frames may be usable."
fli_big  "Can't open ![1] X ![2] flic. (Try setting flic size to screen size.)\n"
fli_delete "Can't delete frames"
fli_frame "Error reading flic frame"
fli_insert "Can't expand flic to insert altered frame.\n"
		   "Save current frame to picture if possible."
fli_load "Can't load flic from ![1]"
fli_max "The maximum number of frames allowed for a flic is ![1]."
fli_reorder "Can't re-order frames"
fli_savef "Error saving frames to \"![1]\""
home_menu "Error loading quick menu!!"
ink_load "Can't load \"![1]\" ink driver."
inks_100 "Can only have 100 inks"
join_boxsize "error setting boxil size."
join_flis "Can't join flics."
join_load "Unable to load \"![1]\" for join."
join_mask "Unable to load mask from flic \"![1]\""
join_menu "Error loading Join Panel"
join_remask "Unable to re-load mask.\n\"![1]\""
low_dim  "Dimension must be between ![1] and 32767."
macro_create "Can't create macro file \n\"![1]\""
macro_load "Can't load macro file \"![1]\""
macro_read "Macro file read error: - Aborted."
macro_use "Unable to use macro."
macro_write "Macro file write error: - Aborted."
mask_alloc "Can't alloc mask"
mod_unable "Unable to use module \"![1]\""
nomouse "Unable to load mouse driver\n![1]\nUse arrow keys to move cursor.\n"
	"Left shift for left button.\nRight shift for right button."
no_smu_class_item "Can't load ![1] ![2] ![3]"
no_sprite "No more sprite frames available."
opt_cont "Couldn't continue move."
opt_path "Couldn't OptSetPath"
opt_view "Can't preview optics motion in wireframe"
outa_range "Value must be between ![1] and ![2]"
pj_min_memory "This program requires at least 2 megabytes of memory to\n"
			  "run in low resolution,  4 megabytes in medium (640x480) res,\n"
			  "and still more for higher resolutions."
poco_compile "Poco compile error:"
poco_edit "Can't open poco editor window,"
poco_leaf "loading Poco menu leaf"
poco_run "Poco runtime error:"
pull_load "Couldn't load Pull defs of ![1] from menu file"
qcolor "Qcolor"
ram_dir "Error reading ram directory"
readonly_pdr "Can't use module:\n\"![1]\"\n"
			 "to save images.  It can only READ\n"
			 "image files.",
redo_points "Error saving redo input points."
redo_poly "Can't redo text."
redo_save "Can't save redo."
redo_text "Can't redo text."
resource_init "Can't initialize resources."
restore_tempc "restoring temp cel"
save_tempc "Couldn't save temp cel."
screen_menu "Failure loading image file module menu."
seq_corrupt "Fatal error: sequence corrupted."
seq_lost "Critical Error: sequence lost"
seq_term "Error: sequence terminated."
set_load "Can't read settings from\n\"![1]\"\n, using defaults"
smu_choice "Can't load choice menu."
smu_number "Can't get number."
smu_string "Can't get string."
tdev_badchar "Invalid char '![1]' in temp-path element \"![2]\"."
tdev_delim "'\\' required after device in temp-path element \"![1]\"."
tdev_device "Device required in temp-path element \"![1]\"."
tdev_exist "Can't find or create path element \"![1]\"."
tdev_floppy "Can't use floppies for temp files in: \"![1]\"."
tdev_root "Can't use the root directory for temp files: \"![1]\"."
temp_copy "Can't copy temp files to:\n![1]."
tflx_buffers "Can't get buffers for ![1] x ![2] flic.  Try smaller size."
tflx_celdata "Cdata size err sz ![1] asz ![2]"
tflx_empty "Can't create empty\n\"![1]\""
tflx_expand "Can't expand tempflx"
tflx_frame "Bad FLX frame"
tflx_header "Can't create header for\n\"![1]\""
tflx_open "Error opening flx \"![1]\""
tflx_read1 "Error reading flx"
tflx_read2 "Error reading tempflx."
tflx_reopen "Fatal reopen tempflx failure, file lost."
tflx_ring "Loading of \"![1]\" interrupted.\nLoaded ![2] frames of ![3]",
tflx_screen "Can't reopen tempflx for this screen"
tflx_seek "Error seeking flx"
tflx_write1 "Error writing FLX \"![1]\""
tool_load "Can't load \"![1]\" tool."
tool_fail "\"![1]\" tool failure."
tween_link "Inserting link"
tween_sstate "Saving tween state to ![1]"
tween_state "Trouble loading tween state"
tween_undo "saving tween undo"
unknown_image "Unable to recognize image type of\n\"![1]\""
zoom_open "Can't open zoom window."
zoom_win "Can't open zoom window that size."
//Convert error context messages
conv_err "Error in convert."
conv_pull "Error loading convert pulldowns."
conv_view "Trouble viewing ![1] on frame ![2]."
conv_pencel "Can't open window that size,  reverting to screen sized window."
other_menu "Failure loading image file module menu."
cant_read "Can't read \"![1]\"."
rgb_temp "Trouble with RGB temporary files."
conv_wrong_res "Try changing window size to ![1] x ![2]."
conv_num_frames "![1] can only handle ![2] frame(s)."
conv_over_input "![1] is the current convert input file."
// Player messages
play_cant_display	"Error in script file\n"
					"\"![1]\"\nline ![2]\n"
					"Unable to display \"![3]\"."
play_cant_script  "Unable to play script \"![1]\"."
play_fatal "Fatal error in player."
play_menu "Error loading player menus."
play_nfli_load "Unable to open flic \"![1]\"."
play_nfli_open "Unable to load flic \"![1]\"."
play_nopic "Can't load picture \"![1]\"."
play_script_err "Error in script file \"![1]\"\nline ![2]\n![3]"
}

// Error messages for error codes in errcodes.h
// Match numeric error messages to a string

NumString errors
{
1,"ERROR!"
2,"Out of Memory: asked for ![1]"
3,"Bad input data"
4,"Bad data format"
5,"Out of video display memory"
6,"Out of stack space"
8,"Feature unimplemented"
9,"Data overflow for size of result"
10,"Object not found"
11,"File is not correct type"
13,"Function waiting timed out"
14,"Can't deal with object of this dimension"
15,"Object too big to handle"
16,"Correct file type but wrong version"
17,"Record data is corrupted"
18,"Subsystem not initialized"
19,"Object is not right type"
20,"Too many points in spline"
21,"Hardware lock not present"
22,"File contains RGB/truecolor data, must be processed with CONVERT program"
23,"Unrecognized picture or flic file format"
24,"Value out of range"
25," " // "no message" must be one space
26,"Internal: routine detected bad pointer"
27,"Internal: routine detected bad parameter value"
50,"Error in stdio routine"
75,"Driver isn't REX relocatable executable.  Not a driver"
76,"Driver is REX but not a Autodesk Animator Pro library driver"
77,"Driver is wrong library type"
78,"Missing host provided library"
79,"Wrong host provided library type or version"
80,"User type REX library id_string doesn't match"
100,"File not found. (File name misspelled?)"
101,"Path not found.  (Misspelled directory?)"
102,"Device doesn't exist"
103,"Write error"
104,"Read error"
105,"Seek error"
106,"End of file"
107,"File in use"
108,"File already exists"
109,"Couldn't create file"
110,"File truncated"
111,"File corrupted"
112,"Out of space on device"
113,"The device, window, or file is disabled"
114,"Invalid id value"
115,"File is not open"
116,"Unknown file suffix"
117,"Too many open files.  (Set FILES in config.sys to something higher?)"
118,"Access denied.  (File read only?)"
119,"MS-DOS system tables damaged.  (Reboot time?)"
120,"Bad environment block.  Suggest you reboot"
121,"Bad address to DOS.  (Internal error)"
122,"Unrecognized disk format"
123,"Invalid file access code. (Internal error)"
124,"Invalid data to DOS. (Internal error)"
125,"No more files"
126,"Disk is write protected"
127,"Disk drive not ready. (No disk in drive or door open?)"
128,"Not a DOS disk"
129,"Disk data error, sorry"
130,"Disk sector not found"
131,"Printer out of paper"
132,"General failure"
133,"Critical error"
134,"General network failure"
135,"File sharing error"
136,"File lock error"
137,"Invalid disk  change"
138,"Remote computer not listening"
139, "Network busy"
140,"Sharing temporarily paused"
141,"File/printer redirection paused"
142,"Can't create directory entry"
143,"Directory name too big for MS-DOS"
144,"Invalid directory name."
145,"Invalid file name."
146,"Temp file path doesn't have any valid devices"
147,"Input macro out of synchronization"
148,"Data record not found"
149,"End of data record"
150,"Not enough room for temporary files.\nSee Extra/Configure/Path for Temp Files."
200,"Invalid raster type for operation"
201,"Display hardware not found or invalid"
202,"Blit clipped out entirely"
203,"8514/a adapter not found"
204,"Video BIOS could not set up requested graphics mode"
207,"Asking for a mode that isn't there"
208,"Driver is already open can't open again"
209,"Driver behaving irrationally"
210,"Requested pixel depth not available"
211,"Aspect ratio not displayable"
220,"Too many windows"
300,"Unknown font type"
350,"ADI digitizer status looks bad"
351,"No bitmap in file"
402,"Out of stack space"
403,"Illegal instruction in poco interpreter"
404,"C syntax error"
405,"compiler failed self-check"
407,"See poco.err for more info on error"
408,"Attempting to use a NULL pointer"
410,"No main() routine"
411,"Attempt to divide by zero"
412,"Floating point math error"
413,"Using a FILE that's not fopen()'ed"
414,"Array index or pointer less than array start"
415,"Array index or pointer greater than array end"
416,"bad free() attempt"
417,"free(NULL) error"
418,"program corrupted system resources"
419,"trying to allocate less than 1 byte"
420,"string too big to fit in buffer"
421,"buffer too small for fread"
422,"buffer too small for fwrite"
423,"buffer not big enough"
424,"parameter with invalid value"
426,"too few parameters to function"
427,"function not found"
428,"too many parameters to function"
430,"out of String space"
500,"Cel not present"
501,"Too many frames"
502,"Data chunk not found"
600,"Unmatched quote"
601,"Unmatched single quote"
602,"Unmatched brace"
603,"Line too long"
604,"Syntax error"
605,"No such class"
606,"Not enough fields"
607,"Too many fields"
608,"Expecting string"
609,"Expecting {"
610,"Expecting }"
611,"Expecting numeric ID"
612,"Expecting number"
613,"Invalid character"
614,"Expecting name"
615,"Duplicate symbol in menu file"
}

// *************** converter unique stuff from here down ****************

Text conv_help
{
"Usage: convert [options]\n"
"options: -cfg|-c config_file_path\n"
"         -drv|-d video_driver mode#"
}

Pull conv_pull
{
"Converter",100
	{
	"About",101
	"Status",102,'t'
	"Scale",103
	"Move",104
	"Slide",105,'l'
	"----",106
	"Quit", 107
	}
"Flic",200
	{
	"Load Other",201
	"Load Amiga",202,'a'
	"Load ST",203,'t'
	"Load Flic",204,'f'
	"----",205
	"View",206
	"----",205
	"Save Other",209,
	"Save Old Flic (.FLI)", 207, 'o'
	"Save Flic (.FLC)",208,'c'
	}
"Pic",300
	{
	"Load Targa",301,'g'
	"Load Amiga",302,'a'
	"Load ST",303,'t'
	"Load Macintosh",304,'m'
	"Load PCX",305,'p'
	"Load TIFF",311,'i'
	"Load GIF",306
	"Load BMP",314,'b'
	"----",307
	"View",308
	"----",307
	"Save Targa",313
	"Save PCX",310,'x'
	"Save TIFF",312,'f'
	"Save GIF",309,'e'
	"Save BMP",315,'m'
	}
"Extra",400
	{
	" Screen Size",401,'z'
	" Window Size",402
	"*Tile",403
	}
}

Text conv_about
{
"   Autodesk Animator Pro\n"
"   File Format Converter\n"
"         v. ![1]\n"//    ![2]\n"
"Copyright 1992,1993 by Jim Kent.\n"
"    ![3] ![4], ![5]. ![6]\n"
" Produced exclusively for\n"
"       Autodesk Inc.\n"
"            By\n"
"      Yost Group Inc."
}

Text conv_status
{
"![1] bytes free of ![2] bytes total.\n"
"Largest block ![3].\n"
"Loaded ![4] ![5]x![6] ![7] frames."
}

Qchoice conv_scale
{
"Scale Image (currently ![1]x![2])",
"Set width ![3]",
"Set height ![4]",
"Default dims ![5]x![6]",
"Correct aspect ratio",
"Revert to ![7]x![8]",
"Options",
"Render",
"Exit Scale",
}

Qchoice conv_scale_opts
{
"Scaling Options"
" Dither",
" Recalculate colors",
" RGB Values 0-63",
" RGB Values 0-255",
" Exit",
}

Text conv_scale_width {"Set destination width"}
Text conv_scale_height {"Set destination height"}
Text conv_scale_flilo {"Save as Animator 1.0 compatible flic?"}
Text scale_abort {"Abort scaling?"}
Text rgb_abort {"Abort truecolor load?"}
Text cpack_abort {"Abort color packing?"}
Text conv_abort {"Abort operation?"}
Text conv_quit {"Quit Converter?"}
Text conv_set_load_type {"Load in what file format?"}
Text conv_set_save_type {"Save in what file format?"}

Qchoice conv_slide
{
"Slide..."
"Slide with mouse"
"Set X  ![1]",
"Set Y  ![2]",
"Set frames ![3]",
"Complete",
"Preview",
"Render & Save",
"Exit Slide",
}

Text conv_slidex {"Set X slide?"}
Text conv_slidey {"Set Y slide?"}
Text conv_slidef {"Set frame count?"}

Qchoice conv_st_pics
{
"Which type of Atari ST format picture?",
"Load NEOchrome (.NEO)",
"Load DEGAS (.PI?)",
"Load DEGAS Elite (.PC?)",
"Cancel",
}

Qchoice conv_rgb
{
"Load a Truecolor File"
"In Grays"
"In Color"
"In Color Scaled"
"Dither"
"Cancel"
}

Qchoice conv_amiga_flic
{
"Which type of Amiga Flic?",
"Load RIF (Zoetrope & Live!)",
"Load ANIM (Dpaint III etc.)",
"Cancel",
}

Qchoice conv_window
{
"Set size of convert window",
"Full Screen",
"To Image Size",
"To 320x200",
"To 640x400",
"To 640x480",
"To 800x600",
"To 1024x768",
"Set Custom (now ![1]x![2])",
"Cancel",
}

Text conv_window_width {"Set window width"}
Text conv_window_height {"Set window height"}

// filename requestor strings

Text conv_loadf {"Load File?"}
Text conv_svpic {"Save Pic?"}
Text conv_svflic {"Save Flic?"}
Text conv_sclname {"Pick name for scaled flic."}
Text conv_sclword {"Scale"}

Text conv_waitld {"loading ![1]"}
Text conv_waitsv {"saving ![1]"}

// status line texts

Text ctop_scaleline {"Reading and x scaling line ![1] of ![2]."}
Text ctop_xscale {"Scaling along X"}
Text ctop_hist {"Making color histogram."}
Text ctop_rfit {"Reading and fitting line ![1] of ![2]."}
Text ctop_moved {"Moved x ![1]  y ![2]"}
Text ctop_yscale {"yscaling ![1]"}
Text ctop_cfitline {"color fitting line ![1] of ![2]"}
Text ctop_xscline {"X-scaling line ![1] of ![2]"}
Text ctop_yscline {"Y-scaling ![1] line ![2] of ![3]"}
Text ctop_cpack {"Packing ![1] colors to ![2] threshold ![3]"}
Text ctop_scalef {"Scaling frame ![1] of ![2]"}

// Player Stuff ***********************************

Text play_help
{
"Usage: player [scriptfile] [options]\n"
"options: -cfg|-c config_file_path\n"
"         -drv|-d video_driver mode#\n"
"         -auto|-a\n"
"The auto option will bypass the initial about box."
}

NameString play_panel
{
title "Frames"
speed "Play speed"
}

Pull play_pull
{
"Player",100
	{
	"About",101,'a'
	"Memory",102,'m'
	"----",106
	"Quit",107,'q'
	}
"File",200
	{
	"Load Flic",201,'f'
	"Open Flic",202,'o'
	"Load GIF",203,'g'
	"Load PCX",204,'x'
	"Load Other",205,'l'
	"Play Script",206,'p'
	"----",207
	"Screen Size",208,'s'
	}
}

Text play_about
{
"Autodesk Animator Pro Player(TM) ![1]\n"
"This program may be freely distributed.\n"
// ![2] is serializer string
"Copyright 1992,1993 Jim Kent.\n"
"    ![3] ![4], ![5]. ![6]\n"
" Produced exclusively for\n"
"       Autodesk, Inc.\n"
"            By\n"
"      Yost Group Inc."
}

Text aniplay_free
{
"Autodesk Animator Pro Player(TM) 1.3\n"
"\n"
"This program may be freely\n"
"distributed.\n"
"\n"
"Copyright 1992,1993 Jim Kent.\n"
"Produced exclusively for\n"
"       Autodesk, Inc.\n"
"            By\n"
"       Yost Group Inc."
}

Text play_mem
{
"Player memory Info:\n\n"
"![1] bytes free of ![2].\n"
"Memory used ![3].  Max used ![4].\n"
}

Text play_flimem { "There is not enough ram to load \"![1]\"\n"
				   "Play off disk?" }
Text play_load { "Warning! ![1] loaded but never played. Ignored." }
Text play_free { "Warning! ![1] freed but never loaded. Ignored." }
Text play_quit {"Quit Player?"}
Text play_lock { "Lock keyboard until next\n"
				"control '![1]' is entered?" }
Text play_unlock { "Canceling keyboard lock." }
Text play_loadscr {"Load and play a script file?"}
Text play_ask_fd {"Insert disk in drive \"![1]\"\nto load \"![2]\""}

NameString play_flicload
{
to_load "Pick flic file to load to ram"
load "Load"
to_open "Pick flic file to open"
open "Open"
}

Text play_gifl {"Pick GIF file to load"}
Text play_pcxl {"Pick PCX file to load"}
Text play_fmtl {"Pick image format to load"}
Text play_ffl {"Pick ![1] file to load first frame of"}

// script errors for player
NameString play_serrors
{
gosub_nest "Too many nested gosubs, loops and keychoices."
cant_open_scr "Can't open script \"![1]\""
cant_open "Can't open \"![1]\""
bad_choice	"Invalid choice \"![1]\""
dubl_choice "Redundant choice '![1]'"
loop_count "Invalid loop count."
bad_option "Invalid play option \"![1]\""
bad_arg "Bad argument \"![2]\" for \"![1]\""
unexpected "Unexpected \"![1]\""
eof_err "Unexpected end of file"
sub_not_found "Subroutine \"![1]\" not found."
sub_declared "Subroutine \"![1]\" already declared"
load_unplayed "Can't load flic that is not played!"
not_loaded "flic not loaded"
unterminated "Unterminated \"![1]\""
cant_load_flic "Can't load flic \"![1]\""
}
