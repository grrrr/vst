vst~ - VST plugin external for PD

Copyright (c) 2002-2003 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

Donations for further development of the package are highly appreciated.

----------------------------------------------------------------------------

You will need the flext C++ layer for PD and Max/MSP externals to compile this.

----------------------------------------------------------------------------

The package should at least compile (and is tested) with the following compilers:

pd - Windows:
-------------
o Microsoft Visual C++ 6: edit the project file "vst.dsp" & build 

pd - darwin (MacOSX):
---------------------
o GCC: edit "config-pd-darwin.txt" & run "sh build-pd-darwin.sh" 

Max/MSP - MacOS9/X:
-------------------
o CodeWarrior: edit "vst.cw" and build 

----------------------------------------------------------------------------

Version history:

0.1.0:
- first public release

---------------------------------------------------------------------------

BUGS:
- no MFC application instantiated... crash in debug mode (upon assertion)

features:
- make plugs loadable at run-time


