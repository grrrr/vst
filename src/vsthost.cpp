/* 
vst~ - VST plugin object for PD 
based on the work of Jarno Seppänen and Mark Williamson

Copyright (c)2003-2005 Thomas Grill (gr@grrrr.org)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  
*/

#include "vsthost.h"


const t_symbol 
    *VSTPlugin::sym_param,
    *VSTPlugin::sym_event,
    *VSTPlugin::sym_evmidi,
    *VSTPlugin::sym_evaudio,
    *VSTPlugin::sym_evvideo,
    *VSTPlugin::sym_evparam,
    *VSTPlugin::sym_evtrigger,
    *VSTPlugin::sym_evsysex,
    *VSTPlugin::sym_ev_,
    *VSTPlugin::sym_midi[8];

void VSTPlugin::Setup()
{
    sym_param = flext::MakeSymbol("param");
    sym_event = flext::MakeSymbol("event");
    sym_evmidi = flext::MakeSymbol("midi");
    sym_evaudio = flext::MakeSymbol("audio");
    sym_evvideo = flext::MakeSymbol("video");
    sym_evparam = flext::MakeSymbol("param");
    sym_evtrigger = flext::MakeSymbol("trigger");
    sym_evsysex = flext::MakeSymbol("sysex");
    sym_ev_ = flext::MakeSymbol("???");

    sym_midi[0] = flext::MakeSymbol("noteon");
    sym_midi[1] = flext::MakeSymbol("noteoff");
    sym_midi[2] = flext::MakeSymbol("polyafter");
    sym_midi[3] = flext::MakeSymbol("cntl");
    sym_midi[4] = flext::MakeSymbol("progchg");
    sym_midi[5] = flext::MakeSymbol("chnafter");
    sym_midi[6] = flext::MakeSymbol("pitchbend");
    sym_midi[7] = sym__;
}

VSTPlugin::VSTPlugin(Responder *resp)
    : hdll(NULL),hwnd(NULL)
    , effect(NULL),pluginmain(NULL),audiomaster(NULL)
    , responder(resp)
    , posx(0),posy(0),caption(true)
    , midichannel(0),eventqusz(0)
    , paramnamecnt(0)
    , transchg(true)
    , playing(false),looping(false),feedback(false)
    , samplerate(0)
    , samplepos(0),ppqpos(0)
    , tempo(120)
    , timesignom(4),timesigden(4)
    , barstartpos(0)
    , cyclestartpos(0),cycleendpos(0)
    , smpteoffset(0),smpterate(0)
{}

VSTPlugin::~VSTPlugin()
{
	Free();
}


#if FLEXT_OS == FLEXT_OS_MAC
OSStatus FSPathMakeFSSpec(const UInt8 *path,FSSpec *spec,Boolean *isDirectory)  /* can be NULL */
{
    OSStatus  result;
    FSRef    ref;

    /* check parameters */
    require_action(NULL != spec, BadParameter, result = paramErr);

    /* convert the POSIX path to an FSRef */
    result = FSPathMakeRef(path, &ref, isDirectory);
    require_noerr(result, FSPathMakeRef);

    /* and then convert the FSRef to an FSSpec */
    result = FSGetCatalogInfo(&ref, kFSCatInfoNone, NULL, NULL, spec, NULL);
    require_noerr(result, FSGetCatalogInfo);
  
FSGetCatalogInfo:
FSPathMakeRef:
BadParameter:
    return result;
}
#endif

// hdll, pluginmain and audiomaster are set here
// must be NULL beforehand!
bool VSTPlugin::NewPlugin(const char *plugname)
{
    FLEXT_ASSERT(!hdll && !pluginmain && !audiomaster);

    dllname = plugname;

#if FLEXT_OS == FLEXT_OS_WIN
    hdll = LoadLibrary(dllname.c_str());
    if(hdll) pluginmain = (PVSTMAIN)GetProcAddress(hdll,"main");
    audiomaster = Master;  

#elif FLEXT_OS == FLEXT_OS_MAC
    short   resFileID;
    FSSpec  spec;
    OSErr err;

    err = FSPathMakeFSSpec(dllname.c_str(),&spec,NULL);
    resFileID = FSpOpenResFile(&spec, fsRdPerm);
    short cResCB = Count1Resources('aEff');

    for(int i = 0; i < cResCB; i++) {
        Handle             codeH;
        CFragConnectionID  connID;
        Ptr                mainAddr;
        Str255             errName;
        Str255             fragName;
        char               fragNameCStr[256];
        short              resID;
        OSType             resType;

        codeH = Get1IndResource('aEff', short(i+1));
        if(!codeH) continue;

        GetResInfo(codeH, &resID, &resType, fragName);
        DetachResource(codeH);
        HLock(codeH);

        err = GetMemFragment(*codeH,
                             GetHandleSize(codeH),
                             fragName,
                             kPrivateCFragCopy,
                             &connID, (Ptr *) & mainAddr, errName);

        if(!err) {
           #ifdef __CFM__
           pluginmain = (PVSTMAIN)NewMachOFromCFM(mainAddr);
           #else
           pluginmain = (PVSTMAIN)mainAddr;
           #endif
        }
    }
    CloseResFile(resFileID);

    audiomaster = 
#ifdef __CFM__
        NewCFMFromMachO(Master);
#else
        Master;
#endif

#else
#error Platform not supported
#endif    

    if(hdll && pluginmain && audiomaster)
        return true;
    else {
        FreePlugin();
        return false;
    }
}

