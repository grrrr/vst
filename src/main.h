/* 
vst~ - VST plugin object for PD 
based on the work of Jarno Seppänen and Mark Williamson

Copyright (c)2003-2005 Thomas Grill (gr@grrrr.org)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  
*/

#ifndef __VST_H
#define __VST_H

#define FLEXT_ATTRIBUTES 1

#include <flext.h>

#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 405)
#error You need at least flext version 0.4.5
#endif

#if FLEXT_OS == FLEXT_OS_WIN || FLEXT_OS == FLEXT_OS_MAC
//
#else
#error Platform not supported!
#endif

typedef void V;
typedef int I;
typedef long L;
typedef unsigned long UL;
typedef float F;
typedef t_sample R;
typedef char C;
typedef bool BL;
typedef t_atom A;
typedef t_symbol S;


#endif

