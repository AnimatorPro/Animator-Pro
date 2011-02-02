rem I don't know what the autodiff on the compiler.h is all about, below,
rem but it's likely to never find a difference in the future...

rem integration based on doc\integrat.doc - 90/09/25 17:03:14
copy h:\inc\AACONFIG.H inc\AACONFIG.H
autodiff inc\COMPILER.H h:\inc\COMPILER.H H:COMPILER.DIF -LONG
copy h:\inc\FLI.H inc\FLI.H
copy h:\inc\INC.BAT inc\INC.BAT
copy h:\inc\JIMK.H inc\JIMK.H
copy h:\inc\MAKEHC.INC inc\MAKEHC.INC