void VSTPlugin::FreePlugin()
{
#if FLEXT_OS == FLEXT_OS_WIN
    if(hdll) FreeLibrary(hdll); 
#elif FLEXT_OS == FLEXT_OS_MAC
	
#ifdef __MACOSX__
#ifdef __CFM__
    if(audiomaster) DisposeCFMFromMachO(audiomaster);
    if(pluginmain) DisposeMachOFromCFM(pluginmain);
#endif
#endif

#else
#error Platform not supported
#endif    

    effect = NULL;
    audiomaster = NULL;
    pluginmain = NULL;
    hdll = NULL;
} 

/*
This is static to be able to communicate between the plugin methods 
and the static Audiomaster function
the this (plugin->user) pointer has not been initialized at the point it is needed
static should not be a problem, as we are single-threaded and it is immediately 
queried in a called function
*/
long VSTPlugin::uniqueid = 0;

std::string VSTPlugin::dllloading;

bool VSTPlugin::InstPlugin(long plugid)
{
    uniqueid = plugid;
    dllloading = dllname;

    FLEXT_ASSERT(pluginmain && audiomaster);

	//This calls the "main" function and receives the pointer to the AEffect structure.
	effect = pluginmain(audiomaster);
	if(!effect || effect->magic != kEffectMagic) {
		post("VST plugin : Unable to create effect");
		effect = NULL; 
	    return false;
    }
    return true;
}

bool VSTPlugin::Instance(const char *name,const char *subname)
{
    bool ok = effect != NULL;
    
    if(!ok && dllname != name) {
        FreePlugin();
        // freshly load plugin
        ok = NewPlugin(name) && InstPlugin();
    }

    if(ok && subname && *subname && Dispatch(effGetPlugCategory) == kPlugCategShell) {
        // sub plugin-name given -> scan plugs

        long plugid;
	    char tmp[64];
	    // scan shell for subplugins
	    while((plugid = Dispatch(effShellGetNextPlugin,0,0,tmp))) { 
		    // subplug needs a name
            FLEXT_LOG1("subplug %s",tmp);
            if(!strcmp(subname,tmp)) 
                // found
                break;
	    }

        // re-init with plugid set
        if(plugid) ok = InstPlugin(plugid);
    }

    if(ok) {
	    //init plugin 
	    effect->user = this;
	    ok = Dispatch(effOpen) == 0;
    }

    if(ok) {
	    ok = Dispatch(effIdentify) == 'NvEf';
    }

    if(ok) {
        *productname = 0;
	    long ret = Dispatch(effGetProductString,0,0,productname);

        if(!*productname) {
		    // no product name given by plugin -> extract it from the filename

		    std::string str1(dllname);
		    std::string::size_type slpos = str1.rfind('\\');
		    if(slpos == std::string::npos) {
			    slpos = str1.rfind('/');
			    if(slpos == std::string::npos)
				    slpos = 0;
			    else
				    ++slpos;
		    }
		    else
			    ++slpos;
		    std::string str2 = str1.substr(slpos);
		    int snip = str2.find('.');
            if( snip != std::string::npos )
			    str1 = str2.substr(0,snip);
		    else
			    str1 = str2;
		    strcpy(productname,str1.c_str());
	    }
    	
        if(*productname) {
            char tmp[512];
            sprintf(tmp,"vst~ - %s",productname);
            title = tmp;
        }
        else
            title = "vst~";

	    *vendorname = 0;
	    Dispatch(effGetVendorString,0,0,vendorname);
    }

    if(!ok) Free();
	return ok;
}

void VSTPlugin::Free() // Called also in destruction
{
	if(effect) {
        Edit(false);

        // shut down plugin
		Dispatch(effMainsChanged, 0, 0);
		Dispatch(effClose);
    }

    // \TODO
    // Here, we really have to wait until the editor thread has terminated
    // otherwise WM_DESTROY etc. messages may still be pending
    // in other words: this is a design flaw
    // There should be a data stub accessible from the plugin object and the thread
    // holding the necessary data, so that both can operate independently

    FreePlugin(); 
}

void VSTPlugin::DspInit(float sr,int blsz)
{
    // sample rate and block size must _first_ be set
	Dispatch(effSetSampleRate,0,0,NULL,samplerate = sr);
	Dispatch(effSetBlockSize, 0,blsz);
    // then signal that mains have changed!
    Dispatch(effMainsChanged,0,1);
}

void VSTPlugin::ListPlugs(const t_symbol *sym) const
{
    if(responder) {
        if(Is() && Dispatch(effGetPlugCategory) == kPlugCategShell) {
            t_atom at;
            // sub plugin-name given -> scan plugs
	        char tmp[64];
	        // scan shell for subplugins
            while(Dispatch(effShellGetNextPlugin,0,0,tmp)) {
                SetString(at,tmp);
                responder->Respond(sym,1,&at);
            }
        }

        // bang
        responder->Respond(sym);
    }
}
