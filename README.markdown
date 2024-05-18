kiki's notes on this branch
===========================

The build is set up with CMake, but a makefile wraps the build.  Typing `make` should build the ani executable.

This currently only builds for MacOS.  You'll need the following installed:
- CMake 3.26+
- autoconf
- automake

I recommend installing all through brew:

```
brew install cmake autoconf automake
```

Without the autotools stuff, libffi will fail to build.


Animator Pro A.K.A.
===================

Animator Pro, (A.K.A. Autodesk Animator Pro, Ani Pro, PJ Paint, or simply PJ) 
is a 256 color paint and animation package for MSDOS. It was popular 
in the early to mid 1990's for game art, online animation, 
business presentations and occasional TV productions.

Originally released in 1989 by Autodesk, and licensed from Yost Group, Ani Pro 
was created by Peter Kennard, Gary Yost and Jim Kent. Jim Kent kept copyrights 
to the source code, thus allowing him to grant me permission to make it 
available to you here. For my part, I wish to do my best to preserve and 
document this tiny bit of significant computer history, promote the usefulness 
of this program and hopefully improve and modify it for future and current 
users, many of whom still find it very useful and fun.

More info at [the Animator Pro project homepage][1]


[1]: http://animatorpro.org "Animator Pro project homepage"

binaries
---
To get binaries, click the huge "DOWNLOADS" button on the github project.
or go to [the downloads page][2]

[2]: https://github.com/AnimatorPro/Animator-Pro/downloads "Downloads Page"

Pristine
--------
The original Autodesk Animator (A.K.A. Video Paint, VPaint, or simply V)
exactly as it was sent to me by Jim Kent, preserved for historical purposes.

Pristine-Pro
------------
The original Animator Pro (A.K.A. Autodesk Animator Pro, PJPaint or simply PJ)
exactly as it was sent to me by Jim Kent, preserved for historical purposes.

dev
---
A work in progress version of the Animator Pro source as I work to improve it.

text
----
The assorted text files and documentation found within the original source files.

pocoscripts
-----------
Source code and documentation for scripts written in Animator Pro's built in 
POCO scripting language; essentially an interpreted version of the 
C programming language with some built in libraries for manipulating
the program.

files
-----
Example files that can be natively read into the Animator Pro software.

License
-------
This project is not endorsed by Autodesk, Inc., 
Autodesk is a trademark of Autodesk, Inc.,

However, interestingly, Autodesk, Inc's tradmarks on "Autodesk Animator" 
and "Animator Pro" are expired. 

All source code  (unless otherwise marked, or if better information 
becomes available) is ©1989-1994 Jim Kent and is available here under 
the BSD license, reproduced below:

Copyright (c) 1989-1994, Jim Kent All rights reserved.
 
Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 * Neither the name Animator Pro nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
