pool - a hierarchical storage object for PD and Max/MSP

Copyright (c) 2002-2003 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

Donations for further development of the package are highly appreciated.

----------------------------------------------------------------------------

You will need the flext C++ layer for PD and Max/MSP externals to compile this.

Package files:
- readme.txt: this one
- gpl.txt,license.txt: GPL license stuff
- main.cpp, pool.h, pool.cpp, data.cpp

----------------------------------------------------------------------------

Goals/features of the package:

- pool can store and retrieve key/value pairs, where a key can be any atom and 
	the value can be any list of atoms
- pool can manage folders. A folder name can be any atom.
- pool objects can be named and then share their data space
- clipboard operations are possible in a pool or among several pools
- file operations can load/save data from disk

----------------------------------------------------------------------------

The package should at least compile (and is tested) with the following compilers:

pd - Windows:
-------------
o Borland C++ 5.5 (free): edit "config-pd-bcc.txt" & run "build-pd-bcc.bat" 

o Microsoft Visual C++ 6: edit the project file "pool.dsp" & build 

pd - linux:
-----------
o GCC: edit "config-pd-linux.txt" & run "sh build-pd-linux.sh" 

pd - darwin (MacOSX):
---------------------
o GCC: edit "config-pd-darwin.txt" & run "sh build-pd-darwin.sh" 

Max/MSP - MacOS9:
-----------------
o CodeWarrior: edit "pool.cw" and build 

----------------------------------------------------------------------------

Version history:

0.2.0:
- attributes (pool,private,echodir,absdir)
- added "geti" message for retrieval of a value at an index
- fixed bug in "get" message if key not present
- adapted source to flext 0.4.1 - register methods at class creation
- extensive use of hashing for keys and directories
- database can be saved/loaded as XML data

0.1.0:
- first public release

---------------------------------------------------------------------------

BUGS:
- pool does not handle symbols with spaces or all digits


TODO list:

general:
- speed up the database
- what is output as value if it is key only? (Max->nothing!)


