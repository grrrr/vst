/* 
vst~ - VST plugin object for PD 
based on the work of Jarno Seppänen and Mark Williamson

Copyright (c)2003-2005 Thomas Grill (gr@grrrr.org)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  
*/

#include "vsthost.h"
#include "editor.h"


void VSTPlugin::Edit(bool open)
{	
	if(Is()) { 	
        if(open) {
		    if(HasEditor() && !IsEdited())
                StartEditor(this);
        }
        else if(IsEdited())
            StopEditor(this);
	}
}

void VSTPlugin::StartEditing(WHandle h)
{
    FLEXT_ASSERT(h != NULL);
	Dispatch(effEditOpen,0,0,hwnd = h);
//	Dispatch(effEditTop);

    TitleEditor(this,title.c_str());
}

void VSTPlugin::StopEditing() 
{ 
    if(Is()) {
	    Dispatch(effEditClose);					
        hwnd = NULL; 
    }
}

void VSTPlugin::Visible(bool vis)
{	
	if(Is() && IsEdited()) ShowEditor(this,vis);
}

bool VSTPlugin::IsVisible() const
{	
	return Is() && IsEdited() && IsEditorShown(this);
}


void VSTPlugin::SetPos(int x,int y,bool upd) 
{
    if(Is()) {
        posx = x; posy = y; 
        if(upd && IsEdited()) MoveEditor(this,posx,posy);
    }
}

void VSTPlugin::SetCaption(bool c) 
{
    if(Is()) {
        caption = c; 
        if(IsEdited()) CaptionEditor(this,c);
    }
}

void VSTPlugin::SetTitle(const char *t)
{
    if(Is()) {
        title = t; 
        if(IsEdited()) TitleEditor(this,t);
    }
}
