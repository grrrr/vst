vst~ - VST plugin external for PD
based on the work of Jarno Seppänen and Mark Williamson

Copyright (c)2003-04 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

Donations for further development of the package are highly appreciated.
Visit https://www.paypal.com/xclick/business=t.grill%40gmx.net&item_name=vst&no_note=1&tax=0&currency_code=EUR

----------------------------------------------------------------------------

You will need the flext C++ layer for PD and Max/MSP externals to compile this.
see http://www.parasitaere-kapazitaeten.net/ext

----------------------------------------------------------------------------


BUILDING:
=========


pd - Windows:
-------------
o Microsoft Visual C++ 6 or 7 (.NET): edit "config-pd-msvc.txt" & run "build-pd-msvc.bat"


NOT YET:

pd - darwin (MacOSX):
---------------------
o GCC: edit "config-pd-darwin.txt" & run "sh build-pd-darwin.sh" 


----------------------------------------------------------------------------

Version history:

0.1.0:
- fixed crash when there's no "VST_PATH" environment string found
- included pd path into VST search path
- made thread of editor window low priority
- introduced A LOT of attributes to get info about the plugin
- make editor window closable by patch
- plugin can be changed with plug attribute
- fixed crash on destroying vst~ with open editor window
- stripped all MFC code
- fixed DSP initialization, zero dangling audio vectors
- pre12: added "bypass" and "mute" attributes
- pre13: with flext 0.4.7 no more interruptions on window close
- pre14: allow window titles with spaces and update it on window startup

0.0.0:
- version of mark@junklight.com

---------------------------------------------------------------------------

BUGS:
- mouse interaction in editor can cause audio dropouts
- crash when unloading plugin with open window

TODO:
- examine why Waveshell behaves so strange (vendor string length exceed VST specification)
- add VSTTimeInfo functionality
- add Midi out functionality
- include necessary Steinberg license stuff
- do name scanning in the background
- translate special characters in strings (like ° as param_label) into system-digestible form
