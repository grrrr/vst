vst~ - VST plugin external for PD
based on the work of Jarno Seppänen and Mark Williamson

Copyright (c)2003 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

Donations for further development of the package are highly appreciated.

----------------------------------------------------------------------------

You will need the flext C++ layer for PD and Max/MSP externals to compile this.
see http://www.parasitaere-kapazitaeten.net/ext

----------------------------------------------------------------------------


BUILDING:
=========


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
- fixed crash when there's no "VST_PATH" environment string found
- included pd path into VST search path
- made thread of editor window low priority
- introduced A LOT of attributes to get info about the plugin
- make editor window closable by patch
- plugin can be changed with plug attribute

0.0.0:
- version of mark@junklight.com

---------------------------------------------------------------------------

features:
- allow various window styles and window moving & titling
- include necessary Steinberg license stuff

BUGS:
- strange: GRM reson stutters when GUI is edited with the mouse (mousedown)
	-> plug sends (a lot of) idle messages
	-> no stuttering when a message is posted to the console... ähem
