/* 
vst~ - VST plugin object for PD 
based on the work of Jarno Sepp�nen and Mark Williamson

Copyright (c)2003-2004 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  
*/

#include "Editor.h"
#include "VstHost.h"
#include "AEffectx.h"

#include <ctype.h>

static VstTimeInfo _timeInfo;

typedef AEffect *(VSTCALLBACK *PVSTMAIN)(audioMasterCallback audioMaster);


VSTPlugin::VSTPlugin():
    h_dll(NULL),hwnd(NULL),_pEffect(NULL),
    posx(0),posy(0),caption(true),
	_midichannel(0),queue_size(0),
    paramnamecnt(0)
{}

VSTPlugin::~VSTPlugin()
{
	Free();				// Call free
}
 
int VSTPlugin::Instance(const char *dllname)
{
	h_dll = LoadLibrary(dllname);
	if(!h_dll)
		return VSTINSTANCE_ERR_NO_VALID_FILE;

	PVSTMAIN main = (PVSTMAIN)GetProcAddress(h_dll,"main");
	if(!main) {	
		FreeLibrary(h_dll);
		_pEffect = NULL;
		return VSTINSTANCE_ERR_NO_VST_PLUGIN;
	}

	//This calls the "main" function and receives the pointer to the AEffect structure.
	_pEffect = main((audioMasterCallback)Master);
	
	if(!_pEffect || _pEffect->magic != kEffectMagic) {
		post("VST plugin : Unable to create effect");

	    _pEffect = NULL;
		FreeLibrary(h_dll); h_dll = NULL;
	    return VSTINSTANCE_ERR_REJECTED;
    }
	
	//init plugin 
	_pEffect->user = this;

	long ret = Dispatch( effOpen );
    FLEXT_ASSERT(!ret);

	ret = Dispatch( effIdentify);
	FLEXT_ASSERT(ret == 'NvEf');

    *_sProductName = 0;
	Dispatch( effGetProductString, 0, 0, &_sProductName, 0.0f);
    if(_sProductName) {
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
		strcpy(_sProductName,str1.c_str());
	}
	
    if(*_sProductName) {
        char tmp[256];
        sprintf(tmp,"vst~ - %s",_sProductName);
        title = tmp;
    }
    else
        title = "vst~";

	*_sVendorName = 0;
	Dispatch( effGetVendorString, 0, 0, &_sVendorName, 0.0f);

	_sDllName = dllname;

	return VSTINSTANCE_NO_ERROR;
}


/*
void VSTPlugin::Create(VSTPlugin *plug)
{
	h_dll = plug->h_dll;
	_pEffect = plug->_pEffect;
	_pEffect->user = this;

	Dispatch( effMainsChanged,  0, 1);
//	strcpy(_editName,plug->_editName); On current implementation, this replaces the right one. 
	strcpy(_sProductName,plug->_sProductName);
	strcpy(_sVendorName,plug->_sVendorName);
	
	_sDllName = new char[strlen(plug->_sDllName)+1];
	strcpy(_sDllName,plug->_sDllName);

	_isSynth=plug->_isSynth;
	_version=plug->_version;

	plug->instantiated=false;	// We are "stoling" the plugin from the "plug" object so this
								// is just a "trick" so that when destructing the "plug", it
								// doesn't unload the Dll.
	instantiated=true;
}
*/

void VSTPlugin::Free() // Called also in destruction
{
	if(Is()) {
        if(IsEdited()) StopEditor(this);

        // shut down plugin
		Dispatch(effMainsChanged, 0, 0);
		Dispatch(effClose);

		_pEffect = NULL;
        if(h_dll) { FreeLibrary(h_dll); h_dll = NULL; }
	}
}

void VSTPlugin::DspInit(float samplerate,int blocksize)
{
    // sample rate and block size must _first_ be set
	Dispatch(effSetSampleRate, 0, 0,NULL,samplerate);
	Dispatch(effSetBlockSize,  0, blocksize);
    // than signal that mains have changed!
    Dispatch(effMainsChanged,  0, 1);
}

