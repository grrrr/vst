/* 
vst~ - VST plugin object for PD 
based on the work of Jarno Seppänen and Mark Williamson

Copyright (c)2003-2004 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  
*/

#ifndef __EDITOR_H
#define __EDITOR_H

class VSTPlugin;

void SetupEditor();
void StartEditor(VSTPlugin *p);
void StopEditor(VSTPlugin *p);
void ShowEditor(VSTPlugin *p,bool show);

#endif // __EDITOR_H
