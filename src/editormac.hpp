/* 
vst~ - VST plugin object for PD 
based on the work of Jarno Seppänen and Mark Williamson

Copyright (c)2003-2004 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  
*/

#include "Editor.h"
#include "VstHost.h"
#include <flext.h>


#if FLEXT_OS == FLEXT_OS_MAC
// only Mac OSX code is situated in this file

void SetupEditor()
{
}

void StartEditor(VSTPlugin *p)
{
}

void StopEditor(VSTPlugin *p) 
{
}

void ShowEditor(VSTPlugin *p,bool show) 
{
}

void MoveEditor(VSTPlugin *p,int x,int y) 
{
}

void SizeEditor(VSTPlugin *p,int x,int y) 
{
}

void CaptionEditor(VSTPlugin *plug,bool c)
{
}

void TitleEditor(VSTPlugin *p,const char *t) 
{
}

void HandleEditor(VSTPlugin *p,bool h)
{
}

void FrontEditor(VSTPlugin *p)
{
}

#endif // FLEXT_OS_MAC