static void striptrail(char *txt)
{
    // strip trailing whitespace
    for(int i = strlen(txt)-1; i >= 0; --i) 
        // cast to unsigned char since isspace functions don't want characters like 0x80 = -128
        if(isspace(((unsigned char *)txt)[i])) txt[i] = 0;
}

void VSTPlugin::GetParamName(int numparam,char *name) const
{
    if(numparam < GetNumParams()) {
        name[0] = 0;
        Dispatch(effGetParamName,numparam,0,name,0.0f);
        striptrail(name);
    }
	else 
        name[0] = 0;
}

bool VSTPlugin::SetParamFloat(int parameter,float value)
{
	if(Is() && parameter >= 0 && parameter < GetNumParams()) {
		_pEffect->setParameter(_pEffect,parameter,value);
		return true;
	}
    else
	    return false;
}

void VSTPlugin::GetParamValue(int numparam,char *parval) const
{
    if(Is()) {
        if(numparam < GetNumParams()) {
            // how many chars needed?
            char par_display[64]; par_display[0] = 0;
			Dispatch(effGetParamDisplay,numparam,0,par_display,0.0f);
//            if(par_display[7]) par_display[8] = 0; // set trailing zero

            // how many chars needed?
			char par_label[64]; par_label[0] = 0;
            Dispatch(effGetParamLabel,numparam,0,par_label,0.0f);
            striptrail(par_label);
//            if(par_label[7]) par_label[8] = 0; // set trailing zero

			sprintf(parval,"%s%s",par_display,par_label);
        }
	    else 
            strcpy(parval,"Index out of range");
    }
	else		
        strcpy(parval,"Plugin not loaded");
}

float VSTPlugin::GetParamValue(int numparam) const
{
	if(Is() && numparam < GetNumParams()) 
        return _pEffect->getParameter(_pEffect, numparam);
	else 
        return -1.0;
}

void VSTPlugin::ScanParams(int cnt)
{
    if(cnt < 0) cnt = GetNumParams();
    if(paramnamecnt >= cnt) return;
    if(cnt >= GetNumParams()) cnt = GetNumParams();

    char name[64];
    for(int i = paramnamecnt; i < cnt; ++i) {
        GetParamName(i,name);
        if(*name) paramnames[std::string(name)] = i;
    }
    paramnamecnt = cnt;
}

int VSTPlugin::GetParamIx(const char *p) const
{
    NameMap::const_iterator it = paramnames.find(std::string(p));
    return it == paramnames.end()?-1:it->second;
}

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
}

void VSTPlugin::StopEditing() 
{ 
	Dispatch(effEditClose);					
    hwnd = NULL; 
}

void VSTPlugin::Visible(bool vis)
{	
	if(Is() && IsEdited()) ShowEditor(this,vis);
}

bool VSTPlugin::IsVisible() const
{	
	return Is() && IsEdited() && IsEditorShown(this);
}

bool VSTPlugin::AddMIDI(unsigned char data0,unsigned char data1,unsigned char data2)
{
	if(Is()) {
		VstMidiEvent* pevent = &midievent[queue_size];

		pevent->type = kVstMidiType;
		pevent->byteSize = 24;
		pevent->deltaFrames = 0;
		pevent->flags = 0;
		pevent->detune = 0;
		pevent->noteLength = 0;
		pevent->noteOffset = 0;
		pevent->reserved1 = 0;
		pevent->reserved2 = 0;
		pevent->noteOffVelocity = 0;
		pevent->midiData[0] = data0;
		pevent->midiData[1] = data1;
		pevent->midiData[2] = data2;
		pevent->midiData[3] = 0;

		if ( queue_size < MAX_EVENTS ) queue_size++;
		SendMidi();
		return true;
	}
	else return false;
}


void VSTPlugin::SendMidi()
{
	if(Is() && queue_size > 0) {
		// Prepare MIDI events and free queue dispatching all events
		events.numEvents = queue_size;
		events.reserved  = 0;
		for(int q = 0; q < queue_size; q++) 
            events.events[q] = (VstEvent*)&midievent[q];
		
		Dispatch(effProcessEvents, 0, 0, &events, 0.0f);
		queue_size = 0;
	}
}

static int range(int value,int mn = 0,int mx = 127) 
{
    return value < mn?mn:(value > mx?mx:value);
}

bool VSTPlugin::AddNoteOn( unsigned char note,unsigned char speed,unsigned char midichannel)
{
    return AddMIDI((char)MIDI_NOTEON | midichannel,note,speed);
}

bool VSTPlugin::AddNoteOff( unsigned char note,unsigned char midichannel)
{
    return AddMIDI((char)MIDI_NOTEOFF | midichannel,note,0);
}

void VSTPlugin::AddAftertouch(int value)
{
 	AddMIDI( (char)MIDI_NOTEOFF | _midichannel , range(value) );
}

void VSTPlugin::AddPitchBend(int value)
{
	AddMIDI( MIDI_PITCHBEND + (_midichannel & 0xf) , ((value>>7) & 127), (value & 127));
}

void VSTPlugin::AddProgramChange(int value)
{
    AddMIDI( MIDI_PROGRAMCHANGE + (_midichannel & 0xf), range(value), 0);
}

void VSTPlugin::AddControlChange(int control, int value)
{
    AddMIDI( MIDI_CONTROLCHANGE + (_midichannel & 0xf), range(control), range(value));
}


bool VSTPlugin::GetProgramName( int cat , int p, char *buf) const
{
    buf[0] = 0;
	int parameter = p;
	if(parameter < GetNumPrograms() && cat < GetNumCategories()) {
		Dispatch(effGetProgramNameIndexed,parameter,cat,buf,0.0f);
        striptrail(buf);
		return true;
	}
	else
        return false;
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

void VSTPlugin::processReplacing( float **inputs, float **outputs, long sampleframes )
{
	_pEffect->processReplacing( _pEffect , inputs , outputs , sampleframes );
}

void VSTPlugin::process( float **inputs, float **outputs, long sampleframes )
{
	_pEffect->process( _pEffect , inputs , outputs , sampleframes );
}

#if 1

// Host callback dispatcher
long VSTPlugin::Master(AEffect *effect, long opcode, long index, long value, void *ptr, float opt)
{
#if 0
    audioMasterEnum op = (audioMasterEnum)opcode;
    audioMasterEnumx opx = (audioMasterEnumx)opcode;
#endif

#ifdef FLEXT_DEBUG
//    	post("VST -> host: Eff = 0x%.8X, Opcode = %d, Index = %d, Value = %d, PTR = %.8X, OPT = %.3f\n",(int)effect, opcode,index,value,(int)ptr,opt);
#endif

	switch (opcode) {
    case audioMasterAutomate: // 0
#ifdef FLEXT_DEBUG
        post("Automate index=%li value=%li",index,value);
#endif
		// index, value given
		//! \todo set effect parameter
        return 0;
    case audioMasterVersion: // 1
        // support VST 2.3
        return 2300;
    case audioMasterCurrentId: // 2
        return 0;
	case audioMasterIdle: // 3
//		effect->dispatcher(effect, effEditIdle, 0, 0, NULL, 0.0f);
		return 0;
	case audioMasterPinConnected: // 4
		//! \todo set connection state correctly (if possible..)
		// index=pin, value=0..input, else..output
#ifdef FLEXT_DEBUG
        post("Pin connected pin=%li conn=%li",index,value);
#endif
		return 0; // 0 means connected
	case audioMasterWantMidi: // 6
#ifdef FLEXT_DEBUG
        post("Want MIDI = %li",value);
#endif
		return 0; // VST header says: "currently ignored"
	case audioMasterGetTime: // 7
		return 0; // not supported
    case audioMasterProcessEvents: { // 8
        // VST event data from plugin
        VstEvent *ev = static_cast<VstEvent *>(ptr);
        if(ev->type == kVstMidiType) {
            VstMidiEvent *mev = static_cast<VstMidiEvent *>(ptr);
#ifdef FLEXT_DEBUG
            if(mev->byteSize == 24)
                post("MIDI event delta=%li len=%li offs=%li detune=%i offvel=%i",mev->deltaFrames,mev->noteLength,mev->noteOffset,(int)mev->detune,(int)mev->noteOffVelocity);
            else
                // has incorrect size
                post("MIDI event");
#endif
        }
        else {
#ifdef FLEXT_DEBUG
            post("VST event type=%li",ev->type);
#endif
        }
		return 1;
    }
    case audioMasterSetTime: { // 9
        VstTimeInfo *tminfo = static_cast<VstTimeInfo *>(ptr);
#ifdef FLEXT_DEBUG
        post("TimeInfo pos=%lf rate=%lf filter=%li",tminfo->samplePos,tminfo->sampleRate,value);
#endif
        return 0; // not supported
    }
	case audioMasterTempoAt: // 10
		return 0; // not supported
	case audioMasterGetNumAutomatableParameters: // 11
		return 0; // not supported
    case audioMasterGetCurrentProcessLevel: // 23
        // return thread state
        return flext::GetThreadId() == flext::GetSysThreadId()?2:1;
	case audioMasterGetVendorString: // 32
		strcpy((char*)ptr,"grrrr.org");
        return 0;
	case audioMasterGetProductString: // 33
		strcpy((char *)ptr,"vst~ host external");
		return 0;
	case audioMasterGetVendorVersion: // 34
		return 100;
	case audioMasterCanDo: // 37
#ifdef FLEXT_DEBUG
    	post("\taudioMasterCanDo PTR = %s",ptr);
#endif
		return 0; // not supported
	case audioMasterGetLanguage: // 38
		return kVstLangEnglish;
	case audioMasterGetDirectory: // 41
        // return full path of plugin
		return 0; // not supported
    default:
#ifdef FLEXT_DEBUG
        post("Unknown opcode %li",opcode);
#endif
        return 0;
    }
}

#else

// Host callback dispatcher
long VSTPlugin::Master(AEffect *effect, long opcode, long index, long value, void *ptr, float opt)
{
    VSTPlugin *th = effect?(VSTPlugin *)effect->user:NULL;
    if(!th) FLEXT_LOG("No this");

#ifdef FLEXT_DEBUG
    if(opcode != audioMasterGetTime)
	post("VST plugin call to host dispatcher: Eff: 0x%.8X, Opcode = %d, Index = %d, Value = %d, PTR = %.8X, OPT = %.3f\n",(int)effect, opcode,index,value,(int)ptr,opt);
	//st( "audioMasterWantMidi %d " , audioMasterWantMidi);
#endif

	// Support opcodes
	switch(opcode)
	{
	case audioMasterAutomate:			
		return 0;		// index, value, returns 0
		
	case audioMasterVersion:			
		return 2;		// vst version, currently 7 (0 for older)
		
	case audioMasterCurrentId:			
		return 'AASH';	// returns the unique id of a plug that's currently loading

	case audioMasterIdle:
		effect->dispatcher(effect, effEditIdle, 0, 0, NULL, 0.0f);
		return 0;		// call application idle routine (this will call effEditIdle for all open editors too) 
		
	case audioMasterPinConnected:	
		if(value == 0)
            return index < 2?0:1; //input
		else 
            return index < 2?0:1; //output

/*
	case audioMasterWantMidi:			
		return 0;

	case audioMasterProcessEvents:		
		return 0; 	// Support of vst events to host is not available
*/
	case audioMasterGetTime:
		memset(&_timeInfo, 0, sizeof(_timeInfo));
		_timeInfo.samplePos = 0;
        _timeInfo.sampleRate = th?th->sample_rate:0;
		return (long)&_timeInfo;
		
		
	case audioMasterTempoAt:			
		return 0;

	case audioMasterNeedIdle:	
//		effect->dispatcher(effect, effIdle, 0, 0, NULL, 0.0f);
		return 1;

	case audioMasterGetSampleRate:		
        return th?(long)th->sample_rate:0;	

	case audioMasterGetVendorString:	// Just fooling version string
		strcpy((char*)ptr,"Steinberg");
		return 0;
	
	case audioMasterGetVendorVersion:	
		return 5000;	// HOST version 5000

	case audioMasterGetProductString:	// Just fooling product string
		strcpy((char *)ptr,"Cubase 5.0");
		return 0;

	case audioMasterVendorSpecific:		
		return 0;

	case audioMasterGetLanguage:		
		return kVstLangEnglish;
	
	case audioMasterUpdateDisplay:
		FLEXT_LOG("audioMasterUpdateDisplay");
		effect->dispatcher(effect, effEditIdle, 0, 0, NULL, 0.0f);
		return 0;

	case 	audioMasterCanDo:
		if (!strcmp((char *)ptr,"sendVstEvents")) return 1;
		else if (!strcmp((char *)ptr,"sendVstMidiEvent")) return 1;
		else if (!strcmp((char *)ptr,"sendVstTimeInfo")) return 1;
//			"receiveVstEvents",
//			"receiveVstMidiEvent",
//			"receiveVstTimeInfo",
		
//			"reportConnectionChanges",
//			"acceptIOChanges",
//		else if (!strcmp((char*)ptr,"sizeWindow")) return 1;
		else if (!strcmp((char*)ptr,"supplyIdle")) return 1;
		return -1;
		
	case 	audioMasterSetTime:						FLEXT_LOG("VST master dispatcher: Set Time");break;
	case 	audioMasterGetNumAutomatableParameters:	FLEXT_LOG("VST master dispatcher: GetNumAutPar");break;
	case 	audioMasterGetParameterQuantization:	FLEXT_LOG("VST master dispatcher: ParamQuant");break;
	case 	audioMasterIOChanged:					FLEXT_LOG("VST master dispatcher: IOchanged");break;
	case 	audioMasterSizeWindow:					FLEXT_LOG("VST master dispatcher: Size Window");break;
	case 	audioMasterGetBlockSize:				FLEXT_LOG("VST master dispatcher: GetBlockSize");break;
	case 	audioMasterGetInputLatency:				FLEXT_LOG("VST master dispatcher: GetInLatency");break;
	case 	audioMasterGetOutputLatency:			FLEXT_LOG("VST master dispatcher: GetOutLatency");break;
	case 	audioMasterGetPreviousPlug:				FLEXT_LOG("VST master dispatcher: PrevPlug");break;
	case 	audioMasterGetNextPlug:					FLEXT_LOG("VST master dispatcher: NextPlug");break;
	case 	audioMasterWillReplaceOrAccumulate:		FLEXT_LOG("VST master dispatcher: WillReplace"); break;
	case 	audioMasterGetCurrentProcessLevel:		return 0; break;
	case 	audioMasterGetAutomationState:			FLEXT_LOG("VST master dispatcher: GetAutState");break;
	case 	audioMasterOfflineStart:				FLEXT_LOG("VST master dispatcher: Offlinestart");break;
	case 	audioMasterOfflineRead:					FLEXT_LOG("VST master dispatcher: Offlineread");break;
	case 	audioMasterOfflineWrite:				FLEXT_LOG("VST master dispatcher: Offlinewrite");break;
	case 	audioMasterOfflineGetCurrentPass:		FLEXT_LOG("VST master dispatcher: OfflineGetcurrentpass");break;
	case 	audioMasterOfflineGetCurrentMetaPass:	FLEXT_LOG("VST master dispatcher: GetGetCurrentMetapass");break;
	case 	audioMasterSetOutputSampleRate:			FLEXT_LOG("VST master dispatcher: Setsamplerate");break;
	case 	audioMasterGetSpeakerArrangement:		FLEXT_LOG("VST master dispatcher: Getspeaker");break;
	case 	audioMasterSetIcon:						FLEXT_LOG("VST master dispatcher: seticon");break;
	case 	audioMasterOpenWindow:					FLEXT_LOG("VST master dispatcher: OpenWindow");break;
	case 	audioMasterCloseWindow:					FLEXT_LOG("VST master dispatcher: CloseWindow");break;
	case 	audioMasterGetDirectory:				FLEXT_LOG("VST master dispatcher: GetDirectory");break;
//	case		audioMasterUpdateDisplay:				post("VST master dispatcher: audioMasterUpdateDisplay");break;

#ifdef FLEXT_DEBUG
	default: 
        post("VST master dispatcher: undefed: %d , %d",opcode , effKeysRequired );
#endif
	}	
	
	return 0;
}

#endif 
